/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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
#include "camera_service_client.h"
#include "media_log.h"
#include "samgr_lite.h"
#include "camera_type.h"
#include "camera_manager.h"
#include "meta_data.h"
#include "camera_client.h"

#include <string>
#include <cstdio>

using namespace std;
namespace OHOS {
namespace Media {
CameraServiceClient *CameraServiceClient::GetInstance()
{
    static CameraServiceClient client;
    return &client;
}

CameraServiceClient::CameraServiceClient()
{
    cameraClient_ = CameraClient::GetInstance();
}

CameraServiceClient::~CameraServiceClient()
{
    if (para_ != nullptr) {
        delete para_;
        para_ = nullptr;
    }
    UnregisterIpcCallback(sid_);
}

void CameraServiceClient::InitCameraServiceClient(CameraServiceCallback *callback)
{
    cameraServiceCb_ = callback;
    if (cameraClient_->InitCameraClient()) {
        MEDIA_INFO_LOG("Camera client initialize success.");
        proxy_ = cameraClient_->GetIClientProxy();
        list<string> cameraList = CameraServiceClient::GetInstance()->GetCameraIdList();
        cameraServiceCb_->OnCameraServiceInitialized(cameraList);
    }
}

int CameraServiceClient::Callback(void* owner, int code, IpcIo *reply)
{
    if (code != 0) {
        MEDIA_ERR_LOG("Callback error. (code=%d)", code);
        return -1;
    }
    if (owner == nullptr) {
        return -1;
    }
    CallBackPara* para = (CallBackPara*)owner;
    switch (para->funcId) {
        case CAMERA_SERVER_GET_CAMERA_ABILITY: {
            CameraServiceClient *client = static_cast<CameraServiceClient*>(para->data);
            uint32_t supportProperties = IpcIoPopUint32(reply);
            // Get supported resolution.
            uint32_t listSize = IpcIoPopUint32(reply);
            uint32_t size;
            list<CameraPicSize> supportSizeList;
            for (uint32_t i = 0; i < listSize; i++) {
                CameraPicSize *cameraPicSize = static_cast<CameraPicSize*>(IpcIoPopFlatObj(reply, &size));
                if (cameraPicSize != nullptr) {
                    supportSizeList.emplace_back(*cameraPicSize);    
                } else {
                    MEDIA_ERR_LOG("cameraPicSize is null");
                }
            }
            // Get supported AfModes.
            uint32_t afListSize = IpcIoPopUint32(reply);
            list<int32_t> afModeList;
            for (uint32_t i = 0; i < afListSize; i++) {
                afModeList.emplace_back(IpcIoPopInt32(reply));
            }
            // Get supported AeModes.
            uint32_t aeListSize = IpcIoPopUint32(reply);
            list<int32_t> aeModeList;
            for (uint32_t i = 0; i < aeListSize; i++) {
                aeModeList.emplace_back(IpcIoPopInt32(reply));
            }

            CameraAbility *cameraAbility = new (nothrow) CameraAbility;
            if (cameraAbility != nullptr) {
                cameraAbility->SetParameterRange(supportProperties, supportSizeList);
                cameraAbility->SetParameterRange(CAM_AF_MODE, afModeList);
                cameraAbility->SetParameterRange(CAM_AE_MODE, aeModeList);
                client->deviceAbilityMap_.insert(
                    pair<string, CameraAbility *>(client->cameraIdForAbility, cameraAbility));
            } else {
                MEDIA_ERR_LOG("Callback : cameraAbility construct failed.");
            }
            break;
        }
        case CAMERA_SERVER_GET_CAMERA_INFO: {
            CameraServiceClient *client = static_cast<CameraServiceClient*>(para->data);
            int32_t cameraType = IpcIoPopInt32(reply);
            int32_t cameraFacingType = IpcIoPopInt32(reply);
            CameraInfo *cameraInfo = new (nothrow) CameraInfoImpl(cameraType, cameraFacingType);
            if (cameraInfo != nullptr) {
                client->deviceInfoMap_.insert(pair<string, CameraInfo*>(client->cameraIdForInfo, cameraInfo));
            } else {
                MEDIA_ERR_LOG("Callback : cameraAbility construct failed.");
            }
            break;
        }

        case CAMERA_SERVER_GET_CAMERAIDLIST: {
            CameraServiceClient *client = static_cast<CameraServiceClient*>(para->data);
            uint32_t listSize = IpcIoPopUint32(reply);
            for (uint32_t i = 0; i < listSize; i++) {
                size_t sz;
                uint8_t *id = IpcIoPopString(reply, &sz);
                if (id != nullptr) {
                    string cameraId((const char*)id);
                    client->list_.emplace_back(cameraId);
                    MEDIA_INFO_LOG("Callback : cameraId %s", cameraId.c_str());
                } else {
                    MEDIA_ERR_LOG("IpcIoPopString error, id is null in camera_service_client!");
                }
            }
            break;
        }
        default:
            MEDIA_ERR_LOG("unsupport funcId.");
            break;
    }
    return 0;
}

list<string> CameraServiceClient::GetCameraIdList()
{
    if (list_.empty()) {
        IpcIo io;
        uint8_t tmpData[DEFAULT_IPC_SIZE];
        IpcIoInit(&io, tmpData, DEFAULT_IPC_SIZE, 0);
        CallBackPara para = {};
        para.funcId = CAMERA_SERVER_GET_CAMERAIDLIST;
        para.data = this;
        uint32_t ret = proxy_->Invoke(proxy_, CAMERA_SERVER_GET_CAMERAIDLIST, &io, &para, Callback);
        if (ret != 0) {
            MEDIA_ERR_LOG("Get cameraId list ipc  transmission failed. (ret=%d)", ret);
        }
    }
    return list_;
}

CameraAbility *CameraServiceClient::GetCameraAbility(string &cameraId)
{
    std::map<string, CameraAbility*>::iterator iter = deviceAbilityMap_.find(cameraId);
    if (iter != deviceAbilityMap_.end()) {
        return iter->second;
    }
    cameraIdForAbility = cameraId;
    IpcIo io;
    uint8_t tmpData[DEFAULT_IPC_SIZE];
    IpcIoInit(&io, tmpData, DEFAULT_IPC_SIZE, 0);
    IpcIoPushString(&io, cameraId.c_str());
    CallBackPara para = {};
    para.funcId = CAMERA_SERVER_GET_CAMERA_ABILITY;
    para.data = this;

    // wait for callback.
    uint32_t ret = proxy_->Invoke(proxy_, CAMERA_SERVER_GET_CAMERA_ABILITY, &io, &para, Callback);
    if (ret != 0) {
        MEDIA_ERR_LOG("Get camera ability ipc transmission failed. (ret=%d)", ret);
    }
    // find cameraAbility again.
    iter = deviceAbilityMap_.find(cameraId);
    if (iter != deviceAbilityMap_.end()) {
        return iter->second;
    }
    MEDIA_ERR_LOG("Get cameraAbility of camera %s from cameraService failded", cameraId.c_str());
    return nullptr;
}

CameraInfo *CameraServiceClient::GetCameraInfo(string &cameraId)
{
    std::map<string, CameraInfo*>::iterator iter = deviceInfoMap_.find(cameraId);
    if (iter != deviceInfoMap_.end()) {
        return iter->second;
    }
    cameraIdForInfo = cameraId;
    IpcIo io;
    uint8_t tmpData[DEFAULT_IPC_SIZE];
    IpcIoInit(&io, tmpData, DEFAULT_IPC_SIZE, 0);
    IpcIoPushString(&io, cameraId.c_str());
    CallBackPara para = {};
    para.funcId = CAMERA_SERVER_GET_CAMERA_INFO;
    para.data = this;
    // wait for callback.
    uint32_t ret = proxy_->Invoke(proxy_, CAMERA_SERVER_GET_CAMERA_INFO, &io, &para, Callback);
    if (ret != 0) {
        MEDIA_ERR_LOG("Get camera info ipc transmission failed. (ret=%d)", ret);
    }

    iter = deviceInfoMap_.find(cameraId);
    if (iter != deviceInfoMap_.end()) {
        return iter->second;
    }
    MEDIA_ERR_LOG("Get cameraInfo of camera %s from cameraService failded", cameraId.c_str());
    return nullptr;
}

int32_t CameraServiceClient::ServiceClientCallback(const IpcContext* context, void *ipcMsg, IpcIo *io, void *arg)
{
    if (ipcMsg == nullptr) {
        MEDIA_ERR_LOG("call back error, ipcMsg is null\n");
        return MEDIA_ERR;
    }
    if (arg == nullptr) {
        MEDIA_ERR_LOG("call back error, arg is null\n");
        return MEDIA_ERR;
    }
    CallBackPara* para = static_cast<CallBackPara*>(arg);
    CameraServiceClient *client = static_cast<CameraServiceClient*>(para->data);
    uint32_t funcId;
    (void)GetCode(ipcMsg, &funcId);
    MEDIA_INFO_LOG("ServiceClientCallback, funcId=%d", funcId);
    switch (funcId) {
        case ON_CAMERA_STATUS_CHANGE: {
            CameraServiceCallback::CameraStauts cameraStatus =
                static_cast<CameraServiceCallback::CameraStauts>(IpcIoPopInt32(io));
            string cameraId = para->cameraId;
            client->cameraServiceCb_->OnCameraStatusChange(cameraId, cameraStatus);
            break;
        }
        default: {
            MEDIA_ERR_LOG("unsupport funId\n");
            break;
        }
    }
    client->cameraClient_->ClearIpcMsg(ipcMsg);
    return MEDIA_OK;
}

void CameraServiceClient::CreateCamera(string cameraId)
{
    para_ = new (nothrow) CallBackPara;
    if (para_ == nullptr) {
        MEDIA_ERR_LOG("para_ is null, failed.");
        return;
    }
    para_->cameraId = cameraId;
    para_->data = this;
    int32_t ret = RegisterIpcCallback(ServiceClientCallback, 0, IPC_WAIT_FOREVER, &sid_, para_);
    if (ret != LITEIPC_OK) {
        MEDIA_ERR_LOG("RegisteIpcCallback failed, (ret=%d).", ret);
        return;
    }
    IpcIo io;
    uint8_t tmpData[DEFAULT_IPC_SIZE];
    IpcIoInit(&io, tmpData, DEFAULT_IPC_SIZE, 1);
    IpcIoPushString(&io, cameraId.c_str());
    IpcIoPushSvc(&io, &sid_);
    uint32_t ans = proxy_->Invoke(proxy_, CAMERA_SERVER_CREATE_CAMERA, &io, para_, Callback);
    if (ans != 0) {
        MEDIA_ERR_LOG("Create camera ipc  transmission failed. (ret=%d)", ans);
    }
}
}
}