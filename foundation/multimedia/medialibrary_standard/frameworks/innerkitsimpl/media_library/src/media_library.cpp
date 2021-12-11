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

#include "media_library.h"

#include <cerrno>
#include <dirent.h>
#include <ftw.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_set>

#include "media_lib_service_const.h"
#include "media_library_utils.h"
#include "media_log.h"

using namespace std;

namespace OHOS {
namespace Media {
#define CHK_NULL_RETURN(ptr) \
do { \
    if ((ptr) == nullptr) { \
        return MEDIA_SCAN_FAIL; \
    } \
} while (0)

#define CHK_NULL_RETURN_ERROR(ptr, ret) \
do { \
    if ((ptr) == nullptr) { \
        return ret; \
    } \
} while (0)

MediaLibrary::MediaLibrary() {}

MediaLibrary::~MediaLibrary() = default;

unique_ptr<MediaLibrary> MediaLibrary::GetMediaLibraryInstance()
{
    return make_unique<MediaLibrary>();
}

string GetNameFromUri(const string &uri)
{
    string fileName = DEFAULT_MEDIA_NAME;

    size_t slashIndex = uri.rfind(SLASH_CHAR);
    if (slashIndex != string::npos) {
        if (uri.size() > slashIndex) {
            fileName = uri.substr(slashIndex + 1, uri.length() - slashIndex);
        }
    }

    return fileName;
}

unique_ptr<MediaAsset> UpdateMediaData(const string &assetUri, const struct stat &statInfo)
{
    if (assetUri.empty()) {
        MEDIA_ERR_LOG("Uri is empty!");
        return nullptr;
    }

    unique_ptr<MediaAsset> mediaAsset = make_unique<MediaAsset>();
    mediaAsset->uri_ = assetUri;
    mediaAsset->name_ = GetNameFromUri(assetUri);
    mediaAsset->mediaType_ = MEDIA_TYPE_FILE;
    mediaAsset->size_ = statInfo.st_size;
    mediaAsset->dateModified_ = statInfo.st_mtime * MILLISECONDS;
    mediaAsset->dateAdded_ = statInfo.st_ctime * MILLISECONDS;

    return mediaAsset;
}

unique_ptr<AudioAsset> UpdateAudioData(const string &assetUri, const struct stat &statInfo)
{
    if (assetUri.empty()) {
        MEDIA_ERR_LOG("Uri is empty!");
        return nullptr;
    }

    unique_ptr<AudioAsset> audioAsset = make_unique<AudioAsset>();
    audioAsset->uri_ = assetUri;
    audioAsset->name_ = GetNameFromUri(assetUri);
    audioAsset->mediaType_ = MEDIA_TYPE_AUDIO;
    audioAsset->size_ = statInfo.st_size;
    audioAsset->dateModified_ = statInfo.st_mtime * MILLISECONDS;
    audioAsset->dateAdded_ = statInfo.st_ctime * MILLISECONDS;

    return audioAsset;
}

unique_ptr<VideoAsset> UpdateVideoData(const string &assetUri, const struct stat &statInfo)
{
    if (assetUri.empty()) {
        MEDIA_ERR_LOG("Uri is empty!");
        return nullptr;
    }

    unique_ptr<VideoAsset> videoAsset = make_unique<VideoAsset>();
    videoAsset->uri_ = assetUri;
    videoAsset->name_ = GetNameFromUri(assetUri);
    videoAsset->mediaType_ = MEDIA_TYPE_VIDEO;
    videoAsset->size_ = statInfo.st_size;
    videoAsset->dateModified_ = statInfo.st_mtime * MILLISECONDS;
    videoAsset->dateAdded_ = statInfo.st_ctime * MILLISECONDS;

    return videoAsset;
}

unique_ptr<ImageAsset> UpdateImageData(const string &assetUri, const struct stat &statInfo)
{
    if (assetUri.empty()) {
        MEDIA_ERR_LOG("Uri is empty!");
        return nullptr;
    }

    unique_ptr<ImageAsset> imageAsset = make_unique<ImageAsset>();
    imageAsset->uri_ = assetUri;
    imageAsset->name_ = GetNameFromUri(assetUri);
    imageAsset->mediaType_ = MEDIA_TYPE_IMAGE;
    imageAsset->size_ = statInfo.st_size;
    imageAsset->dateModified_ = statInfo.st_mtime * MILLISECONDS;
    imageAsset->dateAdded_ = statInfo.st_ctime * MILLISECONDS;

    return imageAsset;
}

void ScanMediaAssetInfo(MediaType mediaType, const string &path, const struct stat *statInfo)
{
    switch (mediaType) {
        case MEDIA_TYPE_AUDIO: {
            if (MediaLibraryUtils::requestedMediaType == MEDIA_TYPE_AUDIO) {
                unique_ptr<AudioAsset> audioAsset = UpdateAudioData(path, *statInfo);
                if (audioAsset != nullptr) {
                    vector<unique_ptr<AudioAsset>> &audioAssetList
                        = MediaLibraryUtils::GetAudioAssetsInternal();
                    audioAsset->id_ = (int32_t)audioAssetList.size();
                    audioAssetList.push_back(std::move(audioAsset));
                }
            }
            break;
        }
        case MEDIA_TYPE_VIDEO: {
            if (MediaLibraryUtils::requestedMediaType == MEDIA_TYPE_VIDEO) {
                unique_ptr<VideoAsset> videoAsset = UpdateVideoData(path, *statInfo);
                if (videoAsset != nullptr) {
                    vector<unique_ptr<VideoAsset>> &videoAssetList
                        = MediaLibraryUtils::GetVideoAssetsInternal();
                    videoAsset->id_ = (int32_t)videoAssetList.size();
                    videoAssetList.push_back(std::move(videoAsset));
                }
            }
            break;
        }
        case MEDIA_TYPE_IMAGE: {
            if (MediaLibraryUtils::requestedMediaType == MEDIA_TYPE_IMAGE) {
                unique_ptr<ImageAsset> imageAsset = UpdateImageData(path, *statInfo);
                if (imageAsset != nullptr) {
                    vector<unique_ptr<ImageAsset>> &imageAssetList
                        = MediaLibraryUtils::GetImageAssetsInternal();
                    imageAsset->id_ = (int32_t)imageAssetList.size();
                    imageAssetList.push_back(std::move(imageAsset));
                }
            }
            break;
        }
        default:
            MEDIA_ERR_LOG("ScanMediaAssetInfo :Unknown media type");
            break;
    }

    return;
}

int ScanCallback(const char *path, const struct stat *statInfo, int typeFlag, struct FTW *ftwInfo)
{
    if ((path == nullptr) || (statInfo == nullptr)) {
        MEDIA_ERR_LOG("Invalid arguments!");
        return 0;
    }

    if (typeFlag == FTW_F) {
        MediaType mediaType = MediaAsset::GetMediaType(path);

        // Add all files in media assets vector
        if (MediaLibraryUtils::requestedMediaType == MEDIA_TYPE_MEDIA) {
            unique_ptr<MediaAsset> mediaAsset = UpdateMediaData(path, *statInfo);
            if (mediaAsset != nullptr) {
                vector<unique_ptr<MediaAsset>> &mediaAssetList
                                        = MediaLibraryUtils::GetMediaAssetsInternal();
                mediaAsset->id_ = (int32_t)mediaAssetList.size();
                mediaAsset->mediaType_ = mediaType;
                mediaAssetList.push_back(std::move(mediaAsset));
            }
        }
        ScanMediaAssetInfo(mediaType, string(path), statInfo);
    }

    return 0;
}

void ClearAssets()
{
    vector<unique_ptr<ImageAsset>> &imageAssets
                                    = MediaLibraryUtils::GetImageAssetsInternal();
    imageAssets.clear();

    vector<unique_ptr<VideoAsset>> &videoAssets
                                    = MediaLibraryUtils::GetVideoAssetsInternal();
    videoAssets.clear();

    vector<unique_ptr<AudioAsset>> &audioAssets
                                    = MediaLibraryUtils::GetAudioAssetsInternal();
    audioAssets.clear();

    vector<unique_ptr<MediaAsset>> &mediaAssets
                                    = MediaLibraryUtils::GetMediaAssetsInternal();
    mediaAssets.clear();
}

int32_t ScanDir(const string &scanDir)
{
    int32_t ret = MEDIA_SCAN_SUCCESS;
    // By default use /data/media as scan directory
    string scanUri = ROOT_MEDIA_DIR;

    if (!scanDir.empty()) {
        scanUri = ROOT_MEDIA_DIR + SLASH_CHAR + scanDir;
    }

    // Clear all asset vectors before scanning
    ClearAssets();

    ret = nftw(scanUri.c_str(), ScanCallback, OPEN_FDS, FTW_PHYS);
    if (ret < 0) {
        ret = MEDIA_SCAN_FAIL;
    }

    return ret;
}

vector<string> GetSubDirectories(string rootDir)
{
    DIR *dirPath = nullptr;
    struct dirent *ent = nullptr;
    vector<string> dirList;
    string scanDir = ROOT_MEDIA_DIR;

    if (!rootDir.empty()) {
        scanDir = ROOT_MEDIA_DIR + SLASH_CHAR + rootDir;
    }

    if ((dirPath = opendir(scanDir.c_str())) != nullptr) {
        while ((ent = readdir(dirPath)) != nullptr) {
            if (ent->d_type == DT_DIR) {
                string currentDir(".");
                string parentDir("..");
                if ((currentDir.compare(ent->d_name) != 0)
                    && (parentDir.compare(ent->d_name) != 0)) {
                    string dirPath = scanDir + SLASH_CHAR + ent->d_name;
                    dirList.push_back(dirPath);
                }
            }
        }
        closedir(dirPath);
    }

    return dirList;
}

int32_t ScanAlbums(const unique_ptr<AlbumAsset> &albumAsset, string albumDir, int32_t requestedMediaType)
{
    int32_t ret = MEDIA_SCAN_FAIL;

    if (albumDir.empty()) {
        return ret;
    }

    DIR *dirPath = nullptr;
    struct dirent *ent = nullptr;
    if ((dirPath = opendir(albumDir.c_str())) != nullptr) {
        size_t slashIndex = albumDir.rfind(SLASH_CHAR);
        if (slashIndex != string::npos) {
            if (albumDir.size() > slashIndex) {
                albumAsset->albumName_ = albumDir.substr(slashIndex + 1, albumDir.length() - slashIndex);
            }
        }

        while ((ent = readdir(dirPath)) != nullptr) {
            if (ent->d_type == DT_REG) {
                string fileUri = albumDir + SLASH_CHAR + ent->d_name;
                struct stat statInfo {};
                if (stat(fileUri.c_str(), &statInfo) != 0) {
                    MEDIA_ERR_LOG("No stat info");
                    continue;
                }

                MediaType mediaType = MediaAsset::GetMediaType(fileUri);
                if ((mediaType == MEDIA_TYPE_IMAGE) &&
                    (requestedMediaType == MEDIA_TYPE_IMAGE)) {
                    unique_ptr<ImageAsset> imageAsset = UpdateImageData(fileUri, statInfo);
                    imageAsset->id_ = (int32_t)albumAsset->imageAssetList_.size();
                    albumAsset->imageAssetList_.push_back(std::move(imageAsset));
                } else if ((mediaType == MEDIA_TYPE_VIDEO) &&
                           (requestedMediaType == MEDIA_TYPE_VIDEO)) {
                    unique_ptr<VideoAsset> videoAsset = UpdateVideoData(fileUri, statInfo);
                    videoAsset->id_ = (int32_t)albumAsset->videoAssetList_.size();
                    albumAsset->videoAssetList_.push_back(std::move(videoAsset));
                }
            }
        }
        closedir(dirPath);
        ret = MEDIA_SCAN_SUCCESS;
    }

    return ret;
}

template<typename T>
void SortMediaAssetsByDateAdded(vector<unique_ptr<T>> &mediaAssetList)
{
    sort(mediaAssetList.begin(), mediaAssetList.end(),
        [](const unique_ptr<T> &mediaAsset1, const unique_ptr<T> &mediaAsset2) {
            return (mediaAsset1->dateAdded_ > mediaAsset2->dateAdded_);
        });
}

vector<unique_ptr<MediaAsset>> MediaLibrary::GetMediaAssets(string selection,
                                                            vector<string> selectionArgs)
{
    MediaLibraryUtils::requestedMediaType = MEDIA_TYPE_MEDIA;
    vector<unique_ptr<MediaAsset>> mediaAssets;

    pthread_mutex_lock(&(MediaLibraryUtils::mutexLock));

    if (ScanDir(selection) == MEDIA_SCAN_SUCCESS) {
        vector<unique_ptr<MediaAsset>> &mediaAssetsInternal
                                            = MediaLibraryUtils::GetMediaAssetsInternal();
        for (size_t i = 0; i < mediaAssetsInternal.size(); i++) {
            mediaAssets.push_back(std::move(mediaAssetsInternal[i]));
        }
        mediaAssetsInternal.clear();

        // Sort the Media Asset Vector based on Created Date
        SortMediaAssetsByDateAdded<MediaAsset>(mediaAssets);
    }
    pthread_mutex_unlock(&(MediaLibraryUtils::mutexLock));

    return mediaAssets;
}

bool IsAlbumEmpty(const string albumPath)
{
    bool errCode = false;
    DIR *dirPath = opendir(albumPath.c_str());
    if (dirPath != nullptr) {
        struct dirent *ent = nullptr;
        while ((ent = readdir(dirPath)) != nullptr) {
            // strcmp returns 0 if both strings are same.
            // The if condition will be true only if ent->d_name is neither . nor ..
            if ((strcmp(ent->d_name, ".") != 0) && (strcmp(ent->d_name, "..") != 0)) {
                MEDIA_ERR_LOG("Album is not empty");
                errCode = false;
                break;
            }
            errCode = true;
        }
        closedir(dirPath);
    }

    return errCode;
}

vector<unique_ptr<AlbumAsset>> MediaLibrary::GetAlbumAssets(string selection,
                                                            vector<string> selectionArgs,
                                                            int32_t requestedMediaType)
{
    vector<unique_ptr<AlbumAsset>> albumAssets;
    vector<string> albumSubDirList = GetSubDirectories(selection);

    for (size_t i = 0; i < albumSubDirList.size(); i++) {
        unique_ptr<AlbumAsset> albumAsset = make_unique<AlbumAsset>();

        if ((requestedMediaType == MEDIA_TYPE_IMAGE) && IsAlbumEmpty(albumSubDirList[i])) {
            size_t slashIndex = albumSubDirList[i].rfind(SLASH_CHAR);
            if ((slashIndex != string::npos) && (albumSubDirList[i].size() > slashIndex)) {
                albumAsset->albumName_ = albumSubDirList[i].substr(slashIndex + 1,
                    albumSubDirList[i].length() - slashIndex);
            }
            albumAssets.push_back(std::move(albumAsset));
            continue;
        }

        if (ScanAlbums(albumAsset, albumSubDirList[i], requestedMediaType) == MEDIA_SCAN_SUCCESS) {
            if (((requestedMediaType == MEDIA_TYPE_IMAGE) && (albumAsset->imageAssetList_.size() == 0)) ||
                ((requestedMediaType == MEDIA_TYPE_VIDEO) && (albumAsset->videoAssetList_.size() == 0))) {
                continue;
            }
            // Sort Album's Image Asset List
            SortMediaAssetsByDateAdded<ImageAsset>(albumAsset->imageAssetList_);

            // Sort Album's Video Asset List
            SortMediaAssetsByDateAdded<VideoAsset>(albumAsset->videoAssetList_);
            albumAssets.push_back(std::move(albumAsset));
        } else {
            MEDIA_ERR_LOG("MediaLibrary: Scan albums failed, return empty");
            break;
        }
    }

    return albumAssets;
}

vector<unique_ptr<AudioAsset>> MediaLibrary::GetAudioAssets(string selection,
                                                            vector<string> selectionArgs)
{
    MediaLibraryUtils::requestedMediaType = MEDIA_TYPE_AUDIO;
    vector<unique_ptr<AudioAsset>> audioAssets;

    pthread_mutex_lock(&(MediaLibraryUtils::mutexLock));

    if (ScanDir(selection) == MEDIA_SCAN_SUCCESS) {
        vector<unique_ptr<AudioAsset>> &audioAssetsInternal
                                            = MediaLibraryUtils::GetAudioAssetsInternal();
        for (size_t i = 0; i < audioAssetsInternal.size(); i++) {
            audioAssets.push_back(std::move(audioAssetsInternal[i]));
        }
        audioAssetsInternal.clear();

        // Sort the Audio Asset Vector based on Created Date
        SortMediaAssetsByDateAdded<AudioAsset>(audioAssets);
    }
    pthread_mutex_unlock(&(MediaLibraryUtils::mutexLock));

    return audioAssets;
}

vector<unique_ptr<VideoAsset>> MediaLibrary::GetVideoAssets(string selection,
                                                            vector<string> selectionArgs)
{
    MediaLibraryUtils::requestedMediaType = MEDIA_TYPE_VIDEO;
    vector<unique_ptr<VideoAsset>> videoAssets;

    pthread_mutex_lock(&(MediaLibraryUtils::mutexLock));

    if (ScanDir(selection) == MEDIA_SCAN_SUCCESS) {
        vector<unique_ptr<VideoAsset>> &videoAssetsInternal
                                            = MediaLibraryUtils::GetVideoAssetsInternal();
        for (size_t i = 0; i < videoAssetsInternal.size(); i++) {
            videoAssets.push_back(std::move(videoAssetsInternal[i]));
        }
        videoAssetsInternal.clear();

        // Sort the Video Asset Vector based on Created Date
        SortMediaAssetsByDateAdded<VideoAsset>(videoAssets);
    }
    pthread_mutex_unlock(&(MediaLibraryUtils::mutexLock));

    return videoAssets;
}

vector<unique_ptr<ImageAsset>> MediaLibrary::GetImageAssets(string selection,
                                                            vector<string> selectionArgs)
{
    MediaLibraryUtils::requestedMediaType = MEDIA_TYPE_IMAGE;
    vector<unique_ptr<ImageAsset>> imageAssets;

    pthread_mutex_lock(&(MediaLibraryUtils::mutexLock));

    if (ScanDir(selection) == MEDIA_SCAN_SUCCESS) {
        vector<unique_ptr<ImageAsset>> &imageAssetsInternal
                                            = MediaLibraryUtils::GetImageAssetsInternal();
        for (size_t i = 0; i < imageAssetsInternal.size(); i++) {
            imageAssets.push_back(std::move(imageAssetsInternal[i]));
        }
        imageAssetsInternal.clear();

        // Sort the Image Asset Vector based on Created Date
        SortMediaAssetsByDateAdded<ImageAsset>(imageAssets);
    }
    pthread_mutex_unlock(&(MediaLibraryUtils::mutexLock));

    return imageAssets;
}

bool MediaLibrary::CreateMediaAsset(AssetType assetType, const MediaAsset& crtMediaAsset)
{
    MediaAsset *mediaAsset = (MediaAsset *)&crtMediaAsset;

    return mediaAsset->CreateMediaAsset(assetType);
}

bool MediaLibrary::DeleteMediaAsset(AssetType assetType, const MediaAsset& delMediaAsset)
{
    MediaAsset *mediaAsset = (MediaAsset *)&delMediaAsset;

    return mediaAsset->DeleteMediaAsset();
}

bool MediaLibrary::ModifyMediaAsset(AssetType assetType, const MediaAsset& srcMediaAsset,
                                    const MediaAsset &dstAsset)
{
    MediaAsset *dstMediaAsset = (MediaAsset *)&dstAsset;

    return dstMediaAsset->ModifyMediaAsset(srcMediaAsset);
}

bool MediaLibrary::CopyMediaAsset(AssetType assetType, const MediaAsset& srcMediaAsset,
                                  const MediaAsset& dstAsset)
{
    MediaAsset *dstMediaAsset = (MediaAsset *)&dstAsset;

    return dstMediaAsset->CopyMediaAsset(srcMediaAsset);
}

bool MediaLibrary::CreateMediaAlbumAsset(AssetType assetType, const AlbumAsset& albmAsset)
{
    AlbumAsset *albumAsset = (AlbumAsset *)&albmAsset;

    return albumAsset->CreateAlbumAsset();
}

bool MediaLibrary::DeleteMediaAlbumAsset(AssetType assetType,  const AlbumAsset& albmAsset, const string &albumUri)
{
    AlbumAsset *albumAsset = (AlbumAsset *)&albmAsset;

    return albumAsset->DeleteAlbumAsset(albumUri);
}

bool MediaLibrary::ModifyMediaAlbumAsset(AssetType assetType, const AlbumAsset& srcAlbumAsset,
                                         const AlbumAsset &dstAlbAsset, const string &albumUri)
{
    AlbumAsset *dstAlbumAsset = (AlbumAsset *)&dstAlbAsset;

    return dstAlbumAsset->ModifyAlbumAsset(srcAlbumAsset, albumUri);
}
}  // namespace Media
}  // namespace OHOS
