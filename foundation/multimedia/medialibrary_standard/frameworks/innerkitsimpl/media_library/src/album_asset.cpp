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

#include "album_asset.h"
#include "media_file_utils.h"
#include "media_log.h"

using namespace std;

namespace OHOS {
namespace Media {
AlbumAsset::AlbumAsset()
{
    albumId_ = DEFAULT_ALBUM_ID;
    albumName_ = DEFAULT_ALBUM_NAME;
}

AlbumAsset::~AlbumAsset() = default;

bool AlbumAsset::CreateAlbumAsset()
{
    string albumUri = ROOT_MEDIA_DIR + SLASH_CHAR + albumName_;
    if (!(MediaFileUtils::IsDirectory(albumUri))) {
        return MediaFileUtils::CreateDirectory(albumUri);
    } else {
        MEDIA_ERR_LOG("Cannot create album that already exists");
        return false;
    }
}

bool AlbumAsset::DeleteAlbumAsset(const string &albumUri)
{
    return MediaFileUtils::DeleteDir(albumUri);
}

bool AlbumAsset::ModifyAlbumAsset(const AlbumAsset &albumAsset, const string &albumUri)
{
    string newAlbumUri;
    string oldAlbumUri;
    size_t slashIndex;
    bool errCode = false;

    oldAlbumUri = albumUri;
    slashIndex = albumUri.rfind("/");
    if (slashIndex != string::npos) {
        newAlbumUri = albumUri.substr(0, slashIndex)  + SLASH_CHAR + albumName_;
        errCode =  MediaFileUtils::RenameDir(oldAlbumUri, newAlbumUri);
    }

    return errCode;
}
}  // namespace Media
}  // namespace OHOS
