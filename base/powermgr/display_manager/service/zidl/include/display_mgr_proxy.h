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

#ifndef DISPLAYMGR_DISPLAY_MGR_PROXY_H
#define DISPLAYMGR_DISPLAY_MGR_PROXY_H

#include <iremote_proxy.h>

#include "idisplay_mgr.h"

namespace OHOS {
namespace DisplayMgr {
class DisplayMgrProxy : public IRemoteProxy<IDisplayMgr> {
public:
    explicit DisplayMgrProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IDisplayMgr>(impl) {}
    ~DisplayMgrProxy() override = default;

    bool SetScreenState(ScreenState state) override;
    bool SetBrightness(int32_t value) override;

private:
    static inline BrokerDelegator<DisplayMgrProxy> delegator_;
};
} // namespace DisplayMgr
} // namespace OHOS
#endif // DISPLAYMGR_DISPLAY_MGR_PROXY_H
