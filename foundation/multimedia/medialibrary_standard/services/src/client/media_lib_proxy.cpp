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

#include "media_lib_proxy.h"
#include "media_file_utils.h"
#include "media_log.h"

using namespace std;
namespace OHOS {
namespace Media {
MediaLibProxy::MediaLibProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IMediaLibService>(impl) { }

IMediaLibraryClient *IMediaLibraryClient::GetMediaLibraryClientInstance()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        MEDIA_ERR_LOG("samgr object is NULL.");
        return nullptr;
    }
    sptr<IRemoteObject> object = samgr->GetSystemAbility(MEDIA_LIBRARY_SERVICE_ID);
    if (object == nullptr) {
        MEDIA_ERR_LOG("Media Service object is NULL.");
        return nullptr;
    }
    static MediaLibProxy msProxy(object);

    return &msProxy;
}

bool CopyUriToDstMediaAsset(const MediaAsset &assetData)
{
    bool errCode = false;
    MediaAsset *dstAsset = (MediaAsset *)&assetData;

    // If albumname is empty, napi returns false
    if (!(dstAsset->albumName_.empty())) {
        dstAsset->uri_ = ROOT_MEDIA_DIR + SLASH_CHAR + dstAsset->albumName_;
        errCode = true;
    }

    return errCode;
}

void ModifyDstMediaAssetUri(const MediaAsset &assetData)
{
    MediaAsset *dstAsset = (MediaAsset *)&assetData;

    size_t slashIndex = dstAsset->uri_.rfind("/");
    if (slashIndex != string::npos) {
        string dirPath = dstAsset->uri_.substr(0, slashIndex);
        dstAsset->uri_ = dirPath + "/" + dstAsset->name_;
    }
}

int32_t WriteCommonData(const MediaAsset &assetData, const MessageParcel &dataMsg)
{
    MessageParcel *data = (MessageParcel *)&dataMsg;
    MediaAsset *asset = (MediaAsset *)&assetData;

    if (data->WriteInt32(asset->id_) &&
        data->WriteUint64(asset->size_) &&
        data->WriteInt32(asset->albumId_) &&
        data->WriteString(asset->albumName_) &&
        data->WriteString(asset->uri_) &&
        data->WriteInt32(asset->mediaType_) &&
        data->WriteString(asset->name_) &&
        data->WriteUint64(asset->dateAdded_) &&
        data->WriteUint64(asset->dateModified_)) {
        return SUCCESS;
    }

    return COMMON_DATA_WRITE_FAIL;
}

void ReadCommonData(const MediaAsset &assetData, const MessageParcel &replyMsg)
{
    MessageParcel *reply = (MessageParcel *)&replyMsg;
    MediaAsset *asset = (MediaAsset *)&assetData;

    asset->id_ = reply->ReadInt32();
    asset->size_ = reply->ReadUint64();
    asset->albumId_ = reply->ReadInt32();
    asset->albumName_ = reply->ReadString();
    asset->uri_ = reply->ReadString();
    asset->mediaType_ = (MediaType)reply->ReadInt32();
    asset->name_ = reply->ReadString();
    asset->dateAdded_ = reply->ReadUint64();
    asset->dateModified_ = reply->ReadUint64();
}

int32_t WriteAlbumdata(const AlbumAsset& assetData, const MessageParcel& dataMsg)
{
    MessageParcel *data = (MessageParcel *)&dataMsg;
    AlbumAsset *albumAsset = (AlbumAsset *)&assetData;

    if (data->WriteInt32(albumAsset->albumId_) &&
        data->WriteString(albumAsset->albumName_)) {
        return SUCCESS;
    }

    return ALBUM_ASSET_WRITE_FAIL;
}

void ReadMediaAlbumdata(const AlbumAsset& assetData, const MessageParcel& replyMsg)
{
    MessageParcel *reply = (MessageParcel *)&replyMsg;
    AlbumAsset *albumAsset = (AlbumAsset *)&assetData;

    albumAsset->albumId_ = reply->ReadInt32();
    albumAsset->albumName_ = reply->ReadString();
}

