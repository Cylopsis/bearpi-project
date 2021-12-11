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

#include "media_library_utils.h"

using namespace std;

namespace OHOS {
namespace Media {
MediaType MediaLibraryUtils::requestedMediaType = MEDIA_TYPE_DEFAULT;
pthread_mutex_t (MediaLibraryUtils::mutexLock) = PTHREAD_MUTEX_INITIALIZER;

vector<unique_ptr<AudioAsset>>& MediaLibraryUtils::GetAudioAssetsInternal()
{
    static vector<unique_ptr<AudioAsset>> audioAsset;
    return audioAsset;
}

vector<unique_ptr<ImageAsset>>& MediaLibraryUtils::GetImageAssetsInternal()
{
    static vector<unique_ptr<ImageAsset>> imageAsset;
    return imageAsset;
}

vector<unique_ptr<MediaAsset>>& MediaLibraryUtils::GetMediaAssetsInternal()
{
    static vector<unique_ptr<MediaAsset>> mediaAsset;
    return mediaAsset;
}

vector<unique_ptr<VideoAsset>>& MediaLibraryUtils::GetVideoAssetsInternal()
{
    static vector<unique_ptr<VideoAsset>> videoAsset;
    return videoAsset;
}
}  // namespace Media
}  // namespace OHOS