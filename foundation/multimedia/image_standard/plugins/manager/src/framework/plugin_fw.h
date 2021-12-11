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

#ifndef PLUGIN_FW_H
#define PLUGIN_FW_H

#include <map>
#include <string>
#include <vector>
#include "nocopyable.h"
#include "singleton.h"
#include "attr_data.h"
#include "plugin_class_base.h"
#include "plugin_common_type.h"
#include "plugin_errors.h"
#include "priority_scheme.h"

namespace OHOS {
namespace MultimediaPlugin {
class PluginMgr;
class ImplClassMgr;

class PluginFw final : public NoCopyable {
public:
    uint32_t Register(const std::vector<std::string> &canonicalPaths);
    PluginClassBase *CreateObject(uint16_t interfaceID, const std::string &className, uint32_t &errorCode);
    PluginClassBase *CreateObject(uint16_t interfaceID, uint16_t serviceType,
                                  const std::map<std::string, AttrData> &capabilities,
                                  const PriorityScheme &priorityScheme, uint32_t &errorCode);
    uint32_t PluginFwGetClassInfo(uint16_t interfaceID, uint16_t serviceType,
                          const std::map<std::string, AttrData> &capabilities,
                          std::vector<ClassInfo> &classesInfo);
    DECLARE_DELAYED_REF_SINGLETON(PluginFw);

private:
    PluginMgr &pluginMgr_;
    ImplClassMgr &implClassMgr_;
};
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // PLUGIN_FW_H
