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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <iostream>
#include <sstream>
#include <string>
#include "nocopyable.h"
#include "plugin_errors.h"
#include "plugin_export.h"

namespace OHOS {
namespace MultimediaPlugin {
enum class PluginState : int32_t {
    PLUGIN_STATE_UNREGISTER = 0,
    PLUGIN_STATE_REGISTERED,
    PLUGIN_STATE_RESOLVED,
    PLUGIN_STATE_STARTING,
    PLUGIN_STATE_ACTIVE,
    PLUGIN_STATE_STOPPING
};

enum class VersionParseStep;
class ImplClassMgr;
class PlatformAdp;
struct VersionNum;

class Plugin final : public NoCopyable {
public:
    Plugin();
    ~Plugin();
    uint32_t Register(std::istream &metadata, std::string &&libraryPath, std::weak_ptr<Plugin> &plugin);
    uint32_t Ref();
    void DeRef();
    void Block();
    void Unblock();
    PluginCreateFunc GetCreateFunc();
    const std::string &GetLibraryPath() const;
    const std::string &GetPackageName() const;

private:
    static constexpr uint8_t VERSION_MAJOR_INDEX = 0;
    static constexpr uint8_t VERSION_MINOR_INDEX = 1;
    static constexpr uint8_t VERSION_MICRO_INDEX = 2;
    static constexpr uint8_t VERSION_NANO_INDEX = 3;
    static constexpr uint8_t VERSION_ARRAY_SIZE = 4;
    static constexpr uint8_t UINT16_MAX_DECIMAL_DIGITS = 5;  // uint16_t max number 65535, 5 digits.

    uint32_t ResolveLibrary();
    void FreeLibrary();
    uint32_t RegisterMetadata(std::istream &metadata, std::weak_ptr<Plugin> &plugin);
    uint32_t CheckTargetVersion(const std::string &targetVersion);
    uint32_t AnalyzeVersion(const std::string &versionInfo, VersionNum &versionNum);
    uint32_t ExecuteVersionAnalysis(const std::string &input, VersionParseStep &step,
                                    uint16_t (&versionNum)[VERSION_ARRAY_SIZE]);
    uint32_t GetUint16ValueFromDecimal(const std::string &source, uint16_t &result);

    PlatformAdp &platformAdp_;
    ImplClassMgr &implClassMgr_;
    // dynDataLock_:
    // for data that only changes in the register, we don't call it dynamic data.
    // non-dynamic data are protected by other means, that is: mutual exclusion between
    // the register and createObject processes.
    // current dynamic data includes:
    // state_, handle_, refNum_, startFunc_, stopFunc_, createFunc_, blocked_.
    std::recursive_mutex dynDataLock_;
    PluginState state_ = PluginState::PLUGIN_STATE_UNREGISTER;
    std::weak_ptr<Plugin> plugin_;
    void *handle_ = nullptr;
    uint32_t refNum_ = 0;
    std::string libraryPath_;
    std::string packageName_;
    std::string version_;
    PluginStartFunc startFunc_ = nullptr;
    PluginStopFunc stopFunc_ = nullptr;
    PluginCreateFunc createFunc_ = nullptr;
    bool blocked_ = false;
};
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // PLUGIN_H
