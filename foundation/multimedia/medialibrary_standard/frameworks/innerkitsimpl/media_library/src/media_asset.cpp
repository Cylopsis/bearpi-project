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

#include "media_asset.h"

#include "media_file_utils.h"
#include "media_lib_service_const.h"
#include "media_log.h"

using namespace std;

namespace OHOS {
namespace Media {
MediaAsset::MediaAsset()
{
    id_ = DEFAULT_MEDIA_ID;
    albumId_ = DEFAULT_ALBUM_ID;
    albumName_ = DEFAULT_ALBUM_NAME;
    size_ = DEFAULT_MEDIA_SIZE;
    uri_ = DEFAULT_MEDIA_URI;
    mediaType_ = DEFAULT_MEDIA_TYPE;
    name_ = DEFAULT_MEDIA_NAME;
    dateAdded_ = DEFAULT_MEDIA_DATE_ADDED;
    dateModified_ = DEFAULT_MEDIA_DATE_MODIFIED;
}

MediaAsset::~MediaAsset() = default;

MediaType MediaAsset::GetMediaType(const std::string &filePath)
{
    MediaType mediaType = MEDIA_TYPE_FILE;

    if (filePath.size() == 0) {
        return MEDIA_TYPE_DEFAULT;
    }

    size_t dotIndex = filePath.rfind(DOT_CHAR);
    if (dotIndex != string::npos) {
        string extension = filePath.substr(dotIndex + 1, filePath.length() - dotIndex);
        transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        if (SUPPORTED_AUDIO_FORMATS_SET.find(extension) != SUPPORTED_AUDIO_FORMATS_SET.end()) {
            mediaType = MEDIA_TYPE_AUDIO;
        } else if (SUPPORTED_VIDEO_FORMATS_SET.find(extension)
                   != SUPPORTED_VIDEO_FORMATS_SET.end()) {
            mediaType = MEDIA_TYPE_VIDEO;
        } else if (SUPPORTED_IMAGE_FORMATS_SET.find(extension)
                   != SUPPORTED_IMAGE_FORMATS_SET.end()) {
            mediaType = MEDIA_TYPE_IMAGE;
        } else {
            mediaType = MEDIA_TYPE_FILE;
        }
    }

    return mediaType;
}

bool MediaAsset::CreateMediaAsset(AssetType assetType)
{
    bool errCode = false;
    MediaType mediaType;
    string dirPath = ROOT_MEDIA_DIR + SLASH_CHAR + albumName_
                    + SLASH_CHAR;

    uri_ = ROOT_MEDIA_DIR + SLASH_CHAR + name_;

    if (!(name_.empty())) {
        mediaType = GetMediaType(name_);
        if ((assetType == ASSET_AUDIO && mediaType == MEDIA_TYPE_AUDIO) ||
            (assetType == ASSET_VIDEO && mediaType == MEDIA_TYPE_VIDEO) ||
            (assetType == ASSET_IMAGE && mediaType == MEDIA_TYPE_IMAGE)) {
            errCode = true;
            if (!(albumName_.empty())) {
                uri_ = dirPath + name_;
                if (!(MediaFileUtils::IsDirectory(dirPath))) {
                    errCode = MediaFileUtils::CreateDirectory(dirPath);
                }
            }

            if (errCode) {
                errCode = MediaFileUtils::CreateFile(uri_);
            }
        }
    }

    return errCode;
}

bool MediaAsset::DeleteMediaAsset()
{
    return MediaFileUtils::DeleteFile(uri_);
}

bool MediaAsset::ModifyMediaAsset(const MediaAsset &mediaAsset)
{
    string fileExtnSrc = "";
    string fileExtnDst = "";
    bool errCode = false;
    size_t  dotIndexDst = name_.find(".");
    size_t dotIndexSrc = mediaAsset.name_.find(".");

    if (uri_.empty()) {
        MEDIA_ERR_LOG("MediaAsset ModifyMediaAsset:Uri empty");
        return false;
    }

    if (dotIndexDst != string::npos) {
        if (name_.size() > dotIndexDst) {
            fileExtnDst = name_.substr(dotIndexDst);
            transform(fileExtnDst.begin(), fileExtnDst.end(), fileExtnDst.begin(), ::tolower);
        }
    }

    if (dotIndexSrc != string::npos) {
        if (mediaAsset.name_.size() > dotIndexSrc) {
            fileExtnSrc = mediaAsset.name_.substr(dotIndexSrc);
            transform(fileExtnSrc.begin(), fileExtnSrc.end(), fileExtnSrc.begin(), ::tolower);
        }
    }

    if (fileExtnSrc == fileExtnDst) {
        errCode = MediaFileUtils::MoveFile(mediaAsset.uri_, uri_);
    }

    return errCode;
}

bool MediaAsset::CopyMediaAsset(const MediaAsset &mediaAsset)
{
    return MediaFileUtils::CopyFile(mediaAsset.uri_, uri_);
}
}  // namespace Media
}  // namespace OHOS
