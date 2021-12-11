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

#include "impl_class.h"
#include <algorithm>
#include "hilog/log.h"
#include "impl_class_key.h"
#include "json_helper.h"
#include "log_tags.h"
#include "plugin.h"
#include "plugin_class_base.h"
#include "plugin_common_type.h"
#include "plugin_export.h"

namespace OHOS {
namespace MultimediaPlugin {
using nlohmann::json;
using std::map;
using std::recursive_mutex;
using std::set;
using std::shared_ptr;
using std::size_t;
using std::string;
using std::weak_ptr;
using namespace OHOS::HiviewDFX;

static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "ImplClass" };
string ImplClass::emptyString_;

ImplClass::ImplClass() : selfKey_(*this)
{}

uint32_t ImplClass::Register(weak_ptr<Plugin> &plugin, const json &classInfo)
{
    if (state_ != ClassState::CLASS_STATE_UNREGISTER) {
        // repeat registration
        HiLog::Error(LABEL, "repeat registration.");
        return ERR_INTERNAL;
    }

    if (JsonHelper::GetStringValue(classInfo, "className", className_) != SUCCESS) {
        HiLog::Error(LABEL, "read className failed.");
        return ERR_INVALID_PARAMETER;
    }
    HiLog::Debug(LABEL, "register class: %{public}s.", className_.c_str());

    if (!AnalysisServices(classInfo)) {
        HiLog::Error(LABEL, "failed to analysis services for class %{public}s.", className_.c_str());
        return ERR_INVALID_PARAMETER;
    }

    uint32_t result = JsonHelper::GetUint16Value(classInfo, "priority", priority_);
    if (result != SUCCESS) {
        if (result != ERR_NO_TARGET) {
            HiLog::Error(LABEL, "read priority failed, result: %{public}u.", result);
            return ERR_INVALID_PARAMETER;
        }
        // priority is optional, and default zero.
        priority_ = 0;
    }
    HiLog::Debug(LABEL, "get class priority: %{public}u.", priority_);

    if (!AnalysisMaxInstance(classInfo)) {
        HiLog::Error(LABEL, "failed to analysis maxInstance for class %{public}s.", className_.c_str());
        return ERR_INVALID_PARAMETER;
    }
    HiLog::Debug(LABEL, "get class maxInstance: %{public}u.", maxInstance_);

    if (JsonHelper::CheckElementExistence(classInfo, "capabilities") == SUCCESS) {
        capability_.SetCapability(classInfo["capabilities"]);
    }
    pluginRef_ = plugin;
    state_ = ClassState::CLASS_STATE_REGISTERED;
    return SUCCESS;
}

PluginClassBase *ImplClass::CreateObject(uint32_t &errorCode)
{
    errorCode = ERR_INTERNAL;
    if (state_ != ClassState::CLASS_STATE_REGISTERED) {
        HiLog::Error(LABEL, "failed to create for unregistered, className: %{public}s.", className_.c_str());
        return nullptr;
    }

    auto sharedPlugin = pluginRef_.lock();
    if (sharedPlugin == nullptr) {
        HiLog::Error(LABEL, "failed to dereference Plugin, className: %{public}s.", className_.c_str());
        return nullptr;
    }

    HiLog::Debug(LABEL, "create object, className: %{public}s.", className_.c_str());

    std::unique_lock<std::recursive_mutex> guard(dynDataLock_);
    if (maxInstance_ != INSTANCE_NO_LIMIT_NUM && instanceNum_ >= maxInstance_) {
        HiLog::Error(LABEL, "failed to create for limit, currentNum: %{public}u, maxNum: %{public}u, \
                     className: %{public}s.",
                     instanceNum_, maxInstance_, className_.c_str());
        guard.unlock();
        errorCode = ERR_INSTANCE_LIMIT;
        return nullptr;
    }

    if (instanceNum_ == 0) {
        if (sharedPlugin->Ref() != SUCCESS) {
            return nullptr;
        }
    }

    PluginClassBase *object = DoCreateObject(sharedPlugin);
    if (object == nullptr) {
        HiLog::Error(LABEL, "create object result null, className: %{public}s.", className_.c_str());
        goto CREATE_INTERNAL_ERROR_EXIT;
    }

    ++instanceNum_;
    HiLog::Debug(LABEL, "create object success, InstanceNum: %{public}u.", instanceNum_);
    guard.unlock();

    errorCode = SUCCESS;
    return object;

CREATE_INTERNAL_ERROR_EXIT:
    if (instanceNum_ == 0) {
        sharedPlugin->DeRef();
    }
    return nullptr;
}

weak_ptr<Plugin> ImplClass::GetPluginRef() const
{
    if (state_ != ClassState::CLASS_STATE_REGISTERED) {
        return weak_ptr<Plugin>();
    }

    return pluginRef_;
}

const string &ImplClass::GetClassName() const
{
    return className_;
}

const string &ImplClass::GetPackageName() const
{
    if (state_ != ClassState::CLASS_STATE_REGISTERED) {
        HiLog::Error(LABEL, "get package name, className: %{public}s, state error: %{public}d.", className_.c_str(),
                     state_);
        return emptyString_;
    }

    auto sharedPlugin = pluginRef_.lock();
    if (sharedPlugin == nullptr) {
        HiLog::Error(LABEL, "get package name, failed to dereference Plugin, className: %{public}s.",
                     className_.c_str());
        return emptyString_;
    }

    return sharedPlugin->GetPackageName();
}

bool ImplClass::IsSupport(uint16_t interfaceID) const
{
    HiLog::Debug(LABEL, "search for support iid: %{public}u, className: %{public}s.", interfaceID, className_.c_str());
    for (uint32_t serviceFlag : services_) {
        if (MakeIID(serviceFlag) == interfaceID) {
            return true;
        }
    }

    HiLog::Debug(LABEL, "there is no matching interfaceID");
    return false;
}

void ImplClass::OnObjectDestroy()
{
    // this situation does not happen in design.
    // the process context can guarantee that this will not happen.
    // the judgment statement here is for protection and positioning purposes only.
    if (state_ != ClassState::CLASS_STATE_REGISTERED) {
        HiLog::Error(LABEL, "failed to destroy object because class unregistered, className: %{public}s.",
                     className_.c_str());
        return;
    }

    std::unique_lock<std::recursive_mutex> guard(dynDataLock_);
    // this situation does not happen in design.
    if (instanceNum_ == 0) {
        guard.unlock();
        HiLog::Error(LABEL, "destroy object while instanceNum is zero.");
        return;
    }

    --instanceNum_;

    auto sharedPlugin = pluginRef_.lock();
    // this situation does not happen in design.
    if (sharedPlugin == nullptr) {
        guard.unlock();
        HiLog::Error(LABEL, "destroy object failed because failed to dereference Plugin, className: %{public}s.",
                     className_.c_str());
        return;
    }

    HiLog::Debug(LABEL, "destroy object: className: %{public}s", className_.c_str());
    if (instanceNum_ == 0) {
        sharedPlugin->DeRef();
    }

    HiLog::Debug(LABEL, "destroy object success, InstanceNum: %{public}u.", instanceNum_);
}

const set<uint32_t> &ImplClass::GetServices() const
{
    return services_;
}

bool ImplClass::IsCompatible(const map<string, AttrData> &caps) const
{
    return capability_.IsCompatible(caps);
}

const AttrData *ImplClass::GetCapability(const string &key) const
{
    if (state_ != ClassState::CLASS_STATE_REGISTERED) {
        return nullptr;
    }

    return capability_.GetCapability(key);
}

const std::map<std::string, AttrData> &ImplClass::GetCapability() const
{
    return capability_.GetCapability();
}

// ------------------------------- private method -------------------------------
bool ImplClass::AnalysisServices(const json &classInfo)
{
    size_t serviceNum;
    if (JsonHelper::GetArraySize(classInfo, "services", serviceNum) != SUCCESS) {
        HiLog::Error(LABEL, "read array size of services failed.");
        return false;
    }
    HiLog::Debug(LABEL, "class service num: %{public}zu.", serviceNum);

    uint16_t interfaceID;
#ifndef PLUGIN_FLAG_RTTI_ENABLE
    uint32_t lastInterfaceID = UINT32_MAX_VALUE;
#endif
    uint16_t serviceType;
    uint32_t result;
    bool serviceAdded = false;
    const json &servicesInfo = classInfo["services"];
    for (size_t i = 0; i < serviceNum; i++) {
        const json &serviceInfo = servicesInfo[i];
        if (JsonHelper::GetUint16Value(serviceInfo, "interfaceID", interfaceID) != SUCCESS) {
            HiLog::Error(LABEL, "read interfaceID failed at %{public}zu.", i);
#ifndef PLUGIN_FLAG_RTTI_ENABLE
            // when -frtti is not enable, to ensure correct base class side-to-side conversion, we require that
            // the plugin class inherit only one service interface class and the PluginClassBase class,
            // while the location of the service interface class is in front of the PluginClassBase.
            // below, we check only one business interface class is allowed to inherit.
            HiLog::Error(LABEL, "no valid service info or encounter the risk of more than one business \
                                interface base class.");
            return false;
#else
            continue;
#endif
        }

#ifndef PLUGIN_FLAG_RTTI_ENABLE
        // check only one business interface class is allowed to inherit.
        if (lastInterfaceID != UINT32_MAX_VALUE && lastInterfaceID != interfaceID) {
            HiLog::Error(LABEL, "more than one business interface base class.");
            return false;
        }
        lastInterfaceID = interfaceID;
#endif
        result = JsonHelper::GetUint16Value(serviceInfo, "serviceType", serviceType);
        if (result != SUCCESS) {
            if (result != ERR_NO_TARGET) {
                HiLog::Error(LABEL, "read serviceType failed at %{public}zu.", i);
                continue;
            }
            // serviceType is optional, and default zero.
            serviceType = 0;
        }

        HiLog::Debug(LABEL, "insert class service iid: %{public}u, serviceType: %{public}u.", interfaceID, serviceType);
        services_.insert(MakeServiceFlag(interfaceID, serviceType));
        serviceAdded = true;
    }

    return serviceAdded;
}

bool ImplClass::AnalysisMaxInstance(const json &classInfo)
{
    uint32_t result = JsonHelper::GetUint16Value(classInfo, "maxInstance", maxInstance_);
    if (result == SUCCESS) {
        HiLog::Debug(LABEL, "class maxInstance num: %{public}u.", maxInstance_);
        if (maxInstance_ == 0) {
            HiLog::Error(LABEL, "class maxInstance num is invalid zero.");
            return false;
        }
        return true;
    }

    if (result != ERR_NO_TARGET) {
        HiLog::Error(LABEL, "read maxInstance failed.");
        return false;
    }

    // maxInstance is optional, and value for this case is not limited.
    maxInstance_ = INSTANCE_NO_LIMIT_NUM;
    return true;
}

PluginClassBase *ImplClass::DoCreateObject(shared_ptr<Plugin> &plugin)
{
    // since the plugin library may be unloaded and reloaded, the pointer cannot guarantee a constant value,
    // so it is reread every time here.
    PluginCreateFunc factory = plugin->GetCreateFunc();
    if (factory == nullptr) {
        HiLog::Error(LABEL, "failed to get create func, className: %{public}s.", className_.c_str());
        return nullptr;
    }

    PluginClassBase *pluginBaseObj = factory(className_);
    if (pluginBaseObj == nullptr) {
        HiLog::Error(LABEL, "create object result null, className: %{public}s.", className_.c_str());
        return nullptr;
    }

#ifndef PLUGIN_FLAG_RTTI_ENABLE
    // when -frtti is not enable, to ensure correct base class side-to-side conversion,
    // we require that the plugin class inherit only one service interface class and the PluginClassBase class,
    // while the location of the service interface class is in front of the PluginClassBase.
    // below, we check the inherited position constraint.
    void *obj = dynamic_cast<void *>(pluginBaseObj);  // adjust pointer position when multiple inheritance.
    if (obj == pluginBaseObj) {
        // PluginClassBase is the first base class, not allowed.
        HiLog::Error(LABEL, "service interface class is not the first base class. className: %{public}s.",
                     className_.c_str());
        delete pluginBaseObj;
        return nullptr;
    }
#endif

    if (pluginBaseObj->SetImplClassKey(selfKey_) != PluginClassBase::MAGIC_CODE) {
        HiLog::Error(LABEL, "failed to set key, className: %{public}s.", className_.c_str());
        delete pluginBaseObj;
        return nullptr;
    }

    return pluginBaseObj;
}
} // namespace MultimediaPlugin
} // namespace OHOS
