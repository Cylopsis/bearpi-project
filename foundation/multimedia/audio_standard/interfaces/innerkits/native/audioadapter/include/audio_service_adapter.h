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

#ifndef ST_AUDIO_SERVICE_ADAPTER_H
#define ST_AUDIO_SERVICE_ADAPTER_H

#include <memory>
#include <string>
#include <unistd.h>

#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioServiceAdapterCallback {
public:
    /**
     * @brief computes the volume to be set in audioserver
     *
     * @param streamType streamType for which volume will be computed
     * @return Returns volume level in float
     */
    virtual float OnGetVolumeCb(std::string streamType) = 0;

    virtual ~AudioServiceAdapterCallback() {};
};

class AudioServiceAdapter {
public:
    /**
     * @brief creater audioserviceadapter instance
     *
     * @param cb callback reference for AudioServiceAdapterCallback class
     * @return Returns instance of class that extends AudioServiceAdapter
    */
    static std::unique_ptr<AudioServiceAdapter> CreateAudioAdapter(std::unique_ptr<AudioServiceAdapterCallback> cb);

    /**
     * @brief Connect to underlining audio server
     *
     * @return Returns true if connection is success, else return false
     * @since 1.0
     * @version 1.0
     */
    virtual bool Connect() = 0;

    /**
     * @brief Opens the audio port while loading the audio modules source and sink.
     *
     * @param audioPortName name of the audio modules to be loaded
     * @param moduleArgs audio module info like rate, channel etc
     * @return Returns module index if module loaded sucessfully; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t OpenAudioPort(std::string audioPortName, std::string moduleArgs) = 0;

    /**
     * @brief closes/unloads the audio modules loaded.
     *
     * @param audioHandleIndex the index of the loaded audio module
     * @return Returns {@link SUCCESS} if module/port is closed successfully; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t CloseAudioPort(int32_t audioHandleIndex) = 0;

    /**
     * @brief sets default audio sink.
     *
     * @param name name of default audio sink to be set
     * @return Returns {@link SUCCESS} if default audio sink is set successfully; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t SetDefaultSink(std::string name) = 0;

    /**
     * @brief sets default audio source.
     *
     * @param name name of default audio source to be set
     * @return Returns {@link SUCCESS} if default audio source is set successfully; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t SetDefaultSource(std::string name) = 0;

    /**
     * @brief sets audio volume
     *
     * @param streamType the streamType for which volume will be set, streamType defined in{@link audio_info.h}
     * @param volume the volume level to be set
     * @return Returns {@link SUCCESS} if volume is set successfully; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t SetVolume(AudioStreamType streamType, float volume) = 0;

    /**
     * @brief set mute for give streamType
     *
     * @param streamType the streamType for which mute will be set, streamType defined in{@link audio_info.h}
     * @param mute boolean value, true: Set mute; false: Set unmute
     * @return Returns {@link SUCCESS} if mute/unmute is set successfully; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t SetMute(AudioStreamType streamType, bool mute) = 0;

    /**
     * @brief returns if given streamType is set to mute
     *
     * @param streamType the streamType for which mute status will be fetched, streamType defined in{@link audio_info.h}
     * @return Returns true: Is streamType is set as mute; else returns false
     */
    virtual bool IsMute(AudioStreamType streamType) = 0;

    /**
     * @brief returns if given streamType is active(currently the streamType audio is played)
     *
     * @param streamType the streamType for which status will be fetched streamType defined in{@link audio_info.h}
     * @return Returns true: If streamType is active; else returns false
     */
    virtual bool IsStreamActive(AudioStreamType streamType);

    /**
     * @brief Disconnects the connected audio server
     *
     * @return void
     */
    virtual void Disconnect() = 0;

    virtual ~AudioServiceAdapter();
};
} // namespace AudioStandard
} // namespace OHOS
#endif  // ST_AUDIO_SERVICE_ADAPTER_H
