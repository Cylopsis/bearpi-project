/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ST_PULSEAUDIO_AUDIO_SERVICE_ADAPTER_IMPL_H
#define ST_PULSEAUDIO_AUDIO_SERVICE_ADAPTER_IMPL_H

#include <unistd.h>

#include "pulse_audio_service_adapter_impl.h"
#include "audio_errors.h"
#include "media_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
static unique_ptr<AudioServiceAdapterCallback> mAudioServiceAdapterCallback;

AudioServiceAdapter::~AudioServiceAdapter() = default;
PulseAudioServiceAdapterImpl::~PulseAudioServiceAdapterImpl() = default;

unique_ptr<AudioServiceAdapter> AudioServiceAdapter::CreateAudioAdapter(unique_ptr<AudioServiceAdapterCallback> cb)
{
    return make_unique<PulseAudioServiceAdapterImpl>(cb);
}

PulseAudioServiceAdapterImpl::PulseAudioServiceAdapterImpl(unique_ptr<AudioServiceAdapterCallback> &cb)
{
    mAudioServiceAdapterCallback = move(cb);
}

bool PulseAudioServiceAdapterImpl::Connect()
{
    mMainLoop = pa_threaded_mainloop_new();
    if (!mMainLoop) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] MainLoop creation failed");
        return false;
    }

    if (pa_threaded_mainloop_start(mMainLoop) < 0) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Failed to start mainloop");
        pa_threaded_mainloop_free (mMainLoop);
        return false;
    }

    pa_threaded_mainloop_lock(mMainLoop);

    while (true) {
        pa_context_state_t state;

        if (mContext != NULL) {
            state = pa_context_get_state(mContext);
            if (state == PA_CONTEXT_READY)
                break;
            // if pulseaudio is ready, retry connect to pulseaudio. before retry wait for sometime. reduce sleep later
            usleep(PA_CONNECT_RETRY_SLEEP_IN_MICRO_SECONDS);
        }

        bool result = ConnectToPulseAudio();
        if (!result || !PA_CONTEXT_IS_GOOD(pa_context_get_state(mContext))) {
            continue;
        }

        MEDIA_DEBUG_LOG("[PulseAudioServiceAdapterImpl] pa context not ready... wait");

        // Wait for the context to be ready
        pa_threaded_mainloop_wait(mMainLoop);
    }

    pa_threaded_mainloop_unlock(mMainLoop);

    return true;
}

bool PulseAudioServiceAdapterImpl::ConnectToPulseAudio()
{
    unique_ptr<UserData> userData = make_unique<UserData>();
    userData->thiz = this;

    if (mContext != NULL) {
        pa_context_disconnect(mContext);
        pa_context_set_state_callback(mContext, NULL, NULL);
        pa_context_set_subscribe_callback(mContext, NULL, NULL);
        pa_context_unref(mContext);
    }

    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "PulseAudio Service");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "com.ohos.pulseaudio.service");
    mContext = pa_context_new_with_proplist(pa_threaded_mainloop_get_api(mMainLoop), NULL, proplist);
    pa_proplist_free(proplist);

    if (mContext == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] creating pa context failed");
        return false;
    }

    pa_context_set_state_callback(mContext,  PulseAudioServiceAdapterImpl::PaContextStateCb, this);
    if (pa_context_connect(mContext, NULL, PA_CONTEXT_NOFAIL, NULL) < 0) {
        if (pa_context_errno(mContext) == PA_ERR_INVALID) {
            MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] pa context connect failed: %{public}s",
                pa_strerror(pa_context_errno(mContext)));
            goto Fail;
        }
    }

    return true;

Fail:
    /* Make sure we don't get any further callbacks */
    pa_context_set_state_callback(mContext, NULL, NULL);
    pa_context_set_subscribe_callback(mContext, NULL, NULL);
    pa_context_unref(mContext);
    return false;
}

