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

#include "image_source_util.h"
#include <fstream>
#include <string>
#include "directory_ex.h"
#include "hilog/log.h"
#include "image_packer.h"
#include "image_type.h"
#include "image_utils.h"
#include "log_tags.h"
#include "media_errors.h"
#include "pixel_map.h"

using namespace OHOS::Media;
using namespace OHOS::HiviewDFX;

static constexpr OHOS::HiviewDFX::HiLogLabel LABEL_TEST = { LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "ImageSourceUtil" };
int64_t PackImage(const std::string &filePath, std::unique_ptr<PixelMap> pixelMap)
{
    ImagePacker imagePacker;
    PackOption option;
    option.format = "image/jpeg";
    option.quality = 100;
    option.numberHint = 1;
    std::set<std::string> formats;
    uint32_t ret = imagePacker.GetSupportedFormats(formats);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL_TEST, "image packer get supported format failed, ret=%{public}u.", ret);
        return 0;
    }
    imagePacker.StartPacking(filePath, option);
    imagePacker.AddImage(*pixelMap);
    int64_t packedSize = 0;
    imagePacker.FinalizePacking(packedSize);
    HiLog::Debug(LABEL_TEST, "packedSize=%{public}lld.", static_cast<long long>(packedSize));
    return packedSize;
}

bool ReadFileToBuffer(const std::string &filePath, uint8_t *buffer, size_t bufferSize)
{
    std::string realPath;
    if (!OHOS::PathToRealPath(filePath, realPath)) {
        HiLog::Error(LABEL_TEST, "file path to real path failed, file path=%{public}s.", filePath.c_str());
        return false;
    }
    FILE *fp = fopen(realPath.c_str(), "rb");
    if (fp == nullptr) {
        HiLog::Error(LABEL_TEST, "open file failed, real path=%{public}s.", realPath.c_str());
        return false;
    }
    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (bufferSize < fileSize) {
        HiLog::Error(LABEL_TEST, "buffer size:(%{public}zu) is smaller than file size:(%{public}zu).", bufferSize,
                     fileSize);
        fclose(fp);
        return false;
    }
    size_t retSize = fread(buffer, 1, fileSize, fp);
    if (retSize != fileSize) {
        HiLog::Error(LABEL_TEST, "read file result size = %{public}zu, size = %{public}zu.", retSize, fileSize);
        fclose(fp);
        return false;
    }
    fclose(fp);
    return true;
}