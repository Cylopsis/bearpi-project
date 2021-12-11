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

#include "media_lib_service_stub.h"

#include "media_lib_service.h"
#include "media_lib_service_const.h"
#include "media_log.h"


using namespace std;

namespace OHOS {
namespace Media {
int32_t MediaLibServiceStub::WriteCommonData(const MediaAsset &assetData, const MessageParcel &replyMsg)
{
    MessageParcel *reply = (MessageParcel *)&replyMsg;
    MediaAsset *asset = (MediaAsset *)&assetData;

    if (reply->WriteInt32(asset->id_) &&
        reply->WriteUint64(asset->size_) &&
        reply->WriteInt32(asset->albumId_) &&
        reply->WriteString(asset->albumName_) &&
        reply->WriteString(asset->uri_) &&
        reply->WriteInt32(asset->mediaType_) &&
        reply->WriteString(asset->name_) &&
        reply->WriteUint64(asset->dateAdded_) &&
        reply->WriteUint64(asset->dateModified_)) {
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

int32_t WriteImageAlbumdata(const AlbumAsset& albumAssetData, const MessageParcel& replyMsg)
{
    MessageParcel *reply = (MessageParcel *)&replyMsg;
    AlbumAsset *albumAsset = (AlbumAsset *)&albumAssetData;

    if (reply->WriteInt32(albumAsset->albumId_) &&
        reply->WriteString(albumAsset->albumName_)) {
        return SUCCESS;
    }

    return IMAGEALBUM_ASSET_WRITE_FAIL;
}

int32_t WriteVideoAlbumdata(const AlbumAsset& albumAssetData, const MessageParcel& replyMsg)
{
    MessageParcel *reply = (MessageParcel *)&replyMsg;
    AlbumAsset *albumAsset = (AlbumAsset *)&albumAssetData;

    if (reply->WriteInt32(albumAsset->albumId_) &&
        reply->WriteString(albumAsset->albumName_)) {
        return SUCCESS;
    }

    return VIDEOALBUM_ASSET_WRITE_FAIL;
}

void ReadMediaAlbumdata(const AlbumAsset& albumAssetData, const MessageParcel& replyMsg)
{
    MessageParcel *reply = (MessageParcel *)&replyMsg;
    AlbumAsset *albumAsset = (AlbumAsset *)&albumAssetData;

    albumAsset->albumId_ = reply->ReadInt32();
    albumAsset->albumName_ = reply->ReadString();
}

int32_t MediaLibServiceStub::WriteImageList(vector<unique_ptr<ImageAsset>> &imageAssetList,
    MessageParcel &reply)
{
    int32_t errCode = IMAGE_ASSET_WRITE_FAIL;
    int32_t size;

    size = (int32_t)imageAssetList.size();
    if (!reply.WriteInt32(size)) {
        return errCode;
    }

    if (size == 0) {
        return SUCCESS;
    }

    for (int32_t i = 0; i < size; i++) {
        ImageAsset *imageAsset = imageAssetList[i].get();
        if ((imageAsset != nullptr) && (WriteCommonData(*imageAsset, reply) == SUCCESS)) {
            if (reply.WriteInt32(imageAsset->width_) &&
                reply.WriteInt32(imageAsset->height_) &&
                reply.WriteString(imageAsset->mimeType_)) {
                errCode = SUCCESS;
            }
        }
    }

    return errCode;
}

int32_t MediaLibServiceStub::WriteAudioList(vector<unique_ptr<AudioAsset>> &audioAssetList,
    MessageParcel &reply)
{
    int32_t errCode = AUDIO_ASSET_WRITE_FAIL;
    int32_t size;

    size = (int32_t)audioAssetList.size();
    if (!reply.WriteInt32(size)) {
        return errCode;
    }

    for (int32_t i = 0; i < size; i++) {
        AudioAsset *audioAsset = audioAssetList[i].get();
        if ((audioAsset != nullptr) && (WriteCommonData(*audioAsset, reply) == SUCCESS)) {
            if (reply.WriteString(audioAsset->title_) &&
                reply.WriteString(audioAsset->artist_) &&
                reply.WriteInt32(audioAsset->duration_) &&
                reply.WriteString(audioAsset->mimeType_)) {
                errCode = SUCCESS;
            }
        }
    }

    return errCode;
}

int32_t MediaLibServiceStub::WriteVideoList(vector<unique_ptr<VideoAsset>> &videoAssetList,
    MessageParcel &reply)
{
    int32_t errCode = VIDEO_ASSET_WRITE_FAIL;
    int32_t size;

    size = (int32_t)videoAssetList.size();
    if (!reply.WriteInt32(size)) {
        return errCode;
    }

    for (int32_t i = 0; i < size; i++) {
        VideoAsset *videoAsset = videoAssetList[i].get();
        if ((videoAsset != nullptr) && (WriteCommonData(*videoAsset, reply) == SUCCESS)) {
            if (reply.WriteInt32(videoAsset->width_) &&
                reply.WriteInt32(videoAsset->height_) &&
                reply.WriteInt32(videoAsset->duration_) &&
                reply.WriteString(videoAsset->mimeType_)) {
                errCode = SUCCESS;
            }
        }
    }

    return errCode;
}

int32_t MediaLibServiceStub::WriteMediaList(vector<unique_ptr<MediaAsset>> &mediaAssetList,
    MessageParcel &reply)
{
    int32_t errCode = MEDIA_ASSET_WRITE_FAIL;
    int32_t size;

    size = (int32_t)mediaAssetList.size();
    if (reply.WriteInt32(size)) {
        for (int32_t i = 0; i < size; i++) {
            MediaAsset *mediaAsset = mediaAssetList[i].get();
            if (mediaAsset != nullptr) {
                errCode = WriteCommonData(*mediaAsset, reply);
            }
        }
    }
    return errCode;
}

int32_t MediaLibServiceStub::WriteImageAlbum(vector<unique_ptr<AlbumAsset>> &albumAssetList,
    MessageParcel &reply)
{
    int32_t errCode = IMAGEALBUM_ASSET_WRITE_FAIL;
    int32_t size;

    size = (int32_t)albumAssetList.size();
    if (reply.WriteInt32(size)) {
        for (int32_t i = 0; i < size; i++) {
            AlbumAsset *albumAsset = albumAssetList[i].get();
            if (reply.WriteInt32(albumAsset->albumId_) &&
                reply.WriteString(albumAsset->albumName_)) {
                errCode = WriteImageList(albumAsset->imageAssetList_, reply);
            }
        }
    }
    return errCode;
}

int32_t MediaLibServiceStub::WriteVideoAlbum(vector<unique_ptr<AlbumAsset>> &albumAssetList,
    MessageParcel &reply)
{
    int32_t errCode = VIDEOALBUM_ASSET_WRITE_FAIL;
    int32_t size;

    size = (int32_t)albumAssetList.size();
    if (reply.WriteInt32(size)) {
        for (int32_t i = 0; i < size; i++) {
            AlbumAsset *albumAsset = albumAssetList[i].get();
            if (reply.WriteInt32(albumAsset->albumId_) &&
                reply.WriteString(albumAsset->albumName_)) {
                errCode = WriteVideoList(albumAsset->videoAssetList_, reply);
            }
        }
    }
    return errCode;
}

int32_t MediaLibServiceStub::GetVideoAlbumAssetsOnRequest(MessageParcel &data,
    MessageParcel &reply)
{
    string albumDirPath;
    vector<string> selArgs;

    albumDirPath = data.ReadString();
    data.ReadStringVector(&selArgs);
    vector<unique_ptr<AlbumAsset>> albumAssetList = GetVideoAlbumAssets(albumDirPath, selArgs);

    return WriteVideoAlbum(albumAssetList, reply);
}

int32_t MediaLibServiceStub::GetImageAlbumAssetsOnRequest(MessageParcel &data,
    MessageParcel &reply)
{
    string albumDirPath;
    vector<string> selArgs;

    albumDirPath = data.ReadString();
    data.ReadStringVector(&selArgs);
    vector<unique_ptr<AlbumAsset>> albumAssetList = GetImageAlbumAssets(albumDirPath, selArgs);

    return WriteImageAlbum(albumAssetList, reply);
}

int32_t MediaLibServiceStub::GetVideoAssetsOnRequest(MessageParcel &data,
    MessageParcel &reply)
{
    string dirPath;
    vector<string> selArgs;

    dirPath = data.ReadString();
    data.ReadStringVector(&selArgs);
    vector<unique_ptr<VideoAsset>> videoAssetList = GetVideoAssets(dirPath, selArgs);

    return WriteVideoList(videoAssetList, reply);
}

int32_t MediaLibServiceStub::GetAudioAssetsOnRequest(MessageParcel &data,
    MessageParcel &reply)
{
    string dirPath;
    vector<string> selArgs;

    dirPath = data.ReadString();
    data.ReadStringVector(&selArgs);
    vector<unique_ptr<AudioAsset>> audioAssetList = GetAudioAssets(dirPath, selArgs);

    return WriteAudioList(audioAssetList, reply);
}

int32_t MediaLibServiceStub::GetMediaAssetsOnRequest(MessageParcel &data,
    MessageParcel &reply)
{
    string dirPath;
    vector<string> selArgs;

    dirPath = data.ReadString();
    data.ReadStringVector(&selArgs);
    vector<unique_ptr<MediaAsset>> mediaAssetList = GetMediaAssets(dirPath, selArgs);

    return WriteMediaList(mediaAssetList, reply);
}

int32_t MediaLibServiceStub::GetImageAssetsOnRequest(MessageParcel &data,
    MessageParcel &reply)
{
    string dirPath;
    vector<string> selArgs;

    dirPath = data.ReadString();
    data.ReadStringVector(&selArgs);
    vector<unique_ptr<ImageAsset>> imageAssetList = GetImageAssets(dirPath, selArgs);

    return WriteImageList(imageAssetList, reply);
}

bool MediaLibServiceStub::CreateMediaOnRequest(MessageParcel &data, MessageParcel &reply)
{
    MediaAsset mediaAsset;
    AssetType assetType;

    assetType = (AssetType)data.ReadInt32();
    ReadCommonData(mediaAsset, data);

    return CreateMediaAsset(assetType, mediaAsset);
}

bool MediaLibServiceStub::DeleteMediaOnRequest(MessageParcel &data, MessageParcel &reply)
{
    MediaAsset mediaAsset;
    AssetType assetType;

    assetType = (AssetType)data.ReadInt32();
    ReadCommonData(mediaAsset, data);

    return DeleteMediaAsset(assetType, mediaAsset);
}

bool MediaLibServiceStub::ModifyMediaOnRequest(MessageParcel &data, MessageParcel &reply)
{
    MediaAsset srcMediaAsset;
    MediaAsset dstMediaAsset;
    AssetType assetType;

    assetType = (AssetType)data.ReadInt32();
    ReadCommonData(srcMediaAsset, data);
    ReadCommonData(dstMediaAsset, data);

    return  ModifyMediaAsset(assetType, srcMediaAsset, dstMediaAsset);
}

bool MediaLibServiceStub::CopyMediaOnRequest(MessageParcel &data, MessageParcel &reply)
{
    MediaAsset srcMediaAsset;
    MediaAsset dstMediaAsset;
    AssetType assetType;

    assetType = (AssetType)data.ReadInt32();
    ReadCommonData(srcMediaAsset, data);
    ReadCommonData(dstMediaAsset, data);

    return CopyMediaAsset(assetType, srcMediaAsset, dstMediaAsset);
}

bool MediaLibServiceStub::CreateAlbumMediaOnRequest(MessageParcel &data, MessageParcel &reply)
{
    AlbumAsset albumAsset;
    AssetType assetType;

    assetType = (AssetType)data.ReadInt32();
    ReadMediaAlbumdata(albumAsset, data);

    return CreateMediaAlbumAsset(assetType, albumAsset);
}

bool MediaLibServiceStub::DeleteAlbumMediaOnRequest(MessageParcel &data, MessageParcel &reply)
{
    AlbumAsset albumAsset;
    AssetType assetType;
    string albumUri;

    assetType = (AssetType)data.ReadInt32();
    ReadMediaAlbumdata(albumAsset, data);
    albumUri = data.ReadString();

    return DeleteMediaAlbumAsset(assetType, albumAsset, albumUri);
}

bool MediaLibServiceStub::ModifyAlbumMediaOnRequest(MessageParcel &data, MessageParcel &reply)
{
    AlbumAsset srcAlbumAsset;
    AlbumAsset dstAlbumAsset;
    AssetType assetType;

    assetType = (AssetType)data.ReadInt32();
    ReadMediaAlbumdata(srcAlbumAsset, data);
    ReadMediaAlbumdata(dstAlbumAsset, data);

    string albumPath = data.ReadString();

    return  ModifyMediaAlbumAsset(assetType, srcAlbumAsset, dstAlbumAsset, albumPath);
}

int MediaLibServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    int32_t errCode = SUCCESS;

    MEDIA_INFO_LOG(" MediaLibServiceStub::OnRemoteRequest called.code: %{public}d", code);
    if (code > MEDIA_GET_VIDEOALBUM_ASSETS) {
        errCode = ProcessMediaOperationRequests(code, data, reply, option);
    } else {
        switch (code) {
            case MEDIA_GET_IMAGE_ASSETS:
                errCode = GetImageAssetsOnRequest(data, reply);
                break;
            case MEDIA_GET_MEDIA_ASSETS:
                errCode = GetMediaAssetsOnRequest(data, reply);
                break;
            case MEDIA_GET_AUDIO_ASSETS:
                errCode = GetAudioAssetsOnRequest(data, reply);
                break;
            case MEDIA_GET_VIDEO_ASSETS:
                errCode = GetVideoAssetsOnRequest(data, reply);
                break;
            case MEDIA_GET_IMAGEALBUM_ASSETS:
                errCode = GetImageAlbumAssetsOnRequest(data, reply);
                break;
            case MEDIA_GET_VIDEOALBUM_ASSETS:
                errCode = GetVideoAlbumAssetsOnRequest(data, reply);
                break;
            default:
                MEDIA_INFO_LOG("Default called");
                break;
        }
    }

    MEDIA_INFO_LOG(" MediaLibServiceStub: errCode: %{public}d", errCode);
    return errCode;
}

int MediaLibServiceStub::ProcessMediaOperationRequests(uint32_t code, MessageParcel &data,
                                                       MessageParcel &reply, MessageOption &option)
{
    int32_t errCode = SUCCESS;

    switch (code) {
        case MEDIA_CREATE_MEDIA_ASSET: {
            if ((CreateMediaOnRequest(data, reply) != true))
                errCode = FAIL;
            break;
        }
        case MEDIA_DELETE_MEDIA_ASSET: {
            if ((DeleteMediaOnRequest(data, reply) != true))
                errCode = FAIL;
            break;
        }
        case MEDIA_MODIFY_MEDIA_ASSET: {
            if ((ModifyMediaOnRequest(data, reply) != true))
                errCode = FAIL;
            break;
        }
        case MEDIA_COPY_MEDIA_ASSET: {
            if ((CopyMediaOnRequest(data, reply) != true))
                errCode = FAIL;
            break;
        }
        case MEDIA_CREATE_MEDIA_ALBUM_ASSET: {
            if ((CreateAlbumMediaOnRequest(data, reply) != true))
                errCode = FAIL;
            break;
        }
        case MEDIA_DELETE_MEDIA_ALBUM_ASSET: {
            if ((DeleteAlbumMediaOnRequest(data, reply) != true))
                errCode = FAIL;
            break;
        }
        case MEDIA_MODIFY_MEDIA_ALBUM_ASSET: {
            if ((ModifyAlbumMediaOnRequest(data, reply) != true))
                errCode = FAIL;
            break;
        }
        default:
            MEDIA_INFO_LOG("ProcessMediaOperationRequests :Default called");
            break;
    }

    return errCode;
}
} // namespace Media
} // namespace OHOS
