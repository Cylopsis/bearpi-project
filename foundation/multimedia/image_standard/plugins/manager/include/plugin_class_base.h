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

#ifndef PLUGIN_CLASS_BASE_H
#define PLUGIN_CLASS_BASE_H

#include <string>

namespace OHOS {
namespace MultimediaPlugin {
class AbsImplClassKey;

class PluginClassBase {
public:
    PluginClassBase() = default;
    virtual ~PluginClassBase();
    static constexpr uint32_t MAGIC_CODE = 0x1122CCFF;

private:
    friend class ImplClass;
    // the plugin manager guarantees that the key object continue to be valid until the plugin object is destroyed.
    // return MAGIC_CODE used to check if the plugin class correctly inherits the PluginClassBase class.
    uint32_t SetImplClassKey(AbsImplClassKey &key);
    AbsImplClassKey *implClassKey_ = nullptr;
};
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // PLUGIN_CLASS_BASE_H
