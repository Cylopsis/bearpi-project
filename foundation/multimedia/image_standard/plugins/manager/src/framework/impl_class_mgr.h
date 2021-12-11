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

#ifndef IMPL_CLASS_MGR_H
#define IMPL_CLASS_MGR_H

#include <list>
#include <string>
#include "json.hpp"
#include "nocopyable.h"
#include "plugin_common_type.h"
#include "plugin_errors.h"
#include "pointer_key_map.h"
#include "priority_scheme.h"
#include "singleton.h"

namespace OHOS {
namespace MultimediaPlugin {
class ImplClass;
class Plugin;
class PluginClassBase;

class ImplClassMgr final : public NoCopyable {
public:
    uint32_t AddClass(std::weak_ptr<Plugin> &plugin, const nlohmann::json &classInfo);
    void DeleteClass(const std::weak_ptr<Plugin> &plugin);
    PluginClassBase *CreateObject(uint16_t interfaceID, const std::string &className, uint32_t &errorCode);
    PluginClassBase *CreateObject(uint16_t interfaceID, uint16_t serviceType,
                                  const std::map<std::string, AttrData> &capabilities,
                                  const PriorityScheme &priorityScheme, uint32_t &errorCode);
    uint32_t ImplClassMgrGetClassInfo(uint16_t interfaceID, uint16_t serviceType,
                          const std::map<std::string, AttrData> &capabilities, std::vector<ClassInfo> &classesInfo);
    std::shared_ptr<ImplClass> GetImplClass(const std::string &packageName, const std::string &className);
    DECLARE_DELAYED_REF_SINGLETON(ImplClassMgr);

private:
    std::shared_ptr<ImplClass> SearchByPriority(const std::list<std::shared_ptr<ImplClass>> &candidates,
                                                const PriorityScheme &priorityScheme);
    std::shared_ptr<ImplClass> SearchSimplePriority(const std::list<std::shared_ptr<ImplClass>> &candidates);
    uint32_t ComparePriority(const AttrData &lhs, const AttrData &rhs, PriorityType type);
    uint32_t CompareBoolPriority(const AttrData &lhs, const AttrData &rhs, PriorityType type);
    uint32_t CompareUint32Priority(const AttrData &lhs, const AttrData &rhs, PriorityType type);
    uint32_t CompareStringPriority(const AttrData &lhs, const AttrData &rhs, PriorityType type);

    using NameClassMultimap = PointerKeyMultimap<const std::string, std::shared_ptr<ImplClass>>;
    using ServiceClassMultimap = std::multimap<uint32_t, std::shared_ptr<ImplClass>>;
    NameClassMultimap classMultimap_;
    ServiceClassMultimap srvSearchMultimap_;
};
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // IMPL_CLASS_MGR_H
