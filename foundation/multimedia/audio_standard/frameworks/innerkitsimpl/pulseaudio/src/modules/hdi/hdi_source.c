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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>

#include <pulse/rtclock.h>
#include <pulse/timeval.h>
#include <pulse/util.h>
#include <pulse/xmalloc.h>

#include <pulsecore/core.h>
#include <pulsecore/core-rtclock.h>
#include <pulsecore/core-util.h>
#include <pulsecore/log.h>
#include <pulsecore/macro.h>
#include <pulsecore/memchunk.h>
#include <pulsecore/modargs.h>
#include <pulsecore/module.h>
#include <pulsecore/rtpoll.h>
#include <pulsecore/thread.h>
#include <pulsecore/thread-mq.h>

#include <audio_manager.h>
#include <audio_capturer_source_intf.h>

#include "media_log.h"

#define DEFAULT_SOURCE_NAME "hdi_input"
#define DEFAULT_AUDIO_DEVICE_NAME "Internal Mic"

#define DEFAULT_BUFFER_SIZE (1024 * 16)
#define MAX_VOLUME_VALUE 15.0
#define DEFAULT_LEFT_VOLUME MAX_VOLUME_VALUE
#define DEFAULT_RIGHT_VOLUME MAX_VOLUME_VALUE
#define MAX_LATENCY_USEC (PA_USEC_PER_SEC * 2)
#define MIN_LATENCY_USEC 500
#define AUDIO_POINT_NUM  1024
#define AUDIO_FRAME_NUM_IN_BUF 30

struct userdata {
    pa_core *core;
    pa_module *module;
    pa_source *source;
    pa_thread *thread;
    pa_thread_mq thread_mq;
    pa_rtpoll *rtpoll;
    uint32_t buffer_size;
    pa_usec_t block_usec;
    pa_usec_t timestamp;
    AudioSourceAttr attrs;
    bool IsCapturerInit;
};

static int pa_capturer_init(struct userdata *u);
static void pa_capturer_exit(void);

static void userdata_free(struct userdata *u) {
    pa_assert(u);
    if (u->source)
        pa_source_unlink(u->source);

    if (u->thread) {
        pa_asyncmsgq_send(u->thread_mq.inq, NULL, PA_MESSAGE_SHUTDOWN, NULL, 0, NULL);
        pa_thread_free(u->thread);
    }

    pa_thread_mq_done(&u->thread_mq);

    if (u->source)
        pa_source_unref(u->source);

    if (u->rtpoll)
        pa_rtpoll_free(u->rtpoll);

    AudioCapturerSourceStop();
    AudioCapturerSourceDeInit();
    pa_xfree(u);
}

static int source_process_msg(pa_msgobject *o, int code, void *data, int64_t offset, pa_memchunk *chunk) {
    struct userdata *u = PA_SOURCE(o)->userdata;

    switch (code) {
        case PA_SOURCE_MESSAGE_GET_LATENCY: {
            pa_usec_t now;
            now = pa_rtclock_now();
            *((int64_t*)data) = (int64_t)now - (int64_t)u->timestamp;
            return 0;
        }
        default: {
            pa_log("source_process_msg default case");
            return pa_source_process_msg(o, code, data, offset, chunk);
        }
    }
}

/* Called from the IO thread. */
static int source_set_state_in_io_thread_cb(pa_source *s, pa_source_state_t new_state,
    pa_suspend_cause_t new_suspend_cause) {
    struct userdata *u = NULL;
    pa_assert(s);
    pa_assert_se(u = s->userdata);

    if ((s->thread_info.state == PA_SOURCE_SUSPENDED || s->thread_info.state == PA_SOURCE_INIT) &&
        PA_SOURCE_IS_OPENED(new_state)) {
        u->timestamp = pa_rtclock_now();
        if (new_state == PA_SOURCE_RUNNING && !u->IsCapturerInit) {
            if (pa_capturer_init(u)) {
                MEDIA_ERR_LOG("HDI capturer reinitialization failed");
                return -PA_ERR_IO;
            }
            u->IsCapturerInit = true;
            MEDIA_DEBUG_LOG("Successfully reinitialized HDI renderer");
        }
    } else if (s->thread_info.state == PA_SOURCE_IDLE) {
        if (new_state == PA_SOURCE_SUSPENDED) {
            if (u->IsCapturerInit) {
                pa_capturer_exit();
                u->IsCapturerInit = false;
                MEDIA_DEBUG_LOG("Deinitialized HDI capturer");
            }
        } else if (new_state == PA_SOURCE_RUNNING && !u->IsCapturerInit) {
            MEDIA_DEBUG_LOG("Idle to Running reinitializing HDI capturing device");
            if (pa_capturer_init(u)) {
                MEDIA_ERR_LOG("Idle to Running HDI capturer reinitialization failed");
                return -PA_ERR_IO;
            }
            u->IsCapturerInit = true;
            MEDIA_DEBUG_LOG("Idle to RunningsSuccessfully reinitialized HDI renderer");
        }
    }

    return 0;
}

