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

#include "plugin_fw.h"
#include "hilog/log.h"
#include "singleton.h"
#include "log_tags.h"
#include "impl_class_mgr.h"
#include "plugin_info_lock.h"
#include "plugin_mgr.h"

namespace OHOS {
namespace MultimediaPlugin {
using std::map;
using std::string;
using std::vector;
using OHOS::Utils::RWLock;
using OHOS::Utils::UniqueReadGuard;
using OHOS::Utils::UniqueWriteGuard;
using namespace OHOS::HiviewDFX;

static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "PluginFw" };

uint32_t PluginFw::Register(const vector<string> &canonicalPaths)
{
    HiLog::Debug(LABEL, "plugin register.");
    // Use the read-write lock to mutually exclusive write plugin information and read plugin information operations,
    // where Register() plays the write role.
    UniqueWriteGuard<RWLock> lk(DelayedRefSingleton<PluginInfoLock>::GetInstance().rwLock_);
    return pluginMgr_.Register(canonicalPaths);
}

PluginClassBase *PluginFw::CreateObject(uint16_t interfaceID, const string &className, uint32_t &errorCode)
{
    // Use the read-write lock to mutually exclusive write plugin information and read plugin information operations,
    // where CreateObject() plays the read role.
    UniqueReadGuard<RWLock> lk(DelayedRefSingleton<PluginInfoLock>::GetInstance().rwLock_);
    return implClassMgr_.CreateObject(interfaceID, className, errorCode);
}

PluginClassBase *PluginFw::CreateObject(uint16_t interfaceID, uint16_t serviceType,
                                        const map<string, AttrData> &capabilities,
                                        const PriorityScheme &priorityScheme, uint32_t &errorCode)
{
    // Use the read-write lock to mutually exclusive write plugin information and read plugin information operations,
    // where CreateObject() plays the read role.
    UniqueReadGuard<RWLock> lk(DelayedRefSingleton<PluginInfoLock>::GetInstance().rwLock_);
    return implClassMgr_.CreateObject(interfaceID, serviceType, capabilities, priorityScheme, errorCode);
}

uint32_t PluginFw::PluginFwGetClassInfo(uint16_t interfaceID, uint16_t serviceType,
                                        const map<std::string, AttrData> &capabilities,
                                        vector<ClassInfo> &classesInfo)
{
    // Use the read-write lock to mutually exclusive write plugin information and read plugin information operations,
    // where GetClassInfo() plays the read role.
    UniqueReadGuard<RWLock> lk(DelayedRefSingleton<PluginInfoLock>::GetInstance().rwLock_);
    return implClassMgr_.ImplClassMgrGetClassInfo(interfaceID, serviceType, capabilities, classesInfo);
}

// ------------------------------- private method -------------------------------
PluginFw::PluginFw()
    : pluginMgr_(DelayedRefSingleton<PluginMgr>::GetInstance()),
      implClassMgr_(DelayedRefSingleton<ImplClassMgr>::GetInstance()) {}

PluginFw::~PluginFw() {}
} // namespace MultimediaPlugin
} // namespace OHOS
