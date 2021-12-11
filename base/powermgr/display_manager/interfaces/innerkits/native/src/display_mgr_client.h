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

#ifndef DISPLAYMGR_DISPLAY_MGR_CLIENT_H
#define DISPLAYMGR_DISPLAY_MGR_CLIENT_H

#include <iremote_object.h>
#include <singleton.h>

#include "display_info.h"
#include "idisplay_mgr.h"

namespace OHOS {
namespace DisplayMgr {
class DisplayMgrClient : public DelayedRefSingleton<DisplayMgrClient> {
    DECLARE_DELAYED_REF_SINGLETON(DisplayMgrClient);

public:
    bool SetScreenState(ScreenState state);
    bool SetBrightness(int32_t value);

private:
    class DisplayDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit DisplayDeathRecipient(DisplayMgrClient& client) : client_(client) {}
        ~DisplayDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        DisplayMgrClient& client_;
    };

    sptr<IDisplayMgr> GetProxy();
    void OnRemoteDied(const wptr<IRemoteObject>& remote);

    std::mutex mutex_;
    sptr<IDisplayMgr> proxy_{nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_{nullptr};
};
} // namespace DisplayMgr
} // namespace OHOS
#endif // DISPLAYMGR_DISPLAY_MGR_CLIENT_H
