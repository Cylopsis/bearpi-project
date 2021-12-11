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

#include "platform_adp.h"
#ifndef _WIN32
#include <dlfcn.h>
#endif
#include "directory_ex.h"
#include "hilog/log.h"
#include "log_tags.h"

namespace OHOS {
namespace MultimediaPlugin {
using std::string;
using namespace OHOS::HiviewDFX;

static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "PlatformAdp" };

const string PlatformAdp::DIR_SEPARATOR = "/";
const string PlatformAdp::LIBRARY_FILE_SUFFIX = "so";

#ifdef _WIN32
HMODULE PlatformAdp::AdpLoadLibrary(const string &packageName)
{
    return LoadLibrary(packageName.c_str());
}

void PlatformAdp::AdpFreeLibrary(HMODULE handle)
{
    FreeLibrary(handle);
}

FARPROC PlatformAdp::AdpGetSymAddress(HMODULE handle, const string &symbol)
{
    return GetProcAddress(handle, symbol.c_str());
}
#else
void *PlatformAdp::LoadLibrary(const string &packageName)
{
    return dlopen(packageName.c_str(), RTLD_LAZY);
}

void PlatformAdp::FreeLibrary(void *handle)
{
    dlclose(handle);
}

void *PlatformAdp::GetSymAddress(void *handle, const string &symbol)
{
    return dlsym(handle, symbol.c_str());
}
#endif

uint32_t PlatformAdp::CheckAndNormalizePath(string &path)
{
#if !defined(_WIN32) && !defined(_APPLE)
    if (path.empty()) {
        HiLog::Error(LABEL, "check path empty.");
        return ERR_GENERAL;
    }
#endif

    string realPath;
    if (!PathToRealPath(path, realPath)) {
        HiLog::Error(LABEL, "path to real path error.");
        return ERR_GENERAL;
    }

    path = std::move(realPath);

    return SUCCESS;
}

// ------------------------------- private method -------------------------------
PlatformAdp::PlatformAdp() {}

PlatformAdp::~PlatformAdp() {}
} // namespace MultimediaPlugin
} // namespace OHOS
