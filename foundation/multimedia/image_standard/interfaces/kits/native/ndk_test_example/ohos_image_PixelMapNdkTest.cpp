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

#include "hilog/log.h"
#include "image_pixel_map.h"
#include "jkit_utils.h"
#include "log_tags.h"
#include "media_errors.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::Media;

static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "PixelMapNdkTest_JNI" };
static constexpr int32_t PIXEL_OFFSET = 588 * 342 + 360;
static jclass g_imageNdkTestClass;

static bool InitImageNdkTest(JNIEnv *env)
{
    jclass imageClazz = env->FindClass("ohos/media/image/PixelMapNdkTest");
    if (imageClazz == nullptr) {
        HiLog::Error(LABEL, "find PixelMapNdkTest class fail");
        return false;
    }
    g_imageNdkTestClass = static_cast<jclass>(env->NewGlobalRef(imageClazz));
    env->DeleteLocalRef(imageClazz);
    return true;
}

void ohos_media_image_PixelMapNdkTest_nativeInit(JNIEnv *env, jclass thiz)
{
    HiLog::Debug(LABEL, "nativeInit begin");
    if (!InitImageNdkTest(env)) {
        HiLog::Error(LABEL, "nativeInit InitImageNdkTest failed");
        return;
    }
    HiLog::Debug(LABEL, "nativeInit end");
}

static jint ohos_media_image_PixelMapNdkTest_ValidPixelMapInfo(JNIEnv *env, jobject thiz, jobject jpixelMap,
                                                               jint width, jint height, jboolean is565)
{
    HiLog::Debug(LABEL, "input:width[%{public}u], height[%{public}u]", static_cast<uint32_t>(width),
                 static_cast<uint32_t>(height));
    OhosPixelMapInfo info;
    int32_t err = GetImageInfo(env, jpixelMap, info);
    HiLog::Debug(LABEL, "output:width[%{public}u], height[%{public}u], format[%{public}d], err[%{public}d]", info.width,
                 info.height, info.pixelFormat, err);
    int32_t format = is565 ? OHOS_PIXEL_MAP_FORMAT_RGB_565 : OHOS_PIXEL_MAP_FORMAT_RGBA_8888;
    if (err == OHOS_IMAGE_RESULT_SUCCESS && format == info.pixelFormat &&
        static_cast<uint32_t>(width) == info.width && static_cast<uint32_t>(height) == info.height) {
        return OHOS_IMAGE_RESULT_SUCCESS;
    }
    return OHOS_IMAGE_RESULT_BAD_PARAMETER;
}

static jint ohos_media_image_PixelMapNdkTest_AccessPixels(JNIEnv *env, jobject thiz, jobject jpixelMap)
{
    void *pixels = nullptr;
    return AccessPixels(env, jpixelMap, &pixels);
}

static jint ohos_media_image_PixelMapNdkTest_ReadPixelsValue(JNIEnv *env, jobject thiz, jobject jpixelMap)
{
    void *pixels = nullptr;
    int32_t err = AccessPixels(env, jpixelMap, &pixels);
    if (err != OHOS_IMAGE_RESULT_SUCCESS) {
        return OHOS_IMAGE_RESULT_BAD_PARAMETER;
    }
    uint32_t *offset = static_cast<uint32_t *>(pixels) + PIXEL_OFFSET;
    return *offset;
}

static jint ohos_media_image_PixelMapNdkTest_UnAccessPixels(JNIEnv *env, jobject thiz, jobject jpixelMap)
{
    return UnAccessPixels(env, jpixelMap);
}

static jint ohos_media_image_PixelMapNdkTest_GetPixelFormat(JNIEnv *env, jobject thiz, jobject jpixelMap)
{
    OhosPixelMapInfo info;
    info.pixelFormat = OHOS_PIXEL_MAP_FORMAT_NONE;
    int err = 0;
    err = GetImageInfo(env, jpixelMap, info);
    if (err != OHOS_IMAGE_RESULT_SUCCESS) {
        return OHOS_PIXEL_MAP_FORMAT_NONE;
    }
    return info.pixelFormat;
}

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void *>(ohos_media_image_PixelMapNdkTest_nativeInit) },
    { "nativeValidatePixelMapInfo", "(Lohos/media/image/PixelMap;IIZ)I",
      reinterpret_cast<void *>(ohos_media_image_PixelMapNdkTest_ValidPixelMapInfo) },
    { "nativeAccessPixels", "(Lohos/media/image/PixelMap;)I",
      (void *)ohos_media_image_PixelMapNdkTest_AccessPixels },
    { "nativeReadPixelsValue", "(Lohos/media/image/PixelMap;)I",
      (void *)ohos_media_image_PixelMapNdkTest_ReadPixelsValue },
    { "nativeUnAccessPixels", "(Lohos/media/image/PixelMap;)I",
      (void *)ohos_media_image_PixelMapNdkTest_UnAccessPixels },
    { "nativeGetPixelFormat", "(Lohos/media/image/PixelMap;)I",
      (void *)ohos_media_image_PixelMapNdkTest_GetPixelFormat },
};

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv *env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_4) != JNI_OK) {
        return ERROR;
    }
    int ret = JkitRegisterNativeMethods(env, "ohos/media/image/PixelMapNdkTest", METHODS, ARRCOUNT(METHODS));
    if (ret == JNI_ERR) {
        return ERROR;
    }
    Jkit::nativeInit(vm);
    return JNI_VERSION_1_4;
}