int32_t PulseAudioServiceAdapterImpl::OpenAudioPort(string audioPortName, string moduleArgs)
{
    unique_ptr<UserData> userData = make_unique<UserData>();
    userData->thiz = this;

    pa_threaded_mainloop_lock(mMainLoop);
    pa_operation *operation = pa_context_load_module(mContext, audioPortName.c_str(), moduleArgs.c_str(),
        PaModuleLoadCb, reinterpret_cast<void*>(userData.get()));
    if (operation == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] pa_context_load_module returned nullptr");
        pa_threaded_mainloop_unlock(mMainLoop);
        return ERR_INVALID_HANDLE;
    }

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mMainLoop);
    }

    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(mMainLoop);

    return userData->idx;
}

int32_t PulseAudioServiceAdapterImpl::CloseAudioPort(int32_t audioHandleIndex)
{
    pa_threaded_mainloop_lock(mMainLoop);

    pa_operation *operation = pa_context_unload_module(mContext, audioHandleIndex, NULL, NULL);
    if (operation == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] pa_context_unload_module returned nullptr!");
        pa_threaded_mainloop_unlock(mMainLoop);
        return ERROR;
    }

    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(mMainLoop);
    return SUCCESS;
}

int32_t PulseAudioServiceAdapterImpl::SetDefaultSink(string name)
{
    pa_threaded_mainloop_lock(mMainLoop);
    pa_operation *operation = pa_context_set_default_sink(mContext, name.c_str(), NULL, NULL);
    if (operation == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] pa_context_set_default_sink failed!");
        pa_threaded_mainloop_unlock(mMainLoop);
        return ERR_OPERATION_FAILED;
    }
    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(mMainLoop);

    return SUCCESS;
}

int32_t PulseAudioServiceAdapterImpl::SetDefaultSource(string name)
{
    pa_threaded_mainloop_lock(mMainLoop);
    pa_operation *operation = pa_context_set_default_source(mContext, name.c_str(), NULL, NULL);
    if (operation == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] pa_context_set_default_source failed!");
        pa_threaded_mainloop_unlock(mMainLoop);
        return ERR_OPERATION_FAILED;
    }
    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(mMainLoop);

    return SUCCESS;
}

int32_t PulseAudioServiceAdapterImpl::SetVolume(AudioStreamType streamType, float volume)
{
    unique_ptr<UserData> userData = make_unique<UserData>();
    userData->thiz = this;
    userData->volume = volume;
    userData->streamType = streamType;

    if (mContext == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] SetVolume mContext is nullptr");
        return ERROR;
    }
    pa_threaded_mainloop_lock(mMainLoop);
    pa_operation *operation = pa_context_get_sink_input_info_list(mContext,
        PulseAudioServiceAdapterImpl::PaGetSinkInputInfoVolumeCb, reinterpret_cast<void*>(userData.get()));
    if (operation == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] pa_context_get_sink_input_info_list nullptr");
        pa_threaded_mainloop_unlock(mMainLoop);
        return ERROR;
    }
    userData.release();

    pa_threaded_mainloop_accept(mMainLoop);

    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(mMainLoop);

    return SUCCESS;
}

int32_t PulseAudioServiceAdapterImpl::SetMute(AudioStreamType streamType, bool mute)
{
    unique_ptr<UserData> userData = make_unique<UserData>();
    userData->thiz = this;
    userData->mute = mute;
    userData->streamType = streamType;

    if (mContext == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] SetMute mContext is nullptr");
        return ERROR;
    }
    pa_threaded_mainloop_lock(mMainLoop);

    pa_operation *operation = pa_context_get_sink_input_info_list(mContext,
        PulseAudioServiceAdapterImpl::PaGetSinkInputInfoMuteCb, reinterpret_cast<void*>(userData.get()));
    if (operation == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] pa_context_get_sink_input_info_list returned nullptr");
        pa_threaded_mainloop_unlock(mMainLoop);
        return ERROR;
    }

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mMainLoop);
    }

    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(mMainLoop);

    return SUCCESS;
}

