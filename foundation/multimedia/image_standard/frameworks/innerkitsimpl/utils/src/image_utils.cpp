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

#include "image_utils.h"
#include <cmath>
#include <sys/stat.h>
#include "image_log.h"
#include "media_errors.h"
#include "plugin_server.h"
#ifdef _WIN32
#include <stdlib.h>
#endif

namespace OHOS {
namespace Media {
using namespace OHOS::HiviewDFX;
using namespace std;
using namespace MultimediaPlugin;

constexpr int32_t ALPHA8_BYTES = 1;
constexpr int32_t RGB565_BYTES = 2;
constexpr int32_t RGB888_BYTES = 3;
constexpr int32_t ARGB8888_BYTES = 4;
constexpr int32_t RGBA_F16_BYTES = 8;
constexpr int32_t NV21_BYTES = 2;  // Each pixel is sorted on 3/2 bytes.
constexpr float EPSILON = 1e-6;
constexpr int MAX_DIMENSION = INT32_MAX >> 2;
static bool g_pluginRegistered = false;

bool ImageUtils::GetFileSize(const string &pathName, size_t &size)
{
    if (pathName.empty()) {
        IMAGE_LOGE("[ImageUtil]input parameter exception.");
        return false;
    }
    struct stat statbuf;
    int ret = stat(pathName.c_str(), &statbuf);
    if (ret != 0) {
        IMAGE_LOGE("[ImageUtil]get the file size failed, ret:%{public}d.", ret);
        return false;
    }
    size = statbuf.st_size;
    return true;
}

bool ImageUtils::GetInputStreamSize(istream &inputStream, size_t &size)
{
    if (inputStream.rdbuf() == nullptr) {
        IMAGE_LOGE("[ImageUtil]input parameter exception.");
        return false;
    }
    size_t original = inputStream.tellg();
    inputStream.seekg(0, ios_base::end);
    size = inputStream.tellg();
    inputStream.seekg(original);
    return true;
}

int32_t ImageUtils::GetPixelBytes(const PixelFormat &pixelFormat)
{
    int pixelBytes = 0;
    switch (pixelFormat) {
        case PixelFormat::ARGB_8888:
        case PixelFormat::BGRA_8888:
        case PixelFormat::RGBA_8888:
        case PixelFormat::CMYK:
            pixelBytes = ARGB8888_BYTES;
            break;
        case PixelFormat::ALPHA_8:
            pixelBytes = ALPHA8_BYTES;
            break;
        case PixelFormat::RGB_888:
            pixelBytes = RGB888_BYTES;
            break;
        case PixelFormat::RGB_565:
            pixelBytes = RGB565_BYTES;
            break;
        case PixelFormat::RGBA_F16:
            pixelBytes = RGBA_F16_BYTES;
            break;
        case PixelFormat::NV21:
        case PixelFormat::NV12:
            pixelBytes = NV21_BYTES;  // perl pixel 1.5 Bytes but return int so return 2
            break;
        default:
            IMAGE_LOGE("[ImageUtil]get pixel bytes failed, pixelFormat:%{public}d.", static_cast<int32_t>(pixelFormat));
            break;
    }
    return pixelBytes;
}

uint32_t ImageUtils::RegisterPluginServer()
{
#ifdef _WIN32
    vector<string> pluginPaths = { "" };
#elif defined(_APPLE)
    vector<string> pluginPaths = { "./" };
#else
    vector<string> pluginPaths = { "/system/etc/multimediaplugin/image" };
#endif
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    uint32_t result = pluginServer.Register(std::move(pluginPaths));
    if (result != SUCCESS) {
        IMAGE_LOGE("[ImageUtil]failed to register plugin server, ERRNO: %{public}u.", result);
    } else {
        g_pluginRegistered = true;
        IMAGE_LOGI("[ImageUtil]success to register plugin server");
    }
    return result;
}

PluginServer& ImageUtils::GetPluginServer()
{
    if (!g_pluginRegistered) {
        uint32_t result = RegisterPluginServer();
        if (result != SUCCESS) {
            IMAGE_LOGI("[ImageUtil]failed to register plugin server, ERRNO: %{public}u.", result);
        }
    }
    return DelayedRefSingleton<PluginServer>::GetInstance();
}

bool ImageUtils::PathToRealPath(const string &path, string &realPath)
{
    if (path.empty()) {
        IMAGE_LOGE("path is empty!");
        return false;
    }

    if ((path.length() >= PATH_MAX)) {
        IMAGE_LOGE("path len is error, the len is: [%{public}lu]", static_cast<unsigned long>(path.length()));
        return false;
    }

    char tmpPath[PATH_MAX] = { 0 };

#ifdef _WIN32
    if (_fullpath(tmpPath, path.c_str(), path.length()) == nullptr) {
        IMAGE_LOGW("path to _fullpath error");
    }
#else
    if (realpath(path.c_str(), tmpPath) == nullptr) {
        IMAGE_LOGE("path to realpath is nullptr");
        return false;
    }
#endif

    realPath = tmpPath;
    return true;
}

bool ImageUtils::FloatCompareZero(float src)
{
    return fabs(src - 0) < EPSILON;
}

AlphaType ImageUtils::GetValidAlphaTypeByFormat(const AlphaType &dstType, const PixelFormat &format)
{
    switch (format) {
        case PixelFormat::RGBA_8888:
        case PixelFormat::BGRA_8888:
        case PixelFormat::ARGB_8888:
        case PixelFormat::RGBA_F16: {
            break;
        }
        case PixelFormat::ALPHA_8: {
            if (dstType != AlphaType::IMAGE_ALPHA_TYPE_PREMUL) {
                return AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
            }
            break;
        }
        case PixelFormat::RGB_888:
        case PixelFormat::RGB_565: {
            if (dstType != AlphaType::IMAGE_ALPHA_TYPE_OPAQUE) {
                return AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
            }
            break;
        }
        case PixelFormat::NV21:
        case PixelFormat::NV12:
        case PixelFormat::CMYK:
        default: {
            HiLog::Error(LABEL, "GetValidAlphaTypeByFormat unsupport the format(%{public}d).", format);
            return AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN;
        }
    }
    return dstType;
}

bool ImageUtils::IsValidImageInfo(const ImageInfo &info)
{
    if (info.size.width <= 0 || info.size.height <= 0 || info.size.width > MAX_DIMENSION ||
        info.size.height > MAX_DIMENSION) {
        HiLog::Error(LABEL, "width(%{public}d) or height(%{public}d) is invalid.", info.size.width, info.size.height);
        return false;
    }
    if (info.pixelFormat == PixelFormat::UNKNOWN || info.alphaType == AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN) {
        HiLog::Error(LABEL, "check pixelformat and alphatype is invalid.");
        return false;
    }
    return true;
}

bool ImageUtils::CheckMulOverflow(int32_t width, int32_t bytesPerPixel)
{
    if (width == 0 || bytesPerPixel == 0) {
        HiLog::Error(LABEL, "para is 0");
        return true;
    }
    int64_t rowSize = static_cast<uint64_t>(width) * bytesPerPixel;
    if ((rowSize / width) != bytesPerPixel) {
        HiLog::Error(LABEL, "width * bytesPerPixel overflow!");
        return true;
    }
    return false;
}

bool ImageUtils::CheckMulOverflow(int32_t width, int32_t height, int32_t bytesPerPixel)
{
    if (width == 0 || height == 0 || bytesPerPixel == 0) {
        HiLog::Error(LABEL, "para is 0");
        return true;
    }
    int64_t rectSize = static_cast<uint64_t>(width) * height;
    if ((rectSize / width) != height) {
        HiLog::Error(LABEL, "width * height overflow!");
        return true;
    }
    int64_t bufferSize = rectSize * bytesPerPixel;
    if ((bufferSize / bytesPerPixel) != rectSize) {
        HiLog::Error(LABEL, "bytesPerPixel overflow!");
        return true;
    }
    return false;
}
} // namespace Media
} // namespace OHOS