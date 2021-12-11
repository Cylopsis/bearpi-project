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

#include "image_pixel_map.h"
#include "ohos_image_pixelmap.h"

int32_t GetImageInfo(JNIEnv *env, jobject pixelMapObject, OhosPixelMapInfo &info)
{
    if (env == nullptr || pixelMapObject == nullptr) {
        return OHOS_IMAGE_RESULT_BAD_PARAMETER;
    }

    OHOS::Media::ohos_media_image_GetImageInfo(env, pixelMapObject, info);
    return OHOS_IMAGE_RESULT_SUCCESS;
}

int32_t AccessPixels(JNIEnv *env, jobject pixelMapObject, void **addrPtr)
{
    if (env == nullptr || pixelMapObject == nullptr) {
        return OHOS_IMAGE_RESULT_BAD_PARAMETER;
    }

    void *addr = OHOS::Media::ohos_media_image_AccessPixels(env, pixelMapObject);
    if (!addr) {
        return OHOS_IMAGE_RESULT_JNI_EXCEPTION;
    }

    if (addrPtr) {
        *addrPtr = addr;
    }
    return OHOS_IMAGE_RESULT_SUCCESS;
}

int32_t UnAccessPixels(JNIEnv *env, jobject pixelMapObject)
{
    if (env == nullptr || pixelMapObject == nullptr) {
        return OHOS_IMAGE_RESULT_BAD_PARAMETER;
    }

    bool unAccessed = OHOS::Media::ohos_media_image_UnAccessPixels(env, pixelMapObject);
    if (!unAccessed) {
        return OHOS_IMAGE_RESULT_JNI_EXCEPTION;
    }
    return OHOS_IMAGE_RESULT_SUCCESS;
}