static int get_capturer_frame_from_hdi(pa_memchunk *chunk, struct userdata *u) {
    uint64_t requestBytes;
    uint64_t replyBytes = 0;
    void *p = NULL;

    chunk->length = u->buffer_size;
    MEDIA_DEBUG_LOG("HDI Source: chunk.length = u->buffer_size: %{public}zu", chunk->length);
    chunk->memblock = pa_memblock_new(u->core->mempool, chunk->length);
    pa_assert(chunk->memblock);
    p = pa_memblock_acquire(chunk->memblock);
    pa_assert(p);

    requestBytes = pa_memblock_get_length(chunk->memblock);
    AudioCapturerSourceFrame((char *)p, (uint64_t)requestBytes, &replyBytes);

    pa_memblock_release(chunk->memblock);
    MEDIA_DEBUG_LOG("HDI Source: request bytes: %{public}" PRIu64 ", replyBytes: %{public}" PRIu64,
            requestBytes, replyBytes);
    if (replyBytes > requestBytes) {
        MEDIA_ERR_LOG("HDI Source: Error replyBytes > requestBytes. Requested data Length: "
                "%{public}" PRIu64 ", Read: %{public}" PRIu64 " bytes", requestBytes, replyBytes);
        pa_memblock_unref(chunk->memblock);
        return -1;
    }

    if (replyBytes == 0) {
        MEDIA_ERR_LOG("HDI Source: Failed to read, Requested data Length: %{public}" PRIu64 " bytes,"
                " Read: %{public}" PRIu64 " bytes", requestBytes, replyBytes);
        pa_memblock_unref(chunk->memblock);
        return -1;
    }

    chunk->index = 0;
    chunk->length = replyBytes;
    pa_source_post(u->source, chunk);
    pa_memblock_unref(chunk->memblock);

    return 0;
}

static void thread_func(void *userdata) {
    struct userdata *u = userdata;
    bool timer_elapsed = false;

    pa_assert(u);

    if (u->core->realtime_scheduling)
        pa_thread_make_realtime(u->core->realtime_priority);

    pa_thread_mq_install(&u->thread_mq);
    u->timestamp = pa_rtclock_now();
    MEDIA_DEBUG_LOG("HDI Source: u->timestamp : %{public}" PRIu64, u->timestamp);

    while (true) {
        int ret = 0;

        if (PA_SOURCE_IS_RUNNING(u->source->thread_info.state) && u->IsCapturerInit) {
            pa_memchunk chunk;
            pa_usec_t now;

            now = pa_rtclock_now();
            MEDIA_DEBUG_LOG("HDI Source: now: %{public}" PRIu64 " timer_elapsed: %{public}d", now, timer_elapsed);

            if (timer_elapsed && (chunk.length = pa_usec_to_bytes(now - u->timestamp, &u->source->sample_spec)) > 0) {
                ret = get_capturer_frame_from_hdi(&chunk, u);
                if (ret != 0) {
                    break;
                }

                u->timestamp += pa_bytes_to_usec(chunk.length, &u->source->sample_spec);
                MEDIA_INFO_LOG("HDI Source: new u->timestamp : %{public}" PRIu64, u->timestamp);
            }

            pa_rtpoll_set_timer_absolute(u->rtpoll, u->timestamp + u->block_usec);
        } else {
            pa_rtpoll_set_timer_disabled(u->rtpoll);
            MEDIA_INFO_LOG("HDI Source: pa_rtpoll_set_timer_disabled done ");
        }

        /* Hmm, nothing to do. Let's sleep */
        if ((ret = pa_rtpoll_run(u->rtpoll)) < 0) {
            /* If this was no regular exit from the loop we have to continue
            * processing messages until we received PA_MESSAGE_SHUTDOWN */
            MEDIA_INFO_LOG("HDI Source: pa_rtpoll_run ret:%{public}d failed", ret);
            pa_asyncmsgq_post(u->thread_mq.outq, PA_MSGOBJECT(u->core), PA_CORE_MESSAGE_UNLOAD_MODULE, u->module,
                0, NULL, NULL);
            pa_asyncmsgq_wait_for(u->thread_mq.inq, PA_MESSAGE_SHUTDOWN);
            return;
        }

        timer_elapsed = pa_rtpoll_timer_elapsed(u->rtpoll);

        if (ret == 0) {
            return;
        }
    }
}

static int pa_capturer_init(struct userdata *u) {
    int ret;

    ret = AudioCapturerSourceInit(&u->attrs);
    if (ret != 0) {
        MEDIA_ERR_LOG("Audio capturer init failed!");
        return ret;
    }

    ret = AudioCapturerSourceSetVolume(DEFAULT_LEFT_VOLUME, DEFAULT_RIGHT_VOLUME);
    if (ret != 0) {
        MEDIA_ERR_LOG("audio capturer set volume failed!");
        goto fail;
    }

    bool muteState = AudioCapturerSourceIsMuteRequired();
    ret = AudioCapturerSourceSetMute(muteState);
    if (ret != 0) {
        MEDIA_ERR_LOG("audio capturer set muteState: %d failed!", muteState);
        goto fail;
    }

    ret = AudioCapturerSourceStart();
    if (ret != 0) {
        MEDIA_ERR_LOG("Audio capturer start failed!");
        goto fail;
    }

    u->IsCapturerInit = true;
    return ret;

fail:
    pa_capturer_exit();
    return ret;
}

