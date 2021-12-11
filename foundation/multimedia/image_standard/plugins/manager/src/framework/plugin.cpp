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

#include "plugin.h"
#include <utility>
#include "hilog/log.h"
#include "impl_class_mgr.h"
#include "json.hpp"
#include "json_helper.h"
#include "log_tags.h"
#include "platform_adp.h"
#include "singleton.h"
#ifdef _WIN32
#include <windows.h>
HMODULE hDll = NULL;
#endif

namespace OHOS {
namespace MultimediaPlugin {
using nlohmann::json;
using std::istream;
using std::istringstream;
using std::recursive_mutex;
using std::size_t;
using std::string;
using std::weak_ptr;
using namespace OHOS::HiviewDFX;

enum class VersionParseStep : int32_t { STEP_MAJOR = 0, STEP_MINOR, STEP_MICRO, STEP_NANO, STEP_FINISHED };

struct VersionNum {
    uint16_t major = 0;
    uint16_t minor = 0;
    uint16_t micro = 0;
    uint16_t nano = 0;
};

static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "Plugin" };

Plugin::Plugin()
    : platformAdp_(DelayedRefSingleton<PlatformAdp>::GetInstance()),
      implClassMgr_(DelayedRefSingleton<ImplClassMgr>::GetInstance()) {}

Plugin::~Plugin()
{
    std::unique_lock<std::recursive_mutex> guard(dynDataLock_);
    if (refNum_ != 0) {
        // this situation does not happen in design.
        // the process context can guarantee that this will not happen.
        // the judgment statement here is for protection and positioning purposes only.
        HiLog::Error(LABEL, "release plugin: refNum: %{public}u.", refNum_);
    }

    implClassMgr_.DeleteClass(plugin_);
    FreeLibrary();
}

uint32_t Plugin::Register(istream &metadata, string &&libraryPath, weak_ptr<Plugin> &plugin)
{
    std::unique_lock<std::recursive_mutex> guard(dynDataLock_);
    if (state_ != PluginState::PLUGIN_STATE_UNREGISTER) {
        guard.unlock();
        HiLog::Error(LABEL, "repeat registration.");
        return ERR_INTERNAL;
    }

    auto ret = RegisterMetadata(metadata, plugin);
    if (ret != SUCCESS) {
        guard.unlock();
        HiLog::Error(LABEL, "failed to register metadata, ERRNO: %{public}u.", ret);
        return ret;
    }

    libraryPath_ = std::move(libraryPath);
    plugin_ = plugin;
    state_ = PluginState::PLUGIN_STATE_REGISTERED;
    return SUCCESS;
}

uint32_t Plugin::Ref()
{
    // once the client make a ref, it can use the plugin at any time,
    // so we do the necessary preparations here.
    std::unique_lock<std::recursive_mutex> guard(dynDataLock_);
    if (state_ == PluginState::PLUGIN_STATE_REGISTERED) {
        if (ResolveLibrary() != SUCCESS) {
            guard.unlock();
            HiLog::Error(LABEL, "failed to resolve library.");
            return ERR_GENERAL;
        }
        state_ = PluginState::PLUGIN_STATE_RESOLVED;
    }

    if (state_ == PluginState::PLUGIN_STATE_RESOLVED) {
        // maybe asynchronous, or for reduce the locking time
        state_ = PluginState::PLUGIN_STATE_STARTING;
        if (!startFunc_()) {
            HiLog::Error(LABEL, "failed to start plugin.");
            FreeLibrary();
            state_ = PluginState::PLUGIN_STATE_REGISTERED;
            return ERR_GENERAL;
        }
        state_ = PluginState::PLUGIN_STATE_ACTIVE;
    }

    if (state_ != PluginState::PLUGIN_STATE_ACTIVE) {
        HiLog::Error(LABEL, "plugin ref: state error, state: %{public}d.", state_);
        return ERR_GENERAL;
    }

    ++refNum_;
    HiLog::Debug(LABEL, "plugin refNum: %{public}d.", refNum_);
    return SUCCESS;
}

void Plugin::DeRef()
{
    std::unique_lock<std::recursive_mutex> guard(dynDataLock_);
    if (refNum_ == 0) {
        // this situation does not happen in design.
        // the process context can guarantee that this will not happen.
        // the judgment statement here is for protection and positioning purposes only.
        guard.unlock();
        HiLog::Error(LABEL, "DeRef while RefNum is zero.");
        return;
    }

    --refNum_;
    HiLog::Debug(LABEL, "plugin refNum: %{public}d.", refNum_);
}

void Plugin::Block()
{
    // used to protect against business interruptions during plugin upgrades.
    // after the plugin is upgraded, if the original .so is being used,
    // it cannot be released immediately and should be locked,
    // and the subsequent requests are migrated to the new .so.
    std::unique_lock<std::recursive_mutex> guard(dynDataLock_);
    blocked_ = true;
}

void Plugin::Unblock()
{
    std::unique_lock<std::recursive_mutex> guard(dynDataLock_);
    blocked_ = false;
}

PluginCreateFunc Plugin::GetCreateFunc()
{
    std::unique_lock<std::recursive_mutex> guard(dynDataLock_);
    if ((state_ != PluginState::PLUGIN_STATE_ACTIVE) || (refNum_ == 0)) {
        // In this case, we can't guarantee that the pointer is lasting valid.
        HiLog::Error(LABEL, "failed to get create func, State: %{public}d, RefNum: %{public}u.", state_, refNum_);
        return nullptr;
    }

    return createFunc_;
}

const string &Plugin::GetLibraryPath() const
{
    return libraryPath_;
}

const string &Plugin::GetPackageName() const
{
    return packageName_;
}

// ------------------------------- private method -------------------------------
uint32_t Plugin::ResolveLibrary()
{
    std::string pluginStartSymbol = "PluginExternalStart";
    std::string pluginStopSymbol = "PluginExternalStop";
    std::string pluginCreateSymbol = "PluginExternalCreate";

#ifdef _WIN32
    hDll = platformAdp_.AdpLoadLibrary(libraryPath_);
    if (hDll == NULL) {
        HiLog::Error(LABEL, "failed to load library.");
        return ERR_GENERAL;
    }

    startFunc_ = (PluginStartFunc)platformAdp_.AdpGetSymAddress(hDll, pluginStartSymbol);
    stopFunc_ = (PluginStopFunc)platformAdp_.AdpGetSymAddress(hDll, pluginStopSymbol);
    createFunc_ = (PluginCreateFunc)platformAdp_.AdpGetSymAddress(hDll, pluginCreateSymbol);
    if (startFunc_ == NULL || stopFunc_ == NULL || createFunc_ == NULL) {
        HiLog::Error(LABEL, "failed to get export symbol for the plugin.");
        FreeLibrary();
        return ERR_GENERAL;
    }

    return SUCCESS;
#else
    handle_ = platformAdp_.LoadLibrary(libraryPath_);
    if (handle_ == nullptr) {
        HiLog::Error(LABEL, "failed to load library.");
        return ERR_GENERAL;
    }

    startFunc_ = (PluginStartFunc)platformAdp_.GetSymAddress(handle_, pluginStartSymbol);
    stopFunc_ = (PluginStopFunc)platformAdp_.GetSymAddress(handle_, pluginStopSymbol);
    createFunc_ = (PluginCreateFunc)platformAdp_.GetSymAddress(handle_, pluginCreateSymbol);
    if (startFunc_ == nullptr || stopFunc_ == nullptr || createFunc_ == nullptr) {
        HiLog::Error(LABEL, "failed to get export symbol for the plugin.");
        FreeLibrary();
        return ERR_GENERAL;
    }

    return SUCCESS;
#endif
}

void Plugin::FreeLibrary()
{
#ifdef _WIN32
    if (state_ == PluginState::PLUGIN_STATE_STARTING || state_ == PluginState::PLUGIN_STATE_ACTIVE) {
        if (stopFunc_ != NULL) {
            stopFunc_();
        }
    }
    if (handle_ == NULL) {
        return;
    }
    platformAdp_.AdpFreeLibrary(hDll);
    hDll = NULL;
    startFunc_ = NULL;
    stopFunc_ = NULL;
    createFunc_ = NULL;
#else
    if (state_ == PluginState::PLUGIN_STATE_STARTING || state_ == PluginState::PLUGIN_STATE_ACTIVE) {
        if (stopFunc_ != nullptr) {
            stopFunc_();
        }
    }

    if (handle_ == nullptr) {
        return;
    }

    platformAdp_.FreeLibrary(handle_);
    handle_ = nullptr;
    startFunc_ = nullptr;
    stopFunc_ = nullptr;
    createFunc_ = nullptr;
#endif
}

uint32_t Plugin::RegisterMetadata(istream &metadata, weak_ptr<Plugin> &plugin)
{
    json root;
    metadata >> root;
    if (JsonHelper::GetStringValue(root, "packageName", packageName_) != SUCCESS) {
        HiLog::Error(LABEL, "read packageName failed.");
        return ERR_INVALID_PARAMETER;
    }

    string targetVersion;
    if (JsonHelper::GetStringValue(root, "targetVersion", targetVersion) != SUCCESS) {
        HiLog::Error(LABEL, "read targetVersion failed.");
        return ERR_INVALID_PARAMETER;
    }
    uint32_t ret = CheckTargetVersion(targetVersion);
    if (ret != SUCCESS) {
        // target version is not compatible
        HiLog::Error(LABEL, "check targetVersion failed, Version: %{public}s, ERRNO: %{public}u.",
                     targetVersion.c_str(), ret);
        return ret;
    }

    if (JsonHelper::GetStringValue(root, "version", version_) != SUCCESS) {
        HiLog::Error(LABEL, "read version failed.");
        return ERR_INVALID_PARAMETER;
    }
    VersionNum versionNum;
    ret = AnalyzeVersion(version_, versionNum);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "check version failed, Version: %{public}s, ERRNO: %{public}u.", version_.c_str(), ret);
        return ret;
    }

    size_t classNum;
    if (JsonHelper::GetArraySize(root, "classes", classNum) != SUCCESS) {
        HiLog::Error(LABEL, "get array size of classes failed.");
        return ERR_INVALID_PARAMETER;
    }
    HiLog::Debug(LABEL, "parse class num: %{public}zu.", classNum);
    for (size_t i = 0; i < classNum; i++) {
        const json &classInfo = root["classes"][i];
        if (implClassMgr_.AddClass(plugin, classInfo) != SUCCESS) {
            HiLog::Error(LABEL, "failed to add class, index: %{public}zu.", i);
            continue;
        }
    }

    return SUCCESS;
}

