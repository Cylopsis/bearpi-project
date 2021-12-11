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

#include "display_mgr_proxy.h"

#include "display_common.h"

namespace OHOS {
namespace DisplayMgr {
bool DisplayMgrProxy::SetScreenState(ScreenState state)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "write descriptor failed!");
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, static_cast<uint32_t>(state), false);

    int ret = remote->SendRequest(static_cast<int32_t>(IDisplayMgr::SET_POWER_STATE), data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "SendRequest is failed, error code: %d", ret);
        return result;
    }
    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "Readback fail!");
    }

    return result;
}

bool DisplayMgrProxy::SetBrightness(int32_t value)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "write descriptor failed!");
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Int32, value, false);

    int ret = remote->SendRequest(static_cast<int32_t>(IDisplayMgr::SET_BRIGHTNESS), data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "SendRequest is failed, error code: %d", ret);
        return result;
    }
    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "Readback fail!");
    }

    return result;
}
} // namespace DisplayMgr
} // namespace OHOS
