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

#ifndef DISPLAYMGR_DISPLAY_MGR_ERRORS_H
#define DISPLAYMGR_DISPLAY_MGR_ERRORS_H

#include <errors.h>

namespace OHOS {
namespace DisplayMgr {
enum {
    /**
     *  Module type: Display Manager Service
     */
    DISPLAY_MODULE_TYPE_SERVICE = 0,
    /**
     *  Module type: Display Manager Kit
     */
    DISPLAY_MODULE_TYPE_KIT = 1
};

// offset of displaymgr error, only be used in this file.
constexpr ErrCode DISPLAYFWK_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_POWERMNG, DISPLAY_MODULE_TYPE_SERVICE);

enum {
    E_WRITE_PARCEL_ERROR = DISPLAYFWK_SERVICE_ERR_OFFSET,
    E_READ_PARCEL_ERROR,
    E_GET_SYSTEM_ABILITY_MANAGER_FAILED,
    E_GET_POWER_SERVICE_FAILED,
    E_ADD_DEATH_RECIPIENT_FAILED,
    E_INNER_ERR
};
} // namespace DisplayMgr
} // namespace OHOS

#endif // DISPLAYMGR_DISPLAY_MGR_ERRORS_H
