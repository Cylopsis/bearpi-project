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

#ifndef PIXEL_MAP_JNI_UTILS_H
#define PIXEL_MAP_JNI_UTILS_H

#include "hilog/log.h"
#include "jkit_utils.h"
#include "log_tags.h"

namespace OHOS {
namespace Media {
class PixelMapJniUtilsAdapter {
public:
    static jarray GetBufferBaseArray(JNIEnv *env, jobject jbuffer);
    static bool GetBufferBaseArrayOffset(JNIEnv *env, jobject jbuffer, jint &offset);
};
} // namespace Media
} // namespace OHOS

#endif // PIXEL_MAP_JNI_UTILS_H