/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#include "camera_server.h"
#include <cstdio>
#include <list>
#include <string>
#include "camera_device.h"
#include "camera_service.h"
#include "camera_type.h"
#include "media_log.h"
#include "meta_data.h"
#include "surface.h"
#include "surface_impl.h"

using namespace std;
namespace OHOS {
namespace Media {
CameraServer *CameraServer::GetInstance()
{
    static CameraServer server;
    return &server;
}

void CameraServer::CameraServerRequestHandle(int funcId, void *origin, IpcIo *req, IpcIo *reply)
{
    switch (funcId) {
        case CAMERA_SERVER_GET_CAMERA_ABILITY:
            CameraServer::GetInstance()->GetCameraAbility(req, reply);
            break;
        case CAMERA_SERVER_GET_CAMERA_INFO:
            CameraServer::GetInstance()->GetCameraInfo(req, reply);
            break;
        case CAMERA_SERVER_GET_CAMERAIDLIST:
            CameraServer::GetInstance()->GetCameraIdList(req, reply);
            break;
        case CAMERA_SERVER_CREATE_CAMERA:
            CameraServer::GetInstance()->CreateCamera(req, reply);
            break;
        case CAMERA_SERVER_CLOSE_CAMERA:
            CameraServer::GetInstance()->CloseCamera(req, reply);
            break;
        case CAEMRA_SERVER_SET_CAMERA_CONFIG:
            CameraServer::GetInstance()->SetCameraConfig(req, reply);
            break;
        case CAMERA_SERVER_TRIGGER_LOOPING_CAPTURE:
            CameraServer::GetInstance()->TriggerLoopingCapture(req, reply);
            break;
        case CAMERA_SERVER_STOP_LOOPING_CAPTURE:
            CameraServer::GetInstance()->StopLoopingCapture(req, reply);
            break;
        case CAMERA_SERVER_TRIGGER_SINGLE_CAPTURE:
            CameraServer::GetInstance()->TriggerSingleCapture(req, reply);
            break;
        case CAMERA_SERVER_SET_CAMERA_CALLBACK:
            CameraServer::GetInstance()->SetCameraCallback(req, reply);
            break;
        case CAMERA_SERVER_SET_CODEC_FRAME_RATE:
            CameraServer::GetInstance()->setFrameRate(req, reply);
            break;
        case CAMERA_SERVER_SET_CODEC_BIT_RATE:
            CameraServer::GetInstance()->setBitRate(req, reply);
            break;
        case CAMERA_SERVER_SET_CODEC_RESOLUTION:
            CameraServer::GetInstance()->setResolution(req, reply);
            break;
        default:
            MEDIA_ERR_LOG("code not support:%d!", funcId);
            break;
    }
}
void CameraServer::InitCameraServer()
{
    CameraService *service = CameraService::GetInstance();
    service->Initialize();
}

void CameraServer::GetCameraAbility(IpcIo *req, IpcIo *reply)
{
    size_t sz;
    uint8_t *id = IpcIoPopString(req, &sz);
    if (id == nullptr) {
        MEDIA_ERR_LOG("IpcIoPopString error, id is null in camera_server");
        return;
    }
    string cameraId((const char*)id);
    CameraAbility *ability = CameraService::GetInstance()->GetCameraAbility(cameraId);
    if (ability == nullptr) {
        return;
    }
    list<CameraPicSize> supportSizeList = ability->GetSupportedSizes(PARAM_KEY_SIZE);
    uint32_t supportProperties = 0;
    IpcIoPushUint32(reply, supportProperties);
    uint32_t listSize = supportSizeList.size();
    IpcIoPushUint32(reply, listSize);
    for (auto supportSizeItem : supportSizeList) {
        IpcIoPushFlatObj(reply, &supportSizeItem, sizeof(CameraPicSize));
    }
    // af
    list<int32_t> afModeList = ability->GetSupportedAfModes();
    uint32_t afListSize = afModeList.size();
    IpcIoPushUint32(reply, afListSize);
    for (auto supportAfMode : afModeList) {
        IpcIoPushInt32(reply, supportAfMode);
    }
    // ae
    list<int32_t> aeModeList = ability->GetSupportedAeModes();
    uint32_t aeListSize = aeModeList.size();
    IpcIoPushUint32(reply, aeListSize);
    for (auto supportAeMode : aeModeList) {
        IpcIoPushInt32(reply, supportAeMode);
    }
}

void CameraServer::GetCameraInfo(IpcIo *req, IpcIo *reply)
{
    size_t sz;
    uint8_t *id = IpcIoPopString(req, &sz);
    if (id == nullptr) {
        MEDIA_ERR_LOG("IpcIoPopString error, id is null in camera_server");
        return;
    }
    string cameraId((const char*)id);
    CameraInfo *info = CameraService::GetInstance()->GetCameraInfo(cameraId);
    if (info == nullptr) {
        return;
    }
    IpcIoPushInt32(reply, info->GetCameraType());
    IpcIoPushInt32(reply, info->GetCameraFacingType());
}

void CameraServer::GetCameraIdList(IpcIo *req, IpcIo *reply)
{
    list<string> cameraIdList = CameraService::GetInstance()->GetCameraIdList();
    IpcIoPushUint32(reply, cameraIdList.size());
    for (string cameraId : cameraIdList) {
        IpcIoPushString(reply, cameraId.c_str());
    }
}

void CameraServer::CreateCamera(IpcIo *req, IpcIo *reply)
{
    size_t sz;
    uint8_t *id = IpcIoPopString(req, &sz);
    if (id == nullptr) {
        MEDIA_ERR_LOG("IpcIoPopString error, id is null in camera_server");
        return;
    }
    string cameraId((const char*)id);
    int32_t cameraStatus = CameraService::GetInstance()->CreateCamera(cameraId);
    SvcIdentity *sid = IpcIoPopSvc(req);
    if (sid == nullptr) {
        MEDIA_ERR_LOG("sid is null, failed.");
        return;
    }
#ifdef __LINUX__
    BinderAcquire(sid->ipcContext, sid->handle);
#endif
    OnCameraStatusChange(cameraStatus, sid);
}

void CameraServer::CloseCamera(IpcIo *req, IpcIo *reply)
{
    MEDIA_INFO_LOG("CloseCamera enter.");
    size_t sz;
    uint8_t *id = IpcIoPopString(req, &sz);
    if (id == nullptr) {
        MEDIA_ERR_LOG("IpcIoPopString error, id is null in camera_server");
        return;
    }
    string cameraId((const char*)id);
    int32_t cameraStatus = CameraService::GetInstance()->CloseCamera(cameraId);
    SvcIdentity *sid = IpcIoPopSvc(req);
    if (sid == nullptr) {
        MEDIA_ERR_LOG("sid is null, failed.");
        return;
    }
#ifdef __LINUX__
    BinderAcquire(sid->ipcContext, sid->handle);
#endif
    OnCameraStatusChange(cameraStatus, sid);
}

void CameraServer::SetCameraConfig(IpcIo *req, IpcIo *reply)
{
    size_t sz;
    uint8_t *id = IpcIoPopString(req, &sz);
    if (id == nullptr) {
        MEDIA_ERR_LOG("IpcIoPopString error, id is null in camera_server");
        return;
    }
    string cameraId((const char*)id);
    CameraDevice *device_ = CameraService::GetInstance()->GetCameraDevice(cameraId);
    if (device_ == nullptr) {
        MEDIA_ERR_LOG("device_ is null in camera_server.cpp!");
        return;
    }
    int32_t setStatus = device_->SetCameraConfig();
    IpcIoPushInt32(reply, setStatus);
    OnCameraConfigured(setStatus);
}

void CameraServer::SetCameraCallback(IpcIo *req, IpcIo *reply)
{
    SvcIdentity *sid = IpcIoPopSvc(req);
    if (sid == nullptr) {
        MEDIA_ERR_LOG("sid is null, failed.");
        return;
    }
    sid_ = *sid;
#ifdef __LINUX__
    BinderAcquire(sid_.ipcContext, sid_.handle);
#endif
}

FrameConfig *DeserializeFrameConfig(IpcIo &io)
{
    int32_t type = IpcIoPopInt32(&io);
    auto fc = new FrameConfig(type);

    uint32_t surfaceNum = IpcIoPopUint32(&io);
    for (uint32_t i = 0; i < surfaceNum; i++) {
        Surface *surface = SurfaceImpl::GenericSurfaceByIpcIo(io);
        if (surface == nullptr) {
            MEDIA_ERR_LOG("Camera server receive null surface.");
            delete fc;
            return nullptr;
        }
        fc->AddSurface(*surface);
    }

    int32_t qfactor = IpcIoPopInt32(&io);
    if (qfactor >= 0) {
        fc->SetParameter(PARAM_KEY_IMAGE_ENCODE_QFACTOR, qfactor);
    }
    int32_t frameRate = IpcIoPopInt32(&io);
    fc->SetParameter(PARAM_KEY_STREAM_FPS, frameRate);
    MEDIA_INFO_LOG("frameRate is %d", frameRate);
    int32_t streamFormat = IpcIoPopInt32(&io);
    fc->SetParameter(CAM_IMAGE_FORMAT, streamFormat);
    MEDIA_INFO_LOG("streamFormat is %d", streamFormat);
    BuffPtr *dataBuff = IpcIoPopDataBuff(&io);
    if (dataBuff == nullptr || dataBuff->buff == nullptr) {
        MEDIA_ERR_LOG("dataBuff is nullptr.");
        return fc;
    }
    uint8_t *data = (uint8_t *)dataBuff->buff;
    fc->SetVendorParameter((uint8_t *)dataBuff->buff, dataBuff->buffSz);
    FreeBuffer(nullptr, dataBuff->buff);
    return fc;
}

void CameraServer::SetFrameConfig(IpcIo *req, IpcIo *reply)
{
    size_t sz;
    uint8_t *id = IpcIoPopString(req, &sz);
    if (id == nullptr) {
        MEDIA_ERR_LOG("IpcIoPopString error, id is null in camera_server");
        return;
    }
    string cameraId((const char*)id);
    int32_t streamId = IpcIoPopInt32(req);
    MEDIA_ERR_LOG("SetFrameConfig streamId(%d).", streamId);
    CameraDevice *device_ = CameraService::GetInstance()->GetCameraDevice(cameraId);
    if (device_ == nullptr) {
        MEDIA_ERR_LOG("device_ is null in camera_server.cpp!");
        return;
    }
    FrameConfig *fc = DeserializeFrameConfig(*req);
    if (fc == nullptr) {
        MEDIA_ERR_LOG("Deserialize frame config failed.");
        return;
    }
    delete fc;
}


void CameraServer::TriggerLoopingCapture(IpcIo *req, IpcIo *reply)
{
    size_t sz;
    uint8_t *id = IpcIoPopString(req, &sz);
    if (id == nullptr) {
        MEDIA_ERR_LOG("IpcIoPopString error, id is null in camera_server");
        return;
    }
    string cameraId((const char*)id);
    CameraDevice *device_ = CameraService::GetInstance()->GetCameraDevice(cameraId);
    if (device_ == nullptr) {
        MEDIA_ERR_LOG("device_ is null in camera_server.cpp!");
        return;
    }
    FrameConfig *fc = DeserializeFrameConfig(*req);
    if (fc == nullptr) {
        MEDIA_ERR_LOG("Deserialize frame config failed.");
        return;
    }
    uint32_t streamId = 0;
    int32_t loopingCaptureStatus = device_->TriggerLoopingCapture(*fc, &streamId);
    OnTriggerLoopingCaptureFinished(loopingCaptureStatus, streamId);
    delete fc;
}

void CameraServer::TriggerSingleCapture(IpcIo *req, IpcIo *reply)
{
    size_t sz;
    uint8_t *id = IpcIoPopString(req, &sz);
    if (id == nullptr) {
        MEDIA_ERR_LOG("IpcIoPopString error, id is null in camera_server");
        return;
    }
    string cameraId((const char*)id);
    CameraDevice *device_ = CameraService::GetInstance()->GetCameraDevice(cameraId);
    if (device_ == nullptr) {
        MEDIA_ERR_LOG("device_ is null in camera_server.cpp!");
        return;
    }
    FrameConfig *fc = DeserializeFrameConfig(*req);
    if (fc == nullptr) {
        MEDIA_ERR_LOG("Deserialize frame config failed.");
        return;
    }
    uint32_t streamId = 0;
    int32_t singleCaptureStatus = device_->TriggerSingleCapture(*fc, &streamId);
    OnTriggerSingleCaptureFinished(singleCaptureStatus);
    delete fc;
}

void CameraServer::StopLoopingCapture(IpcIo *req, IpcIo *reply)
{
    MEDIA_INFO_LOG("StopLoopingCapture in camera_server.cpp!");
    size_t sz;
    uint8_t *id = IpcIoPopString(req, &sz);
    if (id == nullptr) {
        MEDIA_ERR_LOG("IpcIoPopString error, id is null in camera_server");
        return;
    }
    string cameraId((const char*)id);
    CameraDevice *device_ = CameraService::GetInstance()->GetCameraDevice(cameraId);
    if (device_ == nullptr) {
        MEDIA_INFO_LOG("device_ is  null in camera_server.cpp!");
        return;
    }
    device_->StopLoopingCapture();
}

void CameraServer::OnCameraStatusChange(int32_t ret, SvcIdentity *sid)
{
    IpcIo io;
    uint8_t tmpData[DEFAULT_IPC_SIZE];
    IpcIoInit(&io, tmpData, DEFAULT_IPC_SIZE, 0);
    IpcIoPushInt32(&io, ret);
    int32_t ans = Transact(nullptr, *sid, ON_CAMERA_STATUS_CHANGE, &io, nullptr, LITEIPC_FLAG_ONEWAY, nullptr);
    if (ans != LITEIPC_OK) {
        MEDIA_ERR_LOG("Create camera callback : on camera status change failed.");
    }
}

void CameraServer::OnTriggerSingleCaptureFinished(int32_t ret)
{
    IpcIo io;
    uint8_t tmpData[DEFAULT_IPC_SIZE];
    IpcIoInit(&io, tmpData, DEFAULT_IPC_SIZE, 0);
    IpcIoPushInt32(&io, ret);
    int32_t ans = Transact(nullptr, sid_, ON_TRIGGER_SINGLE_CAPTURE_FINISHED,
        &io, nullptr, LITEIPC_FLAG_ONEWAY, nullptr);
    if (ans != LITEIPC_OK) {
        MEDIA_ERR_LOG("Trigger single capture callback : on trigger single capture frame finished failed.");
        return;
    }
}

void CameraServer::OnTriggerLoopingCaptureFinished(int32_t ret, int32_t streamId)
{
    IpcIo io;
    uint8_t tmpData[DEFAULT_IPC_SIZE];
    IpcIoInit(&io, tmpData, DEFAULT_IPC_SIZE, 0);
    IpcIoPushInt32(&io, ret);
    int32_t ans = Transact(nullptr, sid_, ON_TRIGGER_LOOPING_CAPTURE_FINISHED,
        &io, nullptr, LITEIPC_FLAG_ONEWAY, nullptr);
    if (ans != LITEIPC_OK) {
        MEDIA_ERR_LOG("Trigger looping capture callback : on trigger looping capture finished failed.");
    }
}

void CameraServer::OnCameraConfigured(int32_t ret)
{
    IpcIo io;
    uint8_t tmpData[DEFAULT_IPC_SIZE];
    IpcIoInit(&io, tmpData, DEFAULT_IPC_SIZE, 0);
    IpcIoPushInt32(&io, ret);
    int32_t ans = Transact(nullptr, sid_, ON_CAMERA_CONFIGURED,
        &io, nullptr, LITEIPC_FLAG_ONEWAY, nullptr);
    if (ans != LITEIPC_OK) {
        MEDIA_ERR_LOG("Camera config callback : on trigger looping capture finished failed.");
    }
}

void CameraServer::setFrameRate(IpcIo *req, IpcIo *reply)
{
    size_t sz;
    uint8_t *id = IpcIoPopString(req, &sz);
    if (id == nullptr) {
        MEDIA_ERR_LOG("IpcIoPopString error, id is null in camera_server");
        return;
    }
    string cameraId((const char*)id);
    CameraDevice *device_ = CameraService::GetInstance()->GetCameraDevice(cameraId);
    if (device_ == nullptr) {
        MEDIA_ERR_LOG("device_ is null in camera_server.cpp!");
        return;
    }
    uint32_t frameRate = IpcIoPopUint32(req);
    MEDIA_ERR_LOG("frameRate is %d", frameRate);
    device_->SetRecordCodecFrameRate(frameRate);
}

void CameraServer::setBitRate(IpcIo *req, IpcIo *reply)
{
    size_t sz;
    uint8_t *id = IpcIoPopString(req, &sz);
    if (id == nullptr) {
        MEDIA_ERR_LOG("IpcIoPopString error, id is null in camera_server");
        return;
    }
    string cameraId((const char*)id);
    CameraDevice *device_ = CameraService::GetInstance()->GetCameraDevice(cameraId);
    if (device_ == nullptr) {
        MEDIA_ERR_LOG("device_ is null in camera_server.cpp!");
        return;
    }
    uint32_t bitRate = IpcIoPopUint32(req);
    device_->SetRecordCodecBitRate(bitRate);
}

void CameraServer::setResolution(IpcIo *req, IpcIo *reply)
{
    size_t sz;
    uint8_t *id = IpcIoPopString(req, &sz);
    if (id == nullptr) {
        MEDIA_ERR_LOG("IpcIoPopString error, id is null in camera_server");
        return;
    }
    string cameraId((const char*)id);
    CameraDevice *device_ = CameraService::GetInstance()->GetCameraDevice(cameraId);
    if (device_ == nullptr) {
        MEDIA_ERR_LOG("device_ is null in camera_server.cpp!");
        return;
    }
    uint32_t width = IpcIoPopUint32(req);
    uint32_t height = IpcIoPopUint32(req);
    device_->SetRecordCodecResolution(width, height);
}
} // namespace Media
} // namespace OHOS