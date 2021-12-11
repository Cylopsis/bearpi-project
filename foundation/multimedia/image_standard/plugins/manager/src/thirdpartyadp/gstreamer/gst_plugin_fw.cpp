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

#include "gst_plugin_fw.h"
#include "hilog/log.h"
#include "log_tags.h"

namespace OHOS {
namespace MultimediaPlugin {
using std::map;
using std::mutex;
using std::string;
using std::vector;
using namespace OHOS::HiviewDFX;

static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "GstPluginFw" };

uint32_t GstPluginFw::Register(const vector<string> &canonicalPaths)
{
    (void) canonicalPaths;
    HiLog::Debug(LABEL, "register called.");
    return SUCCESS;
}

PluginClassBase *GstPluginFw::CreateObject(uint16_t interfaceID, const string &className, uint32_t &errorCode)
{
    (void) interfaceID;
    (void) className;
    HiLog::Debug(LABEL, "CreateObject by name called.");
    errorCode = ERR_MATCHING_PLUGIN;

    return nullptr;
}

PluginClassBase *GstPluginFw::CreateObject(uint16_t interfaceID, uint16_t serviceType,
                                           const map<string, AttrData> &capabilities,
                                           const PriorityScheme &priorityScheme, uint32_t &errorCode)
{
    (void) interfaceID;
    (void) serviceType;
    (void) capabilities;
    (void) priorityScheme;
    HiLog::Debug(LABEL, "CreateObject by serviceType called.");
    errorCode = ERR_MATCHING_PLUGIN;

    return nullptr;
}

uint32_t GstPluginFw::GstPluginFwGetClassInfo(uint16_t interfaceID, uint16_t serviceType,
                                              const map<std::string, AttrData> &capabilities,
                                              vector<ClassInfo> &classesInfo)
{
    (void) interfaceID;
    (void) serviceType;
    (void) capabilities;
    (void) classesInfo;
    HiLog::Debug(LABEL, "GetClassInfo by serviceType called.");
    return ERR_MATCHING_PLUGIN;
}

// ------------------------------- private method -------------------------------
GstPluginFw::GstPluginFw() {}
GstPluginFw::~GstPluginFw() {}
} // namespace MultimediaPlugin
} // namespace OHOS
