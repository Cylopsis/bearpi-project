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
#include "hilog/log.h"
#include "log_tags.h"
#include "plugin_utils.h"
#include "cloud_label_detector3.h"
#include "label_detector3.h"

// this file shows how to easily write the plugin_export.cpp file using the code elements provided by plugin_utils.h.
// but sometimes you may need to write this file directly, see plugin_example1.
PLUGIN_EXPORT_REGISTER_PACKAGE("plugin_example3")

// register implement classes of this plugin.
PLUGIN_EXPORT_REGISTER_CLASS_BEGIN
PLUGIN_EXPORT_REGISTER_CLASS(OHOS::PluginExample::LabelDetector3)
PLUGIN_EXPORT_REGISTER_CLASS(OHOS::PluginExample::CloudLabelDetector3)
PLUGIN_EXPORT_REGISTER_CLASS_END

using namespace OHOS::HiviewDFX;

static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "plugin_example3" };

#define PLUGIN_LOG_D(...) HiLog::Debug(LABEL, __VA_ARGS__);
#define PLUGIN_LOG_E(...) HiLog::Error(LABEL, __VA_ARGS__);

// define the external interface of this plugin
PLUGIN_EXPORT_DEFAULT_EXTERNAL_START()
PLUGIN_EXPORT_DEFAULT_EXTERNAL_STOP()
// PLUGIN_EXPORT_DEFAULT_EXTERNAL_CREATE()