uint32_t Plugin::CheckTargetVersion(const string &targetVersion)
{
    VersionNum versionNum;
    auto ret = AnalyzeVersion(targetVersion, versionNum);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "failed to analyze version, ERRNO: %{public}u.", ret);
        return ret;
    }

    return SUCCESS;
}

uint32_t Plugin::AnalyzeVersion(const string &versionInfo, VersionNum &versionNum)
{
    VersionParseStep step = VersionParseStep::STEP_MAJOR;
    istringstream versionInput(versionInfo);
    uint16_t versionArray[VERSION_ARRAY_SIZE] = { 0 };  // major, minor, micro, nano.
    string tmp;

    while (getline(versionInput, tmp, '.')) {
        auto ret = ExecuteVersionAnalysis(tmp, step, versionArray);
        if (ret != SUCCESS) {
            HiLog::Error(LABEL, "failed to execute version analysis, ERRNO: %{public}u.", ret);
            return ret;
        }
    }

    if (step == VersionParseStep::STEP_NANO) {
        // we treat nano version as optional, and default 0.
        HiLog::Debug(LABEL, "default nano version 0.");
        versionArray[VERSION_NANO_INDEX] = 0;
        step = VersionParseStep::STEP_FINISHED;
    }

    if (step != VersionParseStep::STEP_FINISHED) {
        HiLog::Error(LABEL, "analysis version failed, step = %{public}d.", step);
        return ERR_INVALID_PARAMETER;
    }

    versionNum.major = versionArray[VERSION_MAJOR_INDEX];
    versionNum.minor = versionArray[VERSION_MINOR_INDEX];
    versionNum.micro = versionArray[VERSION_MICRO_INDEX];
    versionNum.nano = versionArray[VERSION_NANO_INDEX];

    HiLog::Debug(LABEL, "analysis result: %{public}u.%{public}u.%{public}u.%{public}u.", versionNum.major,
                 versionNum.minor, versionNum.micro, versionNum.nano);

    return SUCCESS;
}

