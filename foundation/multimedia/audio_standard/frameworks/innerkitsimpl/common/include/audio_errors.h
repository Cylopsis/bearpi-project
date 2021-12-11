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

/**
 * @addtogroup MultiMedia_AudioCommon
 * @{
 *
 * @brief Provides data types and audio formats required for recording and playing and recording audio.
 *
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file audio_errors.h
 *
 * @brief Declares the <b>audio_errors</b> class to define errors that may occur during audio operations.
 *
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef AUDIO_ERRORS_H
#define AUDIO_ERRORS_H

#include <cstdint>

namespace OHOS {
namespace AudioStandard {
constexpr int MODULE_AUDIO = 1;
constexpr int SUBSYS_AUDIO = 30;

using ErrCode = int32_t;
constexpr int SUBSYSTEM_BIT_NUM = 21;
constexpr int MODULE_BIT_NUM = 16;

constexpr ErrCode ErrCodeOffset(unsigned int subsystem, unsigned int module = 0)
{
    return (subsystem << SUBSYSTEM_BIT_NUM) | (module << MODULE_BIT_NUM);
}

constexpr int32_t BASE_AUDIO_ERR_OFFSET = -ErrCodeOffset(SUBSYS_AUDIO, MODULE_AUDIO);

/** Success */
const int32_t  SUCCESS = 0;

/** Fail */
const int32_t  ERROR = BASE_AUDIO_ERR_OFFSET;

/** Status error */
const int32_t  ERR_ILLEGAL_STATE = BASE_AUDIO_ERR_OFFSET - 1;

/** Invalid parameter */
const int32_t  ERR_INVALID_PARAM = BASE_AUDIO_ERR_OFFSET - 2;

/** Early media preparation */
const int32_t  ERR_EARLY_PREPARE = BASE_AUDIO_ERR_OFFSET - 3;

/** Invalid operation */
const int32_t  ERR_INVALID_OPERATION = BASE_AUDIO_ERR_OFFSET - 4;

/** error operation failed */
const int32_t  ERR_OPERATION_FAILED = BASE_AUDIO_ERR_OFFSET - 5;

/** Buffer reading failed */
const int32_t  ERR_READ_BUFFER = BASE_AUDIO_ERR_OFFSET - 6;

/** Buffer writing failed */
const int32_t  ERR_WRITE_BUFFER = BASE_AUDIO_ERR_OFFSET - 7;

/**  Device not started */
const int32_t  ERR_NOT_STARTED = BASE_AUDIO_ERR_OFFSET - 8;

/**  Invalid Device handle */
const int32_t  ERR_INVALID_HANDLE = BASE_AUDIO_ERR_OFFSET - 9;

/**  unsupported operation */
const int32_t  ERR_NOT_SUPPORTED = BASE_AUDIO_ERR_OFFSET - 10;

/**  unsupported device */
const int32_t  ERR_DEVICE_NOT_SUPPORTED = BASE_AUDIO_ERR_OFFSET - 11;

/**  write operation failed */
const int32_t  ERR_WRITE_FAILED = BASE_AUDIO_ERR_OFFSET - 12;

/**  read operation failed */
const int32_t  ERR_READ_FAILED = BASE_AUDIO_ERR_OFFSET - 13;

/**  device init failed */
const int32_t  ERR_DEVICE_INIT = BASE_AUDIO_ERR_OFFSET - 14;

/** Invalid data size that has been read */
const int32_t  ERR_INVALID_READ = BASE_AUDIO_ERR_OFFSET - 15;

/** Invalid data size that has been written */
const int32_t  ERR_INVALID_WRITE = BASE_AUDIO_ERR_OFFSET - 16;

/** set invalid index < 0 */
const int32_t  ERR_INVALID_INDEX = BASE_AUDIO_ERR_OFFSET - 17;

/** Unknown error */
const int32_t  ERR_UNKNOWN = BASE_AUDIO_ERR_OFFSET - 200;
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_ERRORS_H
