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

#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <cstdio>
#include <istream>
#include <string>
#include "image_type.h"
#include "plugin_server.h"

namespace OHOS {
namespace Media {
const std::string IMAGE_ENCODE_FORMAT = "encodeFormat";

class ImageUtils {
public:
    static bool GetFileSize(const std::string &pathName, size_t &size);
    static bool GetInputStreamSize(std::istream &inputStream, size_t &size);
    static int32_t GetPixelBytes(const PixelFormat &pixelFormat);
    static bool PathToRealPath(const std::string &path, std::string &realPath);
    static bool FloatCompareZero(float src);
    static AlphaType GetValidAlphaTypeByFormat(const AlphaType &dstType, const PixelFormat &format);
    static bool IsValidImageInfo(const ImageInfo &info);
    static MultimediaPlugin::PluginServer& GetPluginServer();
    static bool CheckMulOverflow(int32_t width, int32_t bytesPerPixel);
    static bool CheckMulOverflow(int32_t width, int32_t height, int32_t bytesPerPixel);

private:
    static uint32_t RegisterPluginServer();
};
} // namespace Media
} // namespace OHOS
#endif // IMAGE_UTILS_H