int32_t ReadMediaList(vector<unique_ptr<MediaAsset>> &mediaAssetList, MessageParcel &reply)
{
    int32_t errCode = SUCCESS;
    int32_t count = reply.ReadInt32();

    for (int i = 0; i < count; i++) {
        unique_ptr<MediaAsset> mediaAsset = make_unique<MediaAsset>();
        if (mediaAsset) {
            ReadCommonData(*mediaAsset, reply);
            mediaAssetList.push_back(move(mediaAsset));
        } else {
            MEDIA_ERR_LOG("MediaLibProxy::MediaAsset allocation failed");
            errCode = MEDIA_ASSET_READ_FAIL;
            break;
        }
    }

    return errCode;
}

int32_t ReadImageList(vector<unique_ptr<ImageAsset>> &imageAssetList, MessageParcel &reply)
{
    int32_t errCode = SUCCESS;
    int32_t count = reply.ReadInt32();

    for (int i = 0; i < count; i++) {
        unique_ptr<ImageAsset> imageAsset = make_unique<ImageAsset>();
        if (imageAsset) {
            ReadCommonData(*imageAsset, reply);
            imageAsset->width_ = reply.ReadInt32();
            imageAsset->height_ = reply.ReadInt32();
            imageAsset->mimeType_ = reply.ReadString();

            imageAssetList.push_back(move(imageAsset));
        } else {
            MEDIA_ERR_LOG("MediaLibProxy::ImageAsset allocation failed");
            errCode = IMAGE_ASSET_READ_FAIL;
            break;
        }
    }

    return errCode;
}

int32_t ReadVideoList(vector<unique_ptr<VideoAsset>> &videoAssetList, MessageParcel &reply)
{
    int32_t errCode = SUCCESS;
    int32_t count = reply.ReadInt32();

    for (int i = 0; i < count; i++) {
        unique_ptr<VideoAsset> videoAsset = make_unique<VideoAsset>();
        if (videoAsset) {
            ReadCommonData(*videoAsset, reply);
            videoAsset->width_ = reply.ReadInt32();
            videoAsset->height_ = reply.ReadInt32();
            videoAsset->duration_ = reply.ReadInt32();
            videoAsset->mimeType_ = reply.ReadString();

            videoAssetList.push_back(move(videoAsset));
        } else {
            MEDIA_ERR_LOG("MediaLibProxy::VideoAsset allocation failed");
            errCode = VIDEO_ASSET_READ_FAIL;
            break;
        }
    }

    return errCode;
}

int32_t ReadAudioList(vector<unique_ptr<AudioAsset>> &audioAssetList, MessageParcel &reply)
{
    int32_t errCode = SUCCESS;
    int32_t count = reply.ReadInt32();

    for (int i = 0; i < count; i++) {
        unique_ptr<AudioAsset> audioAsset = make_unique<AudioAsset>();
        if (audioAsset) {
            ReadCommonData(*audioAsset, reply);

            audioAsset->title_ = reply.ReadString();
            audioAsset->artist_ = reply.ReadString();
            audioAsset->duration_ = reply.ReadInt32();
            audioAsset->mimeType_ = reply.ReadString();

            audioAssetList.push_back(move(audioAsset));
        } else {
            MEDIA_ERR_LOG("MediaLibProxy::AudioAsset allocation failed");
            errCode = AUDIO_ASSET_READ_FAIL;
            break;
        }
    }

    return errCode;
}

int32_t ReadImageAlbumList(vector<unique_ptr<AlbumAsset>> &imageAlbumList, MessageParcel &reply)
{
    int32_t errCode = SUCCESS;
    int32_t count = reply.ReadInt32();

    for (int i = 0; i < count; i++) {
        unique_ptr<AlbumAsset> imageAlbum = make_unique<AlbumAsset>();
        if (imageAlbum) {
            imageAlbum->albumId_ = reply.ReadInt32();
            imageAlbum->albumName_ = reply.ReadString();

            if (ReadImageList(imageAlbum->imageAssetList_, reply) == IMAGE_ASSET_READ_FAIL) {
                MEDIA_ERR_LOG("MediaLibProxy::AlbumAsset read image list failed");
                errCode = IMAGEALBUM_ASSET_READ_FAIL;
                break;
            }
            imageAlbumList.push_back(move(imageAlbum));
        } else {
            MEDIA_ERR_LOG("MediaLibProxy::AlbumAsset allocation failed");
            errCode = IMAGEALBUM_ASSET_READ_FAIL;
            break;
        }
    }

    return errCode;
}

