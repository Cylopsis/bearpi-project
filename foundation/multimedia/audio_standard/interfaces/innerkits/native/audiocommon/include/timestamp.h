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
#ifndef TIMESTAMP_H
#define TIMESTAMP_H
#ifdef __MUSL__
#include <sys/time.h>

#include <stdint.h>
#endif // __MUSL__
#include <unistd.h>

namespace OHOS {
namespace AudioStandard {
/**
 * @brief Represents Timestamp information, including the frame position information and high-resolution time source.
 */
class Timestamp {
public:
    Timestamp() : framePosition(0)
    {
        time.tv_sec = 0;
        time.tv_nsec = 0;
    }
    virtual ~Timestamp() = default;
    uint32_t framePosition;
    struct timespec time;

    /**
     * @brief Enumerates the time base of this <b>Timestamp</b>. Different timing methods are supported.
     *
     */
    enum Timestampbase {
        /** Monotonically increasing time, excluding the system sleep time */
        MONOTONIC = 0
    };
};
} // namespace AudioStandard
} // namespace OHOS
#endif // TIMESTAMP_H
