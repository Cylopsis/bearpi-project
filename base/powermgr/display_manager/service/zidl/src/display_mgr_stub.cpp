/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "display_mgr_stub.h"

#include <message_parcel.h>

#include "display_common.h"

namespace OHOS {
namespace DisplayMgr {
int32_t DisplayMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    DISPLAY_HILOGD(MODULE_SERVICE, "DisplayMgrStub::OnRemoteRequest, cmd = %d, flags= %d", code, option.GetFlags());
    std::u16string descripter = DisplayMgrStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        DISPLAY_HILOGE(MODULE_SERVICE, "DisplayMgrStub::OnRemoteRequest failed, descriptor is not matched!");
        return E_GET_POWER_SERVICE_FAILED;
    }

    switch (code) {
        case static_cast<int32_t>(IDisplayMgr::SET_POWER_STATE):
            return SetScreenStateStub(data, reply);
        case static_cast<int32_t>(IDisplayMgr::SET_BRIGHTNESS):
            return SetBrightnessStub(data, reply);
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t DisplayMgrStub::SetScreenStateStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t state = 0;

    READ_PARCEL_WITH_RET(data, Uint32, state, E_READ_PARCEL_ERROR);

    bool ret = SetScreenState(static_cast<ScreenState>(state));
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write SetScreenState return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayMgrStub::SetBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    int32_t value = 0;

    READ_PARCEL_WITH_RET(data, Int32, value, E_READ_PARCEL_ERROR);

    bool ret = SetBrightness(value);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write SetBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}
} // namespace DisplayMgr
} // namespace OHOS