int32_t ReadVideoAlbumList(vector<unique_ptr<AlbumAsset>> &videoAlbumList, MessageParcel &reply)
{
    int32_t errCode = SUCCESS;
    int32_t count = reply.ReadInt32();

    for (int i = 0; i < count; i++) {
        unique_ptr<AlbumAsset> videoAlbum = make_unique<AlbumAsset>();
        if (videoAlbum) {
            videoAlbum->albumId_ = reply.ReadInt32();
            videoAlbum->albumName_ = reply.ReadString();

            if (ReadVideoList(videoAlbum->videoAssetList_, reply) == VIDEO_ASSET_READ_FAIL) {
                MEDIA_ERR_LOG("MediaLibProxy::AlbumAsset read video list failed");
                errCode = VIDEOALBUM_ASSET_READ_FAIL;
                break;
            }
            videoAlbumList.push_back(move(videoAlbum));
        } else {
            MEDIA_ERR_LOG("MediaLibProxy::AlbumAsset allocation failed");
            errCode = VIDEOALBUM_ASSET_READ_FAIL;
            break;
        }
    }

    return errCode;
}

vector<unique_ptr<ImageAsset>> MediaLibProxy::GetImageAssets(string selection,
    vector<string> selArgs)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    vector<unique_ptr<ImageAsset>> imageAssetList;

    if ((data.WriteString(selection)) && (data.WriteStringVector(selArgs))) {
        int error = Remote()->SendRequest(MEDIA_GET_IMAGE_ASSETS, data, reply, option);
        if (error != ERR_NONE) {
            MEDIA_ERR_LOG("GetImageAssets failed, error: %{public}d", error);
            return imageAssetList;
        }
        (void)ReadImageList(imageAssetList, reply);
    } else {
        MEDIA_ERR_LOG("data.WriteStringFailed");
    }

    return imageAssetList;
}

vector<unique_ptr<AudioAsset>> MediaLibProxy::GetAudioAssets(string selection,
    vector<string> selArgs)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    vector<unique_ptr<AudioAsset>> audioAssetList;

    if ((data.WriteString(selection)) && (data.WriteStringVector(selArgs))) {
        int error = Remote()->SendRequest(MEDIA_GET_AUDIO_ASSETS, data, reply, option);
        if (error != ERR_NONE) {
            MEDIA_ERR_LOG("GetAudioAssets failed, error: %{public}d", error);
            return audioAssetList;
        }
        (void)ReadAudioList(audioAssetList, reply);
    } else {
        MEDIA_ERR_LOG("data.WriteStringFailed");
    }

    return audioAssetList;
}

vector<unique_ptr<VideoAsset>> MediaLibProxy::GetVideoAssets(string selection,
    vector<string> selArgs)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    vector<unique_ptr<VideoAsset>> videoAssetList;

    if ((data.WriteString(selection)) && (data.WriteStringVector(selArgs))) {
        int error = Remote()->SendRequest(MEDIA_GET_VIDEO_ASSETS, data, reply, option);
        if (error != ERR_NONE) {
            MEDIA_ERR_LOG("GetVideoAssets failed, error: %{public}d", error);
            return videoAssetList;
        }
        (void)ReadVideoList(videoAssetList, reply);
    } else {
        MEDIA_ERR_LOG("data.WriteStringFailed");
    }

    return videoAssetList;
}

vector<unique_ptr<MediaAsset>> MediaLibProxy::GetMediaAssets(string selection,
    vector<string> selArgs)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    vector<unique_ptr<MediaAsset>> mediaAssetList;

    if ((data.WriteString(selection)) && (data.WriteStringVector(selArgs))) {
        int error = Remote()->SendRequest(MEDIA_GET_MEDIA_ASSETS, data, reply, option);
        if (error != ERR_NONE) {
            MEDIA_ERR_LOG("GetMediaAssets failed, error: %{public}d", error);
            return mediaAssetList;
        }
        (void)ReadMediaList(mediaAssetList, reply);
    } else {
        MEDIA_ERR_LOG("data.WriteStringFailed");
    }

    return mediaAssetList;
}

