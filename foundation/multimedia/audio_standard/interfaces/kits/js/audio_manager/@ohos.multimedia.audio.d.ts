/*
* Copyright (C) 2021 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
import {ErrorCallback, AsyncCallback, Callback} from './basic';
import {VideoPlayer, AudioPlayer} from './@ohos.multimedia.media'
/**
 * @name audio
 * @since 6
 * @sysCap SystemCapability.Multimedia.Audio
 * @import import audio from '@ohos.Multimedia.audio';
 * @permission
 */
declare namespace audio {

  /**
   * Obtains an AudioManager instance.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  function getAudioManager(): AudioManager;

  /**
   * Enumerates audio stream types.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum AudioVolumeType {
	/**
     * Audio streams for ring tones
     */
    RINGTONE = 2,
    /**
     * Audio streams for media purpose
     */
    MEDIA = 3,
  }

  /**
   * Enumerates audio device flags.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum DeviceFlag {
    /**
     * Output devices
     */
    OUTPUT_DEVICES_FLAG = 1,
    /**
     * Input devices
     */
    INPUT_DEVICES_FLAG = 2,
    /**
     * All devices
     */
    ALL_DEVICES_FLAG = 3,
  }
  /**
   * Enumerates device roles.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum DeviceRole {
    /**
     * Input role
     */
    INPUT_DEVICE = 1,
    /**
     * Output role
     */
    OUTPUT_DEVICE = 2,
  }
  /**
   * Enumerates device types.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum DeviceType {
    /**
     * Invalid device
     */
    INVALID = 0,
    /**
     * earpiece
     */
    EARPIECE = 1,
    /**
     * Speaker
     */
    SPEAKER = 2,
    /**
     * Wired headset
     */
    WIRED_HEADSET = 3,
    /**
     * Bluetooth device using the synchronous connection oriented link (SCO)
     */
    BLUETOOTH_SCO = 7,
    /**
     * Bluetooth device using advanced audio distribution profile (A2DP)
     */
    BLUETOOTH_A2DP = 8,
    /**
     * Microphone
     */
    MIC = 15,
  }
  /**
   * Enumerates Active device types.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
   enum ActiveDeviceType {
    /**
     * Speaker
     */
    SPEAKER = 2,
    /**
     * Bluetooth device using the synchronous connection oriented link (SCO)
     */
    BLUETOOTH_SCO = 7,
  }
  /**
   * Enumerates Audio Ringer modes
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum AudioRingMode {
    /**
     * Silent mode
     */
    RINGER_MODE_SILENT = 0,
    /**
     * Vibration mode
     */
    RINGER_MODE_VIBRATE,
	 /**
     * Normal mode
     */
    RINGER_MODE_NORMAL,
  }

  /**
   * Manages audio volume and audio device information.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  interface AudioManager {
    /**
     * Sets volume for a stream. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setVolume(volumeType: AudioVolumeType, volume: number, callback: AsyncCallback<void>): void;
    /**
     * Sets volume for a stream. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setVolume(volumeType: AudioVolumeType, volume: number): Promise<void>;
    /**
     * Obtains volume of a stream. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getVolume(volumeType: AudioVolumeType, callback: AsyncCallback<number>): void;
    /**
     * Obtains the volume of a stream. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getVolume(volumeType: AudioVolumeType): Promise<number>;
    /**
     * Obtains the minimum volume allowed for a stream. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getMinVolume(volumeType: AudioVolumeType, callback: AsyncCallback<number>): void;
    /**
     * Obtains the minimum volume allowed for a stream. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getMinVolume(volumeType: AudioVolumeType): Promise<number>;
    /**
     * Obtains the maximum volume allowed for a stream. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getMaxVolume(volumeType: AudioVolumeType, callback: AsyncCallback<number>): void;
    /**
     * Obtains the maximum volume allowed for a stream. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getMaxVolume(volumeType: AudioVolumeType): Promise<number>;
    /**
     * Sets the stream to mute. This method uses an asynchronous callback to return the execution result.
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    mute(volumeType: AudioVolumeType, mute: boolean, callback: AsyncCallback<void>): void;
    /**
     * Sets the stream to mute. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    mute(volumeType: AudioVolumeType, mute: boolean): Promise<void>;
	  /**
     * Checks whether the stream is muted. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isMute(volumeType: AudioVolumeType, callback: AsyncCallback<boolean>): void;
    /**
     * Checks whether the stream is muted. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isMute(volumeType: AudioVolumeType): Promise<boolean>;
    /**
     * Checks whether the stream is active. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isActive(volumeType: AudioVolumeType, callback: AsyncCallback<boolean>): void;
    /**
     * Checks whether the stream is active. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isActive(volumeType: AudioVolumeType): Promise<boolean>;
	  /**
     * Mute/Unmutes the microphone. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setMicrophoneMute(mute: boolean, callback: AsyncCallback<void>): void;
    /**
     * Mute/Unmutes the microphone. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setMicrophoneMute(mute: boolean): Promise<void>;
    /**
     * Checks whether the microphone is muted. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isMicrophoneMute(callback: AsyncCallback<boolean>): void;
    /**
     * Checks whether the microphone is muted. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isMicrophoneMute(): Promise<boolean>;
	  /**
     * Sets the ringer mode. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setRingerMode(mode: AudioRingMode, callback: AsyncCallback<void>): void;
    /**
     * Sets the ringer mode. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setRingerMode(mode: AudioRingMode): Promise<void>;
	  /**
     * Gets the ringer mode. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getRingerMode(callback: AsyncCallback<AudioRingMode>): void;
    /**
     * Gets the ringer mode. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getRingerMode(): Promise<AudioRingMode>;
	  /**
     * Sets the audio parameter. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setAudioParameter(key: string, value: string, callback: AsyncCallback<void>): void;
    /**
     * Sets the audio parameter. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setAudioParameter(key: string, value: string): Promise<void>;
	  /**
     * Gets the audio parameter. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getAudioParameter(key: string, callback: AsyncCallback<string>): void;
    /**
     * Gets the audio parameter. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getAudioParameter(key: string): Promise<string>;
    /**
     * Obtains the audio devices of a specified flag. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getDevices(deviceFlag: DeviceFlag, callback: AsyncCallback<AudioDeviceDescriptors>): void;
    /**
     * Obtains the audio devices with a specified flag. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getDevices(deviceFlag: DeviceFlag): Promise<AudioDeviceDescriptors>;
	  /**
     * Activates the device. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setDeviceActive(deviceType: ActiveDeviceType, active: boolean, callback: AsyncCallback<void>): void;
    /**
     * Activates the device. This method uses a promise to return the execution result.
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    setDeviceActive(deviceType: ActiveDeviceType, active: boolean): Promise<void>;
    /**
     * Checks whether the device is active. This method uses an asynchronous callback to return the execution result.
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    isDeviceActive(deviceType: ActiveDeviceType, callback: AsyncCallback<boolean>): void;
    /**
     * Checks whether the device is active. This method uses a promise to return the execution result.
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    isDeviceActive(deviceType: ActiveDeviceType): Promise<boolean>;
  }

  /**
   * Describes an audio device.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  interface AudioDeviceDescriptor {
    /**
     * Audio device role
     * @devices
     */
    readonly deviceRole: DeviceRole;
    /**
     * Audio device type
     * @devices
     */
    readonly deviceType: DeviceType;
  }

  /**
   * A queue of AudioDeviceDescriptor, which is read-only.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  type AudioDeviceDescriptors = Array<Readonly<AudioDeviceDescriptor>>;
}

export default audio;