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

#include "plugin_export.h"
#include <map>
#include "hilog/log.h"
#include "log_tags.h"
#include "plugin_class_base.h"
#include "plugin_utils.h"
#include "cloud_label_detector.h"
#include "label_detector.h"

// this file shows how to write plugin_export.cpp file directly.
// but this file can also be simplified using the code elements provided by plugin_utils.h,
// see plugin_example2 and plugin_example3.
using std::map;
using std::string;
using namespace OHOS::HiviewDFX;

static const string PACKAGE_NAME = "plugin_example1";
static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "plugin_example1" };
using ImplClassMap = map<const string, PluginObjectCreatorFunc>;

static ImplClassMap implClassMap = {
    PLUGIN_EXPORT_REGISTER_CLASS(OHOS::PluginExample::LabelDetector)
    PLUGIN_EXPORT_REGISTER_CLASS(OHOS::PluginExample::CloudLabelDetector)
};

bool PluginExternalStart()
{
    HiLog::Debug(LABEL, "call PluginExternalStart() in package: %{public}s.", PACKAGE_NAME.c_str());
    // in this example we don't have to do anything,
    // but you may need to do some preparations below for your plugin...
    return true;
}

void PluginExternalStop()
{
    HiLog::Debug(LABEL, "call PluginExternalStop() in package: %{public}s.", PACKAGE_NAME.c_str());
    // in this example we don't have to do anything,
    // but you may need to do some cleaning work below for your plugin...
    return;
}

OHOS::MultimediaPlugin::PluginClassBase *PluginExternalCreate(const string &className)
{
    HiLog::Debug(LABEL, "PluginExternalCreate: create object for package: %{public}s, class: %{public}s.",
                 PACKAGE_NAME.c_str(), className.c_str());

    auto iter = implClassMap.find(className);
    if (iter == implClassMap.end()) {
        HiLog::Error(LABEL, "PluginExternalCreate: failed to find class: %{public}s, in package: %{public}s.",
                     className.c_str(), PACKAGE_NAME.c_str());
        return nullptr;
    }

    auto creator = iter->second;
    if (creator == nullptr) {
        HiLog::Error(LABEL, "PluginExternalCreate: null creator for class: %{public}s, in package: %{public}s.",
                     className.c_str(), PACKAGE_NAME.c_str());
        return nullptr;
    }

    return creator();
}
