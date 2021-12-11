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

#ifndef PLUGIN_UTILS_H
#define PLUGIN_UTILS_H

#include <map>
#include <string>
#include "plugin_class_base.h"

namespace OHOS {
namespace MultimediaPlugin {
template<class ImplClassType>
PluginClassBase *CreatePluginObject()
{
    return static_cast<PluginClassBase *>(new (std::nothrow) ImplClassType());  // upward conversion.
}
} // namespace MultimediaPlugin
} // namespace OHOS

#define PLUGIN_OBJECT_CREATOR(ImplClassType) OHOS::MultimediaPlugin::CreatePluginObject<ImplClassType>

#define IMPL_CLASS_NAME_STRING(ImplClassType) (#ImplClassType)

using PluginObjectCreatorFunc = OHOS::MultimediaPlugin::PluginClassBase *(*)();

// --------- a set of code fragments that helps define a simple plugin_export.cpp file ----------
#define PLUGIN_EXPORT_REGISTER_PACKAGE(packageName) \
static const std::string PACKAGE_NAME = (packageName);

#define PLUGIN_EXPORT_REGISTER_CLASS_BEGIN \
using ImplClassMap = std::map<const std::string, PluginObjectCreatorFunc>; \
static ImplClassMap implClassMap = {

#define PLUGIN_EXPORT_REGISTER_CLASS(ImplClassType) \
{ IMPL_CLASS_NAME_STRING(ImplClassType), PLUGIN_OBJECT_CREATOR(ImplClassType) },

#define PLUGIN_EXPORT_REGISTER_CLASS_END \
};

#define PLUGIN_EXPORT_DEFAULT_EXTERNAL_START() \
bool PluginExternalStart() \
{                                                               \
    PLUGIN_LOG_D("call PluginExternalStart() in package: %{public}s.", PACKAGE_NAME.c_str()); \
    return true;                                             \
}

#define PLUGIN_EXPORT_DEFAULT_EXTERNAL_STOP() \
void PluginExternalStop() \
{                                                              \
    PLUGIN_LOG_D("call PluginExternalStop() in package: %{public}s.", PACKAGE_NAME.c_str()); \
    return;                                                    \
}
#endif // PLUGIN_UTILS_H
