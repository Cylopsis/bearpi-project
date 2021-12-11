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

#ifndef PLATFORM_ADP_H
#define PLATFORM_ADP_H

#include <string>
#include "nocopyable.h"
#include "singleton.h"
#include "plugin_errors.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace OHOS {
namespace MultimediaPlugin {
class PlatformAdp final : public NoCopyable {
public:
#ifdef _WIN32
    HMODULE AdpLoadLibrary(const std::string &packageName);
    void AdpFreeLibrary(HMODULE handle);
    FARPROC AdpGetSymAddress(HMODULE handle, const std::string &symbol);
#else
    void *LoadLibrary(const std::string &packageName);
    void FreeLibrary(void *handle);
    void *GetSymAddress(void *handle, const std::string &symbol);
#endif

    uint32_t CheckAndNormalizePath(std::string &path);
    DECLARE_DELAYED_REF_SINGLETON(PlatformAdp);

public:
    static const std::string DIR_SEPARATOR;
    static const std::string LIBRARY_FILE_SUFFIX;
};
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // PLATFORM_ADP_H
