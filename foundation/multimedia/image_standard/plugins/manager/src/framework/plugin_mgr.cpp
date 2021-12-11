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

#include "plugin_mgr.h"
#include <fstream>
#include "directory_ex.h"
#include "hilog/log.h"
#include "json.hpp"
#include "json_helper.h"
#include "log_tags.h"
#include "platform_adp.h"
#include "plugin.h"

namespace OHOS {
namespace MultimediaPlugin {
using nlohmann::json;
using std::ifstream;
using std::size_t;
using std::string;
using std::vector;
using std::weak_ptr;
using namespace OHOS::HiviewDFX;

static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "PluginMgr" };
PlatformAdp &PluginMgr::platformAdp_ = DelayedRefSingleton<PlatformAdp>::GetInstance();

uint32_t PluginMgr::Register(const vector<string> &canonicalPaths)
{
    bool pathTraversed = false;
    uint32_t errorCode = SUCCESS;
    for (const string &path : canonicalPaths) {
        uint32_t result = TraverseFiles(path);
        if (result == SUCCESS) {
            pathTraversed = true;
        } else {
            // no target is not a critical error type, giving priority to more serious errors.
            if ((errorCode == SUCCESS) || (errorCode == ERR_NO_TARGET)) {
                errorCode = result;
            }
        }
    }

    if (!pathTraversed) {
        return errorCode;
    }

    return SUCCESS;
}

// ------------------------------- private method -------------------------------
PluginMgr::PluginMgr()
{}

PluginMgr::~PluginMgr()
{}

uint32_t PluginMgr::TraverseFiles(const string &canonicalPath)
{
    bool noTarget = true;
    vector<string> strFiles;
    GetDirFiles(canonicalPath, strFiles);
    if (strFiles.empty()) {
        HiLog::Error(LABEL, "failed to get dir files.");
        return ERR_GENERAL;
    }

    string libraryPath;
    for (const auto &file : strFiles) {
        if (!CheckPluginMetaFile(file, libraryPath)) {
            continue;
        }
        noTarget = false;
        RegisterPlugin(file, std::move(libraryPath));
    }

    if (noTarget) {
        HiLog::Warn(LABEL, "there is no plugin meta file in path.");
        return ERR_NO_TARGET;
    }

    return SUCCESS;
}

bool PluginMgr::CheckPluginMetaFile(const string &candidateFile, string &libraryPath)
{
    const string meatedataFileSuffix = "pluginmeta";
    const string dirSeparator = "/";

#ifdef _WIN32
    const string libraryFileSuffix = "dll";
#elif defined _APPLE
    const string libraryFileSuffix = "dylib";
#else
    const string libraryFileSuffix = "so";
#endif

    string fileExt = ExtractFileExt(candidateFile);
    if (fileExt != meatedataFileSuffix) {
        // not a plugin metadata file, quietly skip this item.
        return false;
    }

    ifstream metadata(candidateFile);
    if (!metadata) {
        HiLog::Error(LABEL, "failed to open metadata file.");
        return false;
    }

    json root;
    metadata >> root;
    if (JsonHelper::GetStringValue(root, "libraryPath", libraryPath) != SUCCESS) {
        HiLog::Error(LABEL, "read libraryPath failed.");
        return false;
    }

#if defined(_WIN32) || defined(_APPLE)
    libraryPath = TransformFileName(libraryPath);
#endif

    fileExt = ExtractFileExt(libraryPath);
    if (fileExt != libraryFileSuffix) {
        HiLog::Error(LABEL, "invalid library suffix.");
        return false;
    }

#if !defined(_WIN32) && !defined(_APPLE)
    if (libraryPath.substr(0, 1) != dirSeparator) {
        // relative path to absolute path.
        // just keep original library name
        return true;
    }
#endif

    string realPath;
    if (!PathToRealPath(libraryPath, realPath)) {
        HiLog::Error(LABEL, "library path to real path error.");
        return false;
    }

    libraryPath = std::move(realPath);
    return true;
}

uint32_t PluginMgr::RegisterPlugin(const string &metadataPath, string &&libraryPath)
{
    auto iter = plugins_.find(&libraryPath);
    if (iter != plugins_.end()) {
        // already registered before, just skip it.
        HiLog::Debug(LABEL, "the libraryPath has already been registered before.");
        return ERR_GENERAL;
    }

    ifstream metadata(metadataPath);
    if (!metadata) {
        HiLog::Error(LABEL, "failed to open metadata file.");
        return ERR_GENERAL;
    }

    auto plugin = std::make_shared<Plugin>();
    if (plugin == nullptr) {
        HiLog::Error(LABEL, "failed to create Plugin.");
        return ERR_INTERNAL;
    }

    weak_ptr<Plugin> weakPtr = plugin;
    auto regRet = plugin->Register(metadata, std::move(libraryPath), weakPtr);
    if (regRet != SUCCESS) {
        HiLog::Error(LABEL, "failed to register plugin,ERRNO: %{public}u.", regRet);
        return regRet;
    }

    const std::string &key = plugin->GetLibraryPath();
    if (key.empty()) {
        HiLog::Error(LABEL, "get empty libraryPath.");
        return ERR_INTERNAL;
    }

    auto insertRet = plugins_.insert(PluginMap::value_type(&key, std::move(plugin)));
    if (!insertRet.second) {
        HiLog::Error(LABEL, "failed to insert Plugin");
        return ERR_INTERNAL;
    }

    return SUCCESS;
}
} // namespace MultimediaPlugin
} // namespace OHOS
