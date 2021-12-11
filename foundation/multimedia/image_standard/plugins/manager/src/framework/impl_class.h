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

#ifndef IMPL_CLASS_H
#define IMPL_CLASS_H

#include <map>
#include <mutex>
#include <set>
#include <string>
#include "json.hpp"
#include "attr_data.h"
#include "capability.h"
#include "impl_class_key.h"
#include "plugin_errors.h"

namespace OHOS {
namespace MultimediaPlugin {
class Plugin;
class PluginClassBase;

enum class ClassState : int32_t {
    CLASS_STATE_UNREGISTER = 0,
    CLASS_STATE_REGISTERED
};

class ImplClass final {
public:
    ImplClass();
    ~ImplClass() = default;
    static uint32_t MakeServiceFlag(uint16_t interfaceID, uint16_t serviceType)
    {
        return (((static_cast<uint32_t>(interfaceID)) << SERVICETYPE_BIT_NUM) | serviceType);
    }
    static uint16_t MakeIID(uint32_t serviceFlag)
    {
        return ((serviceFlag >> SERVICETYPE_BIT_NUM) & IID_MASK);
    }
    uint32_t Register(std::weak_ptr<Plugin> &plugin, const nlohmann::json &classInfo);
    PluginClassBase *CreateObject(uint32_t &errorCode);
    std::weak_ptr<Plugin> GetPluginRef() const;
    const std::string &GetClassName() const;
    const std::string &GetPackageName() const;
    bool IsSupport(uint16_t interfaceID) const;
    void OnObjectDestroy();
    const std::set<uint32_t> &GetServices() const;
    bool IsCompatible(const std::map<std::string, AttrData> &caps) const;

    uint16_t GetPriority() const
    {
        return priority_;
    }

    const AttrData *GetCapability(const std::string &key) const;
    const std::map<std::string, AttrData> &GetCapability() const;

    static constexpr uint8_t SERVICETYPE_BIT_NUM = 16;
    static constexpr uint32_t IID_MASK = 0xFFFF;

private:
    bool AnalysisServices(const nlohmann::json &classInfo);
    bool AnalysisMaxInstance(const nlohmann::json &classInfo);
    PluginClassBase *DoCreateObject(std::shared_ptr<Plugin> &plugin);
    static constexpr uint16_t INSTANCE_NO_LIMIT_NUM = 0;
    static std::string emptyString_;
    // dynDataLock_:
    // for data that only changes in the register, we don't call it dynamic data.
    // non-dynamic data are protected by other means, that is: mutual exclusion between
    // the register and createObject processes.
    // current dynamic data includes:
    // instanceNum_.
    std::recursive_mutex dynDataLock_;
    ClassState state_ = ClassState::CLASS_STATE_UNREGISTER;
    std::string className_;
    std::set<uint32_t> services_;
    uint16_t priority_ = 0;
    uint16_t maxInstance_ = 0;
    Capability capability_;
    std::weak_ptr<Plugin> pluginRef_;
    ImplClassKey selfKey_;
    uint16_t instanceNum_ = 0;
};
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // IMPL_CLASS_H