bool PulseAudioServiceAdapterImpl::IsMute(AudioStreamType streamType)
{
    unique_ptr<UserData> userData = make_unique<UserData>();
    userData->thiz = this;
    userData->streamType = streamType;
    userData->mute = false;

    if (mContext == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] IsMute mContext is nullptr");
        return false;
    }

    pa_threaded_mainloop_lock(mMainLoop);

    pa_operation *operation = pa_context_get_sink_input_info_list(mContext,
        PulseAudioServiceAdapterImpl::PaGetSinkInputInfoMuteStatusCb, reinterpret_cast<void*>(userData.get()));
    if (operation == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] pa_context_get_sink_input_info_list returned nullptr");
        pa_threaded_mainloop_unlock(mMainLoop);
        return false;
    }

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mMainLoop);
    }

    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(mMainLoop);

    return (userData->mute) ? true : false;
}

bool PulseAudioServiceAdapterImpl::IsStreamActive(AudioStreamType streamType)
{
    unique_ptr<UserData> userData = make_unique<UserData>();
    userData->thiz = this;
    userData->streamType = streamType;
    userData->isCorked = true;

    if (mContext == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] IsStreamActive mContext is nullptr");
        return false;
    }

    pa_threaded_mainloop_lock(mMainLoop);

    pa_operation *operation = pa_context_get_sink_input_info_list(mContext,
        PulseAudioServiceAdapterImpl::PaGetSinkInputInfoCorkStatusCb, reinterpret_cast<void*>(userData.get()));
    if (operation == NULL) {
        MEDIA_ERR_LOG("[IsStreamActive] pa_context_get_sink_input_info_list returned nullptr");
        pa_threaded_mainloop_unlock(mMainLoop);
        return false;
    }

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mMainLoop);
    }

    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(mMainLoop);

    MEDIA_INFO_LOG("[IsStreamActive] cork for stream %s : %d",
        GetNameByStreamType(streamType).c_str(), userData->isCorked);

    return (userData->isCorked) ? false : true;
}

void PulseAudioServiceAdapterImpl::Disconnect()
{
    if (mContext != NULL) {
        pa_context_disconnect(mContext);
        /* Make sure we don't get any further callbacks */
        pa_context_set_state_callback(mContext, NULL, NULL);
        pa_context_set_subscribe_callback(mContext, NULL, NULL);
        pa_context_unref(mContext);
    }

    if (mMainLoop != NULL) {
        pa_threaded_mainloop_stop(mMainLoop);
        pa_threaded_mainloop_free(mMainLoop);
    }

    return;
}

string PulseAudioServiceAdapterImpl::GetNameByStreamType(AudioStreamType streamType)
{
    switch (streamType) {
        case STREAM_MUSIC:
            return "music";
        case STREAM_RING:
            return "ring";
        case STREAM_SYSTEM:
            return "system";
        case STREAM_NOTIFICATION:
            return "notification";
        case STREAM_ALARM:
            return "alarm";
        case STREAM_DTMF:
            return "dtmf";
        default:
            return "";
    }
}

AudioStreamType PulseAudioServiceAdapterImpl::GetIdByStreamType(string streamType)
{
    AudioStreamType stream;

    if (!streamType.compare(string("music"))) {
        stream = STREAM_MUSIC;
    } else if (!streamType.compare(string("ring"))) {
        stream = STREAM_RING;
    } else if (!streamType.compare(string("system"))) {
        stream = STREAM_SYSTEM;
    } else if (!streamType.compare(string("notification"))) {
        stream = STREAM_NOTIFICATION;
    } else if (!streamType.compare(string("alarm"))) {
        stream = STREAM_ALARM;
    } else {
        stream = STREAM_MUSIC;
    }

    return stream;
}