static void pa_capturer_exit(void) {
    AudioCapturerSourceStop();
    AudioCapturerSourceDeInit();
}

static int pa_set_source_properties(pa_module *m, pa_modargs *ma, pa_sample_spec *ss, pa_channel_map *map,
    struct userdata *u) {
    pa_source_new_data data;

    pa_source_new_data_init(&data);
    data.driver = __FILE__;
    data.module = m;
    pa_source_new_data_set_name(&data, pa_modargs_get_value(ma, "source_name", DEFAULT_SOURCE_NAME));
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_STRING, DEFAULT_AUDIO_DEVICE_NAME);
    pa_proplist_setf(data.proplist, PA_PROP_DEVICE_DESCRIPTION, "HDI source %s", DEFAULT_AUDIO_DEVICE_NAME);
    pa_source_new_data_set_sample_spec(&data, ss);
    pa_source_new_data_set_channel_map(&data, map);
    pa_proplist_setf(data.proplist, PA_PROP_DEVICE_BUFFERING_BUFFER_SIZE, "%lu", (unsigned long)u->buffer_size);

    if (pa_modargs_get_proplist(ma, "source_properties", data.proplist, PA_UPDATE_REPLACE) < 0) {
        MEDIA_INFO_LOG("Invalid properties");
        pa_source_new_data_done(&data);
        return -1;
    }

    u->source = pa_source_new(m->core, &data, PA_SOURCE_HARDWARE | PA_SOURCE_LATENCY);
    pa_source_new_data_done(&data);

    if (!u->source) {
        MEDIA_INFO_LOG("Failed to create source object");
        return -1;
    }

    u->source->parent.process_msg = source_process_msg;
    u->source->set_state_in_io_thread = source_set_state_in_io_thread_cb;
    u->source->userdata = u;

    pa_source_set_asyncmsgq(u->source, u->thread_mq.inq);
    pa_source_set_rtpoll(u->source, u->rtpoll);

    if (pa_modargs_get_value_u32(ma, "buffer_size", &u->buffer_size) < 0) {
        MEDIA_INFO_LOG("Failed to parse buffer_size argument.");
        return -1;
    }

    u->block_usec = pa_bytes_to_usec(u->buffer_size, &u->source->sample_spec);
    pa_source_set_latency_range(u->source, 0, u->block_usec);
    u->source->thread_info.max_rewind = pa_usec_to_bytes(u->block_usec, &u->source->sample_spec);

    return 0;
}

pa_source *pa_hdi_source_new(pa_module *m, pa_modargs *ma, const char *driver) {
    struct userdata *u = NULL;
    pa_sample_spec ss;
    char *thread_name = NULL;
    pa_channel_map map;
    int ret;

    pa_assert(m);
    pa_assert(ma);

    ss = m->core->default_sample_spec;
    map = m->core->default_channel_map;

    /* Override with modargs if provided */
    if (pa_modargs_get_sample_spec_and_channel_map(ma, &ss, &map, PA_CHANNEL_MAP_DEFAULT) < 0) {
        MEDIA_INFO_LOG("Failed to parse sample specification and channel map");
        goto fail;
    }

    u = pa_xnew0(struct userdata, 1);
    u->core = m->core;
    u->module = m;
    u->rtpoll = pa_rtpoll_new();

    if (pa_thread_mq_init(&u->thread_mq, m->core->mainloop, u->rtpoll) < 0) {
        MEDIA_INFO_LOG("pa_thread_mq_init() failed.");
        goto fail;
    }

    u->buffer_size = DEFAULT_BUFFER_SIZE;
    u->attrs.format = AUDIO_FORMAT_PCM_16_BIT;
    u->attrs.channel = ss.channels;
    u->attrs.sampleRate = ss.rate;
    MEDIA_INFO_LOG("AudioDeviceCreateCapture format: %{public}d, channel: %{public}d, sampleRate: %{public}d",
                   u->attrs.format, u->attrs.channel, u->attrs.sampleRate);
    ret = pa_capturer_init(u);
    if (ret != 0) {
        goto fail;
    }

    ret = pa_set_source_properties(m, ma, &ss, &map, u);
    if (ret != 0) {
        goto fail;
    }

    thread_name = pa_sprintf_malloc("hdi-source-record");
    if (!(u->thread = pa_thread_new(thread_name, thread_func, u))) {
        MEDIA_INFO_LOG("Failed to create hdi-source-record thread!");
        goto fail;
    }

    pa_xfree(thread_name);
    thread_name = NULL;
    pa_source_put(u->source);
    return u->source;

fail:
    pa_xfree(thread_name);

    if (u) {
        if (u->IsCapturerInit) {
            pa_capturer_exit();
        }
        userdata_free(u);
    }

    return NULL;
}

void pa_hdi_source_free(pa_source *s) {
    struct userdata *u = NULL;
    pa_source_assert_ref(s);
    pa_assert_se(u = s->userdata);
    userdata_free(u);
}