vector<unique_ptr<AlbumAsset>> MediaLibProxy::GetImageAlbumAssets(string selection,
    vector<string> selArgs)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    vector<unique_ptr<AlbumAsset>> albumAssetList;

    if ((data.WriteString(selection)) && (data.WriteStringVector(selArgs))) {
        int error = Remote()->SendRequest(MEDIA_GET_IMAGEALBUM_ASSETS, data, reply, option);
        if (error != ERR_NONE) {
            MEDIA_ERR_LOG("GetImageAlbumAssets failed, error: %{public}d", error);
            return albumAssetList;
        }
        (void)ReadImageAlbumList(albumAssetList, reply);
    } else {
        MEDIA_ERR_LOG("data.WriteStringFailed");
    }

    return albumAssetList;
}

vector<unique_ptr<AlbumAsset>> MediaLibProxy::GetVideoAlbumAssets(string selection,
    vector<string> selArgs)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    vector<unique_ptr<AlbumAsset>> albumAssetList;

    if ((data.WriteString(selection)) && (data.WriteStringVector(selArgs))) {
        int error = Remote()->SendRequest(MEDIA_GET_VIDEOALBUM_ASSETS, data, reply, option);
        if (error != ERR_NONE) {
            MEDIA_ERR_LOG("GetVideoAlbumAssets failed, error: %{public}d", error);
            return albumAssetList;
        }
        (void)ReadVideoAlbumList(albumAssetList, reply);
    } else {
        MEDIA_ERR_LOG("data.WriteStringFailed");
    }

    return albumAssetList;
}

bool MediaLibProxy::CreateMediaAsset(AssetType assetType, const MediaAsset& mediaAsset)
{
    bool errRet = true;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if ((data.WriteInt32(assetType) && (WriteCommonData(mediaAsset, data) == SUCCESS))) {
        int error = Remote()->SendRequest(MEDIA_CREATE_MEDIA_ASSET, data, reply, option);
        if (error != ERR_NONE) {
            MEDIA_ERR_LOG("CreateMedia failed, error: %{public}d", error);
            errRet = false;
        }
    } else {
        errRet = false;
        MEDIA_ERR_LOG("CreateMediaAsset wrtie data failed !!");
    }

    return errRet;
}

bool MediaLibProxy::DeleteMediaAsset(AssetType assetType, const MediaAsset& mediaAsset)
{
    bool errRet = true;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if ((data.WriteInt32(assetType) && (WriteCommonData(mediaAsset, data) == SUCCESS))) {
        int error = Remote()->SendRequest(MEDIA_DELETE_MEDIA_ASSET, data, reply, option);
        if (error != ERR_NONE) {
            MEDIA_ERR_LOG("DeleteMediaAsset SendRequest failed, error: %{public}d", error);
            errRet = false;
        }
    } else {
        errRet = false;
    }

    return errRet;
}

bool MediaLibProxy::ModifyMediaAsset(AssetType assetType, const MediaAsset &srcMediaAsset,
                                     const MediaAsset &dstMediaAsset)
{
    bool errRet = true;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    ModifyDstMediaAssetUri(dstMediaAsset);
    if ((data.WriteInt32(assetType)) && (WriteCommonData(srcMediaAsset, data) == SUCCESS) &&
        (WriteCommonData(dstMediaAsset, data) == SUCCESS)) {
        int error = Remote()->SendRequest(MEDIA_MODIFY_MEDIA_ASSET, data, reply, option);
        if (error != ERR_NONE) {
            MEDIA_ERR_LOG("ModifyMediaAsset SendRequest failed, error: %{public}d", error);
            errRet = false;
        }
    } else {
        errRet = false;
        MEDIA_ERR_LOG("ModifyMediaAsset write data failed !!");
    }

    return errRet;
}

bool MediaLibProxy::CopyMediaAsset(AssetType assetType, const MediaAsset &srcMediaAsset,
                                   const MediaAsset &dstMediaAsset)
{
    bool errRet = true;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if ((CopyUriToDstMediaAsset(dstMediaAsset) == true) && (data.WriteInt32(assetType)) &&
        (WriteCommonData(srcMediaAsset, data) == SUCCESS) &&
        (WriteCommonData(dstMediaAsset, data) == SUCCESS)) {
        int error = Remote()->SendRequest(MEDIA_COPY_MEDIA_ASSET, data, reply, option);
        if (error != ERR_NONE) {
            MEDIA_ERR_LOG("CopyMediaAsset SendRequest failed, error: %{public}d", error);
            errRet = false;
        }
    } else {
        errRet = false;
        MEDIA_ERR_LOG("CopyMediaAsset: write data failed !!");
    }

    return errRet;
}

