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

#ifndef PLUGIN_EXPORT_H
#define PLUGIN_EXPORT_H

#include <string>
#include "plugin_class_base.h"
#include "image_type.h"

// The .so of plugin exports C form interface, so we use extern "C" here for consistent
#ifdef __cplusplus
extern "C" {
#endif

NATIVEEXPORT bool PluginExternalStart();
NATIVEEXPORT void PluginExternalStop();
NATIVEEXPORT OHOS::MultimediaPlugin::PluginClassBase *PluginExternalCreate(const std::string &className);

// function pointer for plugin interface "PluginExternalStart"
typedef decltype(PluginExternalStart) *PluginStartFunc;

// function pointer for plugin interface "PluginExternalStop"
typedef decltype(PluginExternalStop) *PluginStopFunc;

// function pointer for plugin interface "PluginExternalCreate"
typedef decltype(PluginExternalCreate) *PluginCreateFunc;

#ifdef __cplusplus
}
#endif

#endif // PLUGIN_EXPORT_H
