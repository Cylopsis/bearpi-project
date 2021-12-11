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

#ifndef PLUGIN_INFO_LOCK_H
#define PLUGIN_INFO_LOCK_H

#include "nocopyable.h"
#include "rwlock.h"
#include "singleton.h"

namespace OHOS {
namespace MultimediaPlugin {
class PluginInfoLock final : public NoCopyable {
public:
    // Use the read-write lock to mutually exclusive write plugin information and read plugin information operations,
    // where Register() plays the write role, CreateObject() and GetClassInfo() play the read role.
    OHOS::Utils::RWLock rwLock_;
    DECLARE_DELAYED_REF_SINGLETON(PluginInfoLock);
};
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // PLUGIN_INFO_LOCK_H