uint32_t Plugin::ExecuteVersionAnalysis(const string &input, VersionParseStep &step,
                                        uint16_t (&versionNum)[VERSION_ARRAY_SIZE])
{
    switch (step) {
        case VersionParseStep::STEP_MAJOR: {
            auto ret = GetUint16ValueFromDecimal(input, versionNum[VERSION_MAJOR_INDEX]);
            if (ret != SUCCESS) {
                HiLog::Error(LABEL, "read major version failed, input: %{public}s, ERRNO: %{public}u.", input.c_str(),
                             ret);
                return ret;
            }
            step = VersionParseStep::STEP_MINOR;
            break;
        }
        case VersionParseStep::STEP_MINOR: {
            auto ret = GetUint16ValueFromDecimal(input, versionNum[VERSION_MINOR_INDEX]);
            if (ret != SUCCESS) {
                HiLog::Error(LABEL, "read minor version failed, input: %{public}s, ERRNO: %{public}u.", input.c_str(),
                             ret);
                return ret;
            }
            step = VersionParseStep::STEP_MICRO;
            break;
        }
        case VersionParseStep::STEP_MICRO: {
            auto ret = GetUint16ValueFromDecimal(input, versionNum[VERSION_MICRO_INDEX]);
            if (ret != SUCCESS) {
                HiLog::Error(LABEL, "read micro version failed, input: %{public}s, ERRNO: %{public}u.", input.c_str(),
                             ret);
                return ret;
            }
            step = VersionParseStep::STEP_NANO;
            break;
        }
        case VersionParseStep::STEP_NANO: {
            auto ret = GetUint16ValueFromDecimal(input, versionNum[VERSION_NANO_INDEX]);
            if (ret != SUCCESS) {
                HiLog::Error(LABEL, "read nano version failed, input: %{public}s, ERRNO: %{public}u.", input.c_str(),
                             ret);
                return ret;
            }
            step = VersionParseStep::STEP_FINISHED;
            break;
        }
        default: {
            HiLog::Error(LABEL, "read redundant version data, input: %{public}s.", input.c_str());
            return ERR_INVALID_PARAMETER;
        }
    }

    return SUCCESS;
}

uint32_t Plugin::GetUint16ValueFromDecimal(const string &source, uint16_t &result)
{
    if (source.empty() || source.size() > UINT16_MAX_DECIMAL_DIGITS) {
        HiLog::Error(LABEL, "invalid string of uint16: %{public}s.", source.c_str());
        return ERR_INVALID_PARAMETER;
    }

    // determine if all characters are numbers.
    for (auto &character : source) {
        if (character < '0' || character > '9') {
            HiLog::Error(LABEL, "character out of the range of digital: %{public}s.", source.c_str());
            return ERR_INVALID_PARAMETER;
        }
    }

    unsigned long tmp = stoul(source);
    if (tmp > UINT16_MAX_VALUE) {
        HiLog::Error(LABEL, "result out of the range of uint16: %{public}s.", source.c_str());
        return ERR_INVALID_PARAMETER;
    }

    result = static_cast<uint16_t>(tmp);
    return SUCCESS;
}
} // namespace MultimediaPlugin
} // namespace OHOS
