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

#ifndef PLUGIN_MGR_H
#define PLUGIN_MGR_H

#include <string>
#include <vector>
#include "nocopyable.h"
#include "singleton.h"
#include "plugin_errors.h"
#include "pointer_key_map.h"

namespace OHOS {
namespace MultimediaPlugin {
class PlatformAdp;
class Plugin;

class PluginMgr final : public NoCopyable {
public:
    uint32_t Register(const std::vector<std::string> &canonicalPaths);
    DECLARE_DELAYED_REF_SINGLETON(PluginMgr);

private:
    uint32_t TraverseFiles(const std::string &canonicalPath);
    bool CheckPluginMetaFile(const std::string &candidateFile, std::string &libraryPath);
    uint32_t RegisterPlugin(const std::string &metadataPath, std::string &&libraryPath);

    static PlatformAdp &platformAdp_;
    using PluginMap = PointerKeyMap<const std::string, std::shared_ptr<Plugin>>;
    PluginMap plugins_;
};
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // PLUGIN_MGR_H
