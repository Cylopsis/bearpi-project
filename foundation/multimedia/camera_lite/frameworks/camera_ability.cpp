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
#include "camera_ability.h"
#include "media_log.h"

using namespace std;
namespace OHOS {
namespace Media {
CameraAbility::CameraAbility() {}
CameraAbility::~CameraAbility() {}

list<CameraPicSize> CameraAbility::GetSupportedSizes(int format) const
{
    auto target = SizeMap_.find((uint32_t)format);
    return target->second;
}

std::list<int32_t> CameraAbility::GetSupportedAfModes() const
{
    return afModes;
}

std::list<int32_t> CameraAbility::GetSupportedAeModes() const
{
    return aeModes;
}

list<CameraPicSize> CameraAbility::GetSupportParameterRange(uint32_t key) const
{
    switch (key) {
        case PARAM_KEY_SIZE: {
            auto target = SizeMap_.find(PARAM_KEY_SIZE);
            return target->second;
        }
        default: {
            list<CameraPicSize> emptyList;
            return emptyList;
        }
    }
}

void CameraAbility::SetSupportParameterRange(uint32_t key, list<CameraPicSize> &rangeList)
{
    SizeMap_[key] = rangeList;
    supportProperties_.emplace(PARAM_KEY_SIZE);
}

void CameraAbility::SetSupportParameterRange(uint32_t key, list<int32_t> &rangeList)
{
    switch (key) {
        case CAM_AF_MODE:
            supportProperties_.emplace(key);
            afModes = rangeList;
            break;
        case CAM_AE_MODE:
            supportProperties_.emplace(key);
            aeModes = rangeList;
            break;
        default:
            break;
    }
}

bool CameraAbility::IsParameterSupport(uint32_t key) const
{
    return true;
}
} // namespace Media
} // namespace OHOS