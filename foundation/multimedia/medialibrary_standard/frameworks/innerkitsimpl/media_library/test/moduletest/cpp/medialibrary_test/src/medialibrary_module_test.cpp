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

#include "medialibrary_module_test.h"
#include "media_log.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
IMediaLibraryClient* mediaLibClientInstance = nullptr;

void MediaLibraryModuleTest::SetUpTestCase(void) {}
void MediaLibraryModuleTest::TearDownTestCase(void) {}

/* SetUp:Execute before each test case */
void MediaLibraryModuleTest::SetUp()
{
    mediaLibClientInstance = IMediaLibraryClient::GetMediaLibraryClientInstance();
}

void MediaLibraryModuleTest::TearDown(void)
{
    mediaLibClientInstance = nullptr;
}

/*
 * Feature: MediaLibrary
 * Function: CreateMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Create media audio asset in requested directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_CreateMediaAsset_test_001, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset mediaAsset;
    AssetType assetType = ASSET_AUDIO;
    mediaAsset.name_ = "test_001_audio.mp3";
    mediaAsset.albumName_ = "gtest/001/audio";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->CreateMediaAsset(assetType, mediaAsset);
    }
    EXPECT_NE(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: CreateMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Create media audio asset in ROOT_DIR=/data/media directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_CreateMediaAsset_test_002, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset mediaAsset;
    AssetType assetType = ASSET_AUDIO;

    mediaAsset.name_ = "gtest_002_audio.mp3";
    mediaAsset.albumName_ = "";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->CreateMediaAsset(assetType, mediaAsset);
    }
    EXPECT_NE(errCode, false);
}
/*
 * Feature: MediaLibrary
 * Function: GetAudioAssets
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Get audio assets prsent in particalar directory and subdirectories.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_GetAudioAssets_test_001, TestSize.Level1)
{
    vector<unique_ptr<AudioAsset>> audioAssetList;
    unsigned int size = 0;
    string dirPath = "gtest/001/audio";
    vector<string> selectionArgs;
    struct stat statInfo {};
    string dirPathCheck = "/data/media/" + dirPath;

    if (stat(dirPathCheck.c_str(), &statInfo) == 0) {
        if (statInfo.st_mode & S_IFDIR) {
            if (mediaLibClientInstance != nullptr) {
                audioAssetList = mediaLibClientInstance->GetAudioAssets(dirPath, selectionArgs);
            }
        }
    }
    EXPECT_NE(audioAssetList.size(), size);
}

/*
 * Feature: MediaLibrary
 * Function: GetAudioAssets
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Get audio assets from ROOT_DIR=/data/media directory and subdirectories.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_GetAudioAssets_test_002, TestSize.Level1)
{
    vector<unique_ptr<AudioAsset>> audioAssetList;
    unsigned int size = 0;
    string dirPath = ""; // ROOT_DIR
    vector<string> selectionArgs;

    if (mediaLibClientInstance != nullptr) {
        audioAssetList = mediaLibClientInstance->GetAudioAssets(dirPath, selectionArgs);
    }
    EXPECT_NE(audioAssetList.size(), size);
}

/*
 * Feature: MediaLibrary
 * Function: CreateMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Create media video asset in requested directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_CreateMediaAsset_test_003, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset mediaAsset;
    AssetType assetType = ASSET_VIDEO;

    mediaAsset.name_ = "test_002_video.mp4";
    mediaAsset.albumName_ = "gtest/002/video";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->CreateMediaAsset(assetType, mediaAsset);
    }
    EXPECT_NE(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: CreateMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Create media video sset in ROOT_DIR=/data/media directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_CreateMediaAsset_test_004, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset mediaAsset;
    AssetType assetType = ASSET_VIDEO;

    mediaAsset.name_ = "test_002_video.mp4";
    mediaAsset.albumName_ = "";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->CreateMediaAsset(assetType, mediaAsset);
    }
    EXPECT_NE(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: GetVideoAssets
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Get video assets present in particalar directory and subdirectories.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_GetVideoAssets_test_001, TestSize.Level1)
{
    vector<unique_ptr<VideoAsset>> videoAssetList;
    unsigned int size = 0;
    string dirPath = "gtest/002/video";
    vector<string> selectionArgs;
    struct stat statInfo {};
    string dirPathCheck = "/data/media/" + dirPath;

    if (stat(dirPathCheck.c_str(), &statInfo) == 0) {
        if (statInfo.st_mode & S_IFDIR) {
            if (mediaLibClientInstance != nullptr) {
                videoAssetList = mediaLibClientInstance->GetVideoAssets(dirPath, selectionArgs);
            }
        }
    }
    EXPECT_NE(videoAssetList.size(), size);
}

/*
 * Feature: MediaLibrary
 * Function: GetVideoAssets
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Get video assets from ROOT_DIR=/data/media directory and subdirectories.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_GetVideoAssets_test_002, TestSize.Level1)
{
    vector<unique_ptr<VideoAsset>> videoAssetList;
    unsigned int size = 0;
    string dirPath = "";
    vector<string> selectionArgs;

    if (mediaLibClientInstance != nullptr) {
        videoAssetList = mediaLibClientInstance->GetVideoAssets(dirPath, selectionArgs);
    }
    EXPECT_NE(videoAssetList.size(), size);
}

/*
 * Feature: MediaLibrary
 * Function: CreateMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Create media image asset in requested directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_CreateMediaAsset_test_005, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset mediaAsset;
    AssetType assetType = ASSET_IMAGE;

    mediaAsset.name_ = "test_003_image.jpg";
    mediaAsset.albumName_ = "gtest/003/image";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->CreateMediaAsset(assetType, mediaAsset);
    }
    EXPECT_NE(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: CreateMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Create media image asset in ROOT_DIR=/data/media directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_CreateMediaAsset_test_006, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset mediaAsset;
    AssetType assetType = ASSET_IMAGE;

    mediaAsset.name_ = "test_003_image.jpg";
    mediaAsset.albumName_ = "";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->CreateMediaAsset(assetType, mediaAsset);
    }
    EXPECT_NE(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: CreateMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Create existing media asset in requested directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_CreateMediaAsset_test_007, TestSize.Level1)
{
    bool errCode = true;
    MediaAsset mediaAsset;
    AssetType assetType = ASSET_IMAGE;
    // create existing file
    mediaAsset.name_ = "test_003_image.jpg";
    mediaAsset.albumName_ = "gtest/003/image";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->CreateMediaAsset(assetType, mediaAsset);
    }
    EXPECT_EQ(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: GetImageAssets
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Get image assets present in particalar directory and subdirectories.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_GetImageAssets_test_001, TestSize.Level1)
{
    vector<unique_ptr<ImageAsset>> imageAssetList;
    unsigned int size = 0;
    string dirPath = "gtest/003/image";
    vector<string> selectionArgs;
    struct stat statInfo {};
    string dirPathCheck = "/data/media/" + dirPath;

    if (stat(dirPathCheck.c_str(), &statInfo) == 0) {
        if (statInfo.st_mode & S_IFDIR) {
            if (mediaLibClientInstance != nullptr) {
                imageAssetList = mediaLibClientInstance->GetImageAssets(dirPath, selectionArgs);
            }
        }
    }
    EXPECT_NE(imageAssetList.size(), size);
}

/*
 * Feature: MediaLibrary
 * Function: GetImageAssets
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Get image assets from ROOT_DIR=/data/media directory and subdirectories.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_GetImageAssets_test_002, TestSize.Level1)
{
    vector<unique_ptr<ImageAsset>> imageAssetList;
    unsigned int size = 0;
    string dirPath = "";
    vector<string> selectionArgs;

    if (mediaLibClientInstance != nullptr) {
        imageAssetList = mediaLibClientInstance->GetImageAssets(dirPath, selectionArgs);
    }
    EXPECT_NE(imageAssetList.size(), size);
}

/*
 * Feature: MediaLibrary
 * Function: GetMediaAssets
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Get media assets present in particalar directory and subdirectories.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_GetMediaAssets_test_001, TestSize.Level1)
{
    vector<unique_ptr<MediaAsset>> mediaAssetList;
    unsigned int size = 0;
    string dirPath = "gtest/003/image";
    vector<string> selectionArgs;
    struct stat statInfo {};
    string dirPathCheck = "/data/media/" + dirPath;

    if (stat(dirPathCheck.c_str(), &statInfo) == 0) {
        if (statInfo.st_mode & S_IFDIR) {
            if (mediaLibClientInstance != nullptr) {
                mediaAssetList = mediaLibClientInstance->GetMediaAssets(dirPath, selectionArgs);
            }
        }
    }
    EXPECT_NE(mediaAssetList.size(), size);
}

/*
 * Feature: MediaLibrary
 * Function: GetMediaAssets
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Get media assets from ROOT_DIR=/data/media directory and subdirectories.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_GetMediaAssets_test_002, TestSize.Level1)
{
    vector<unique_ptr<MediaAsset>> mediaAssetList;
    unsigned int size = 0;
    string dirPath = "";
    vector<string> selectionArgs;

    if (mediaLibClientInstance != nullptr) {
        mediaAssetList = mediaLibClientInstance->GetMediaAssets(dirPath, selectionArgs);
    }
    EXPECT_NE(mediaAssetList.size(), size);
}

/*
 * Feature: MediaLibrary
 * Function: GetImageAlbumAssets
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Get image media assets present in particalar directory and subdirectories.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_GetIMageAlbumAssets_test_001, TestSize.Level1)
{
    vector<unique_ptr<AlbumAsset>> albumAssetList;
    unsigned int size = 0;
    string dirPath = "gtest/003";
    vector<string> selectionArgs;
    struct stat statInfo {};
    string dirPathCheck = "/data/media/" + dirPath;

    if (stat(dirPathCheck.c_str(), &statInfo) == 0) {
        if (statInfo.st_mode & S_IFDIR) {
            if (mediaLibClientInstance != nullptr) {
                albumAssetList = mediaLibClientInstance->GetImageAlbumAssets(dirPath, selectionArgs);
            }
        }
    }
    EXPECT_NE(albumAssetList.size(), size);
}

/*
 * Feature: MediaLibrary
 * Function: GetImageAlbumAssets
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Get image album assets from ROOT_DIR=/data/media/ directory and subdirectories.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_GetImageAlbumAssets_test_002, TestSize.Level1)
{
    vector<unique_ptr<AlbumAsset>> albumAssetList;
    unsigned int size = 0;
    string dirPath = "";
    vector<string> selectionArgs;

    if (mediaLibClientInstance != nullptr) {
        albumAssetList = mediaLibClientInstance->GetImageAlbumAssets(dirPath, selectionArgs);
    }
    EXPECT_NE(albumAssetList.size(), size);
}

/*
 * Feature: MediaLibrary
 * Function: GetVideoAlbumAssets
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Get video media assets present in particalar directory and subdirectories.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_GetVideoAlbumAssets_test_003, TestSize.Level1)
{
    vector<unique_ptr<AlbumAsset>> albumAssetList;
    unsigned int size = 0;
    string dirPath = "gtest/002";
    vector<string> selectionArgs;
    struct stat statInfo {};
    string dirPathCheck = "/data/media/" + dirPath;

    if (stat(dirPathCheck.c_str(), &statInfo) == 0) {
        if (statInfo.st_mode & S_IFDIR) {
            if (mediaLibClientInstance != nullptr) {
                albumAssetList = mediaLibClientInstance->GetVideoAlbumAssets(dirPath, selectionArgs);
            }
        }
    }
    EXPECT_NE(albumAssetList.size(), size);
}

/*
 * Feature: MediaLibrary
 * Function: GetVideoAlbumAssets
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Get video album assets from ROOT_DIR=/data/media directory and subdirectories.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_GetVideoAlbumAssets_test_004, TestSize.Level1)
{
    vector<unique_ptr<AlbumAsset>> albumAssetList;
    unsigned int size = 0;
    string dirPath = "";
    vector<string> selectionArgs;

    if (mediaLibClientInstance != nullptr) {
        albumAssetList = mediaLibClientInstance->GetVideoAlbumAssets(dirPath, selectionArgs);
    }
    EXPECT_NE(albumAssetList.size(), size);
}

/*
 * Feature: MediaLibrary
 * Function: ModifyMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Modify media audio asset in requested directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_ModifyMediaAsset_test_001, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset srcMediaAsset;
    MediaAsset dstMediaAsset;
    AssetType assetType = ASSET_AUDIO;

    srcMediaAsset.name_ = "test_001_audio.mp3";
    srcMediaAsset.uri_ = "/data/media/gtest/001/audio/test_001_audio.mp3";
    dstMediaAsset.name_ = "test_001_audio_modify.mp3";
    dstMediaAsset.uri_ = "/data/media/gtest/001/audio/test_001_audio.mp3";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->ModifyMediaAsset(assetType, srcMediaAsset, dstMediaAsset);
    }
    EXPECT_NE(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: ModifyMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Modify media audio mp3 file to wav in requested directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_ModifyMediaAsset_test_002, TestSize.Level1)
{
    bool errCode = true;
    MediaAsset srcMediaAsset;
    MediaAsset dstMediaAsset;
    AssetType assetType = ASSET_AUDIO;

    srcMediaAsset.name_ = "test_002_audio.mp3";
    srcMediaAsset.uri_ = "/data/media/gtest/001/audio/test_002_audio.mp3";
    dstMediaAsset.name_ = "test_002_audio_modify.wav";
    dstMediaAsset.uri_ = "/data/media/gtest/001/audio/test_002_audio.mp3";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->ModifyMediaAsset(assetType, srcMediaAsset, dstMediaAsset);
    }
    EXPECT_EQ(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: ModifyMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Modify media audio asset  in requested non existing directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_ModifyMediaAsset_test_003, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset srcMediaAsset;
    MediaAsset dstMediaAsset;
    AssetType assetType = ASSET_AUDIO;

    srcMediaAsset.name_ = "test_002_audio.mp3";
    srcMediaAsset.uri_ = "/data/media/gtest/001/audio_nofile/test_002_audio.mp3";
    dstMediaAsset.uri_ = "/data/media/gtest/001/audio_nofile/test_002_audio.mp3";
    dstMediaAsset.name_ = "test_002_audio_modify.mp3";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->ModifyMediaAsset(assetType, srcMediaAsset, dstMediaAsset);
    }
    EXPECT_EQ(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: CopyMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: copy media asset to requested new directory, which gets created.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_CopyMediaAsset_test_001, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset srcMediaAsset;
    MediaAsset dstMediaAsset;
    AssetType assetType = ASSET_AUDIO;
    // copy to requested ablumName
    srcMediaAsset.name_ = "test_001_audio_modify.mp3";
    srcMediaAsset.uri_ = "/data/media/gtest/001/audio/test_001_audio_modify.mp3";
    dstMediaAsset.albumName_ = "gtest/001/copyaudio";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->CopyMediaAsset(assetType, srcMediaAsset, dstMediaAsset);
    }
    EXPECT_NE(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: CopyMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: copy media asset to requested non existing directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_CopyMediaAsset_test_002, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset srcMediaAsset;
    MediaAsset dstMediaAsset;
    AssetType assetType = ASSET_IMAGE;
    // create and Copy to requested ablumName
    srcMediaAsset.name_ = "test_003_image.jpg";
    srcMediaAsset.uri_ = "/data/media/gtest/003/image/test_003_image.jpg";
    dstMediaAsset.albumName_ = "copyjpg/001/image";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->CopyMediaAsset(assetType, srcMediaAsset, dstMediaAsset);
    }
    EXPECT_NE(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: CopyMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: copy media asset to ROOT_DIR=/data/media directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_CopyMediaAsset_test_003, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset srcMediaAsset;
    MediaAsset dstMediaAsset;
    AssetType assetType = ASSET_AUDIO;
    // copy to ROOT_DIR
    srcMediaAsset.name_ = "test_001_audio_modify.mp3";
    srcMediaAsset.uri_ = "/data/media/gtest/001/copyaudio/test_001_audio_modify.mp3";
    dstMediaAsset.albumName_ = "";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->CopyMediaAsset(assetType, srcMediaAsset, dstMediaAsset);
    }
    EXPECT_EQ(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: CopyMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: copy media asset in requested directory without source file name .
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_CopyMediaAsset_test_004, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset srcMediaAsset;
    MediaAsset dstMediaAsset;
    AssetType assetType = ASSET_AUDIO;

    srcMediaAsset.name_ = "";
    srcMediaAsset.uri_ = "/data/media/gtest/001/audio/test_fail.mp3";
    dstMediaAsset.albumName_ = "";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->CopyMediaAsset(assetType, srcMediaAsset, dstMediaAsset);
    }
    EXPECT_EQ(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: DeleteMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Delete media audio asset from requested directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_DeleteMediaAsset_test_001, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset mediaAsset;
    AssetType assetType = ASSET_AUDIO;

    mediaAsset.name_ = "test_001_audio_modify.mp3";
    mediaAsset.uri_ = "/data/media/gtest/001/copyaudio/test_001_audio_modify.mp3";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->DeleteMediaAsset(assetType, mediaAsset);
    }
    EXPECT_NE(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: DeleteMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Delete media video asset in requested directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_DeleteMediaAsset_test_002, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset mediaAsset;
    AssetType assetType = ASSET_VIDEO;

    mediaAsset.name_ = "test_002_video.mp4";
    mediaAsset.uri_ = "/data/media/gtest/002/video/test_002_video.mp4";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->DeleteMediaAsset(assetType, mediaAsset);
    }
    EXPECT_NE(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: DeleteMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Create media image asset in requested directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_DeleteMediaAsset_test_003, TestSize.Level1)
{
    bool errCode = false;
    MediaAsset mediaAsset;
    AssetType assetType = ASSET_IMAGE;

    mediaAsset.name_ = "test_003_image.jpg";
    mediaAsset.uri_ = "/data/media/gtest/003/image/test_003_image.jpg";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->DeleteMediaAsset(assetType, mediaAsset);
    }
    EXPECT_NE(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: DeleteMediaAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Delete media asset from requested empty directory path
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_DeleteMediaAsset_test_004, TestSize.Level1)
{
    bool errCode = true;
    MediaAsset mediaAsset;
    AssetType assetType = ASSET_AUDIO;

    mediaAsset.name_ = "";
    mediaAsset.uri_ = "";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->DeleteMediaAsset(assetType, mediaAsset);
    }
    EXPECT_EQ(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: CreateMediaAlbumAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Create album asset in requested directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_CreateMediaAlbumAsset_test_001, TestSize.Level1)
{
    bool errCode = false;
    AlbumAsset albumAsset;
    AssetType assetType = ASSET_GENERIC_ALBUM;

    albumAsset.albumName_ = "gtest/crtalbum001";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->CreateMediaAlbumAsset(assetType, albumAsset);
    }
    EXPECT_NE(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: ModifyMediaAlbumAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Modify image album asset from requested directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_ModifyMediaAlbumAsset_test_001, TestSize.Level1)
{
    bool errCode = false;
    AlbumAsset srcAlbumAsset;
    AlbumAsset dstAlbumAsset;
    string albumUri;
    AssetType assetType = ASSET_IMAGEALBUM;

    srcAlbumAsset.albumName_ = "copyjpg/001/image";
    dstAlbumAsset.albumName_ = "modify_image001";
    albumUri = "/data/media/copyjpg/001/image";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->ModifyMediaAlbumAsset(assetType, srcAlbumAsset, dstAlbumAsset, albumUri);
    }
    EXPECT_NE(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: ModifyMediaAlbumAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Modify image album asset in requested "non existing" directory.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_ModifyMediaAlbumAsset_test_002, TestSize.Level1)
{
    bool errCode = true;
    AlbumAsset srcAlbumAsset;
    AlbumAsset dstAlbumAsset;
    string albumUri;
    AssetType assetType = ASSET_IMAGEALBUM;

    srcAlbumAsset.albumName_ = "test/album001";
    dstAlbumAsset.albumName_ = "modify_album002";
    albumUri = "/data/media/test2/album001";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->ModifyMediaAlbumAsset(assetType, srcAlbumAsset, dstAlbumAsset, albumUri);
    }
    EXPECT_EQ(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: DeleteMediaAlbumAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Delete media album from requested directory path.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_DeleteMediaAlbumAsset_test_001, TestSize.Level1)
{
    bool errCode = false;
    AlbumAsset albumAsset;
    string albumUri;
    struct stat statInfo {};
    AssetType assetType = ASSET_IMAGEALBUM;

    albumAsset.albumName_ = "copyjpg/001/modify_image001";
    albumUri = "/data/media/copyjpg/001/modify_image001";
    if (stat(albumUri.c_str(), &statInfo) == 0) {
        if (statInfo.st_mode & S_IFDIR) {
            if (mediaLibClientInstance != nullptr) {
                errCode = mediaLibClientInstance->DeleteMediaAlbumAsset(assetType, albumAsset, albumUri);
            }
        }
    }
    EXPECT_NE(errCode, false);
}

/*
 * Feature: MediaLibrary
 * Function: DeleteMediaAlbumAsset
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Delete media album from requested non existing path.
 */
HWTEST_F(MediaLibraryModuleTest, medialibrary_DeleteMediaAlbumAsset_test_002, TestSize.Level1)
{
    bool errCode = true;
    AlbumAsset albumAsset;
    string albumUri;
    AssetType assetType = ASSET_IMAGEALBUM;

    albumAsset.albumName_ = "gtest/modify_album001";
    albumUri = "/data/media/test/modify_album002";
    if (mediaLibClientInstance != nullptr) {
        errCode = mediaLibClientInstance->DeleteMediaAlbumAsset(assetType, albumAsset, albumUri);
    }
    EXPECT_EQ(errCode, false);
}
} // namespace Media
} // namespace OHOS