void PulseAudioServiceAdapterImpl::PaGetSinkInputInfoMuteStatusCb(pa_context *c, const pa_sink_input_info *i, int eol,
    void *userdata)
{
    UserData *userData = reinterpret_cast<UserData*>(userdata);
    PulseAudioServiceAdapterImpl *thiz = userData->thiz;

    if (eol < 0) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Failed to get sink input information: %{public}s",
            pa_strerror(pa_context_errno(c)));
        return;
    }

    if (eol) {
        pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
        return;
    }

    if (i->proplist == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Invalid Proplist for sink input (%{public}d).", i->index);
        return;
    }

    const char *streamtype = pa_proplist_gets(i->proplist, "stream.type");
    if (streamtype == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Invalid StreamType.");
        return;
    }

    string streamType(streamtype);
    if (!streamType.compare(thiz->GetNameByStreamType(userData->streamType))) {
        userData->mute = i->mute;
        MEDIA_INFO_LOG("[PulseAudioServiceAdapterImpl] Mute : %{public}d for stream : %{public}s",
            userData->mute, i->name);
    }

    return;
}

void PulseAudioServiceAdapterImpl::PaGetSinkInputInfoMuteCb(pa_context *c, const pa_sink_input_info *i,
    int eol, void *userdata)
{
    UserData *userData = reinterpret_cast<UserData*>(userdata);
    PulseAudioServiceAdapterImpl *thiz = userData->thiz;

    if (eol < 0) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Failed to get sink input information: %{public}s",
            pa_strerror(pa_context_errno(c)));
        return;
    }

    if (eol) {
        pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
        return;
    }

    if (i->proplist == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Invalid Proplist for sink input (%{public}d).", i->index);
        return;
    }

    const char *streamtype = pa_proplist_gets(i->proplist, "stream.type");
    if (streamtype == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Invalid StreamType.");
        return;
    }

    string streamType(streamtype);
    if (!streamType.compare(thiz->GetNameByStreamType(userData->streamType))) {
        pa_operation_unref(pa_context_set_sink_input_mute(c, i->index, (userData->mute) ? 1 : 0, NULL, NULL));
        MEDIA_INFO_LOG("[PulseAudioServiceAdapterImpl] Applied Mute : %{public}d for stream : %{public}s",
            userData->mute, i->name);
    }

    return;
}

void PulseAudioServiceAdapterImpl::PaContextStateCb(pa_context *c, void *userdata)
{
    PulseAudioServiceAdapterImpl *thiz = reinterpret_cast<PulseAudioServiceAdapterImpl*>(userdata);

    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY: {
            pa_context_set_subscribe_callback(c, PulseAudioServiceAdapterImpl::PaSubscribeCb, thiz);

            pa_operation *operation = pa_context_subscribe(c, (pa_subscription_mask_t)
                (PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SOURCE |
                PA_SUBSCRIPTION_MASK_SINK_INPUT | PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT |
                PA_SUBSCRIPTION_MASK_CARD), NULL, NULL);
            if (operation == NULL) {
                pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
                return;
            }
            pa_operation_unref(operation);
            pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
            break;
        }

        case PA_CONTEXT_FAILED:
            pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
            return;

        case PA_CONTEXT_TERMINATED:
        default:
            return;
    }
}

void PulseAudioServiceAdapterImpl::PaModuleLoadCb(pa_context *c, uint32_t idx, void *userdata)
{
    UserData *userData = reinterpret_cast<UserData*>(userdata);
    if (idx == PA_INVALID_INDEX) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Failure: %{public}s", pa_strerror(pa_context_errno(c)));
        userData->idx = PA_INVALID_INDEX;
    } else {
        userData->idx = idx;
    }
    pa_threaded_mainloop_signal(userData->thiz->mMainLoop, 0);

    return;
}