bool MediaLibProxy::CreateMediaAlbumAsset(AssetType assetType, const AlbumAsset& albumAsset)
{
    bool errRet = true;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if ((data.WriteInt32(assetType)) && (WriteAlbumdata(albumAsset, data) == SUCCESS)) {
        int error = Remote()->SendRequest(MEDIA_CREATE_MEDIA_ALBUM_ASSET, data, reply, option);
        if (error != ERR_NONE) {
            MEDIA_ERR_LOG("CreateMediaAlbumAsset SendRequest failed, error: %{public}d", error);
            errRet = false;
        }
    } else {
        MEDIA_ERR_LOG("CreateMediaAlbumAsset WriteAlbumdata failed");
        errRet = false;
    }

    return errRet;
}

bool MediaLibProxy::DeleteMediaAlbumAsset(AssetType assetType,  const AlbumAsset& albumAsset,
                                          const string &albumUri)
{
    bool errRet = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if ((assetType == ASSET_IMAGEALBUM) && (data.WriteInt32(assetType)) &&
        (WriteAlbumdata(albumAsset, data) == SUCCESS) && data.WriteString(albumUri)) {
            MEDIA_ERR_LOG("DeleteMediaAlbumAsset-WriteAlbumdata done");
            errRet = true;
    } else if ((assetType == ASSET_VIDEOALBUM) && (data.WriteInt32(assetType)) &&
               (WriteAlbumdata(albumAsset, data) == SUCCESS) && data.WriteString(albumUri)) {
                MEDIA_ERR_LOG("DeleteMediaAlbumAsset-WriteAlbumdata done");
                errRet = true;
    }
    if (errRet == true) {
        int error = Remote()->SendRequest(MEDIA_DELETE_MEDIA_ALBUM_ASSET, data, reply, option);
        if (error != ERR_NONE) {
            MEDIA_ERR_LOG("DeleteMediaAlbumAsset SendRequest failed, error: %{public}d", error);
            errRet = false;
        }
    } else {
        MEDIA_ERR_LOG("DeleteMediaAlbumAsset-WriteAlbumdata Failed");
    }

    return errRet;
}

bool MediaLibProxy::ModifyMediaAlbumAsset(AssetType assetType, const AlbumAsset& srcAlbumAsset,
                                          const AlbumAsset &dstAlbumAsset, const string &albumUri)
{
    bool errRet = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    AlbumAsset *dstAsset = (AlbumAsset *)&dstAlbumAsset;

    if (data.WriteInt32(assetType)) {
        if ((assetType == ASSET_IMAGEALBUM) &&
            (WriteAlbumdata(srcAlbumAsset, data) == SUCCESS) &&
            (WriteAlbumdata(dstAlbumAsset, data) == SUCCESS) &&
            data.WriteString(albumUri)) {
            errRet = true;
        } else if ((assetType == ASSET_VIDEOALBUM) &&
                   (WriteAlbumdata(srcAlbumAsset, data) == SUCCESS) &&
                   (WriteAlbumdata(dstAlbumAsset, data) == SUCCESS) &&
                   data.WriteString(albumUri)) {
            errRet = true;
        }

        if (errRet == true) {
            int error = Remote()->SendRequest(MEDIA_MODIFY_MEDIA_ALBUM_ASSET, data, reply, option);
            if (error != ERR_NONE) {
                MEDIA_ERR_LOG("ModifyMediaAlbumAsset SendRequest failed, error: %{public}d", error);
                return false;
            }
            dstAsset->albumName_ = ROOT_MEDIA_DIR + "/" + dstAsset->albumName_;
        } else {
            MEDIA_ERR_LOG("ModifyMediaAlbumAsset: WriteAlbumdata failed");
        }
    } else {
        MEDIA_ERR_LOG("ModifyMediaAlbumAsset assetype write failed");
    }

    return errRet;
}
} // namespace Media
} // namespace OHOS
