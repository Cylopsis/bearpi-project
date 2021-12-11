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

#ifndef DISPLAYMGR_IDISPLAY_MGR_H
#define DISPLAYMGR_IDISPLAY_MGR_H

#include <iremote_broker.h>

#include "display_info.h"

namespace OHOS {
namespace DisplayMgr {
class IDisplayMgr : public IRemoteBroker {
public:
    enum {
        SET_POWER_STATE = 0,
        SET_BRIGHTNESS,
    };

    virtual bool SetScreenState(ScreenState state) = 0;
    virtual bool SetBrightness(int32_t value) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.displaymgr.IDisplayMgr");
};
} // namespace DisplayMgr
} // namespace OHOS
#endif // DISPLAYMGR_IDISPLAY_MGR_H