void PulseAudioServiceAdapterImpl::PaGetSinkInputInfoVolumeCb(pa_context *c, const pa_sink_input_info *i, int eol,
    void *userdata)
{
    UserData *userData = reinterpret_cast<UserData*>(userdata);
    PulseAudioServiceAdapterImpl *thiz = userData->thiz;

    MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] GetSinkInputInfoVolumeCb");
    if (eol < 0) {
        delete userData;
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Failed to get sink input information: %{public}s",
            pa_strerror(pa_context_errno(c)));
        return;
    }

    if (eol) {
        pa_threaded_mainloop_signal(thiz->mMainLoop, 1);
        delete userData;
        return;
    }

    if (i->proplist == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Invalid Proplist for sink input (%{public}d).", i->index);
        return;
    }

    const char *streamtype = pa_proplist_gets(i->proplist, "stream.type");
    const char *streamVolume = pa_proplist_gets(i->proplist, "stream.volumeFactor");
    if ((streamtype == NULL) || (streamVolume == NULL)) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Invalid StreamType.");
        return;
    }

    string streamType(streamtype);
    float volumeFactor = atof(streamVolume);
    AudioStreamType streamID = thiz->GetIdByStreamType(streamType);
    float volumeCb = mAudioServiceAdapterCallback->OnGetVolumeCb(streamtype);
    float vol = volumeCb * volumeFactor;

    pa_cvolume cv = i->volume;
    int32_t volume = pa_sw_volume_from_linear(vol);
    pa_cvolume_set(&cv, i->channel_map.channels, volume);
    pa_operation_unref(pa_context_set_sink_input_volume(c, i->index, &cv, NULL, NULL));

    if (streamID == userData->streamType) {
        if (i->mute) {
            pa_operation_unref(pa_context_set_sink_input_mute(c, i->index, 0, NULL, NULL));
        }
    }
    MEDIA_INFO_LOG("[PulseAudioServiceAdapterImpl]volume : %{public}f for stream : %{public}s, volumeInt%{public}d",
        vol, i->name, volume);

    return;
}

void PulseAudioServiceAdapterImpl::PaGetSinkInputInfoCorkStatusCb(pa_context *c, const pa_sink_input_info *i, int eol,
    void *userdata)
{
    UserData *userData = reinterpret_cast<UserData*>(userdata);
    PulseAudioServiceAdapterImpl *thiz = userData->thiz;

    if (eol < 0) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Failed to get sink input information: %{public}s",
            pa_strerror(pa_context_errno(c)));
        return;
    }

    if (eol) {
        pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
        return;
    }

    if (i->proplist == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Invalid Proplist for sink input (%{public}d).", i->index);
        return;
    }

    const char *streamtype = pa_proplist_gets(i->proplist, "stream.type");
    if (streamtype == NULL) {
        MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Invalid StreamType.");
        return;
    }

    string streamType(streamtype);
    if (!streamType.compare(thiz->GetNameByStreamType(userData->streamType))) {
        userData->isCorked = i->corked;
        MEDIA_INFO_LOG("[PulseAudioServiceAdapterImpl] corked : %{public}d for stream : %{public}s",
            userData->isCorked, i->name);
    }

    return;
}

void PulseAudioServiceAdapterImpl::PaSubscribeCb(pa_context *c, pa_subscription_event_type_t t, uint32_t idx,
    void *userdata)
{
    unique_ptr<UserData> userData = make_unique<UserData>();
    PulseAudioServiceAdapterImpl *thiz = reinterpret_cast<PulseAudioServiceAdapterImpl*>(userdata);
    userData->thiz = thiz;
    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
        case PA_SUBSCRIPTION_EVENT_SINK:
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE:
            break;

        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_NEW) {
                pa_threaded_mainloop_lock(thiz->mMainLoop);
                pa_operation *operation = pa_context_get_sink_input_info(c, idx,
                    PulseAudioServiceAdapterImpl::PaGetSinkInputInfoVolumeCb, reinterpret_cast<void*>(userData.get()));
                if (operation == NULL) {
                    MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] pa_context_get_sink_input_info_list nullptr");
                    pa_threaded_mainloop_unlock(thiz->mMainLoop);
                    return;
                }
                userData.release();
                pa_threaded_mainloop_accept(thiz->mMainLoop);
                pa_operation_unref(operation);
                pa_threaded_mainloop_unlock(thiz->mMainLoop);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
            break;

        default:
            break;
    }

    return;
}
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_PULSEAUDIO_AUDIO_SERVICE_ADAPTER_IMPL_H
