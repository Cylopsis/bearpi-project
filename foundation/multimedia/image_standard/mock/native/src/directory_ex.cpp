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


#include "directory_ex.h"
#include <dirent.h>
#include <iostream>
#include <sys/stat.h>
#include "hilog/log.h"
#include "log_tags.h"
#include "unistd.h"

using namespace std;

namespace OHOS {
using namespace OHOS::HiviewDFX;
static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "directory_ex_mock" };

string ExtractFileExt(const string& fileName)
{
    string::size_type pos = fileName.rfind(".");
    if (pos == string::npos) {
        return "";
    }
    return string(fileName).substr(pos + 1, fileName.size());
}

string TransformFileName(const string& fileName)
{
    string::size_type pos = fileName.find(".");
    if (pos == string::npos) {
        string transformfileName = fileName;

#ifdef _WIN32
        transformfileName = transformfileName.append(".dll");
#elif defined _APPLE
        transformfileName = transformfileName.append(".dylib");
#endif

        return transformfileName;
    } else {
        string transformfileName = string(fileName).substr(0, pos + 1);

#ifdef _WIN32
        transformfileName = transformfileName.append("dll");
#elif defined _APPLE
        transformfileName = transformfileName.append("dylib");
#endif

        return transformfileName;
    }
}

string IncludeTrailingPathDelimiter(const std::string& path)
{
    if (path.rfind("/") != path.size() - 1) {
        return path + "/";
    }
    return path;
}

void GetDirFiles(const string& path, vector<string>& files)
{
    struct stat info = { 0 };
    string pathStringWithDelimiter;
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        return;
    }

    while (true) {
        struct dirent *ptr = readdir(dir);
        if (ptr == nullptr) {
            break;
        }

        // current dir OR parrent dir
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        } else if (S_ISDIR(info.st_mode)) {
            pathStringWithDelimiter = IncludeTrailingPathDelimiter(path) + string(ptr->d_name);
            GetDirFiles(pathStringWithDelimiter, files);
        } else {
            files.push_back(IncludeTrailingPathDelimiter(path) + string(ptr->d_name));
        }
    }
    closedir(dir);
}

bool PathToRealPath(const string& path, string& realPath)
{
#if !defined(_WIN32) && !defined(_APPLE)
    if (path.empty()) {
        HiLog::Error(LABEL, "path is empty!");
        return false;
    }
#endif

    if ((path.length() >= PATH_MAX)) {
        HiLog::Error(LABEL, "path len is error, the len is: [%{public}zu]", path.length());
        return false;
    }

    char tmpPath[PATH_MAX] = {0};

#ifdef _WIN32
    if (_fullpath(tmpPath, path.c_str(), PATH_MAX) == NULL) {
        HiLog::Error(LABEL, "path to realpath error");
        return false;
    }
#else
    if (realpath(path.c_str(), tmpPath) == nullptr) {
        HiLog::Error(LABEL, "path to realpath error");
        return false;
    }
#endif

    realPath = tmpPath;
    if (access(realPath.c_str(), F_OK) != 0) {
        HiLog::Error(LABEL, "check realpath (%{public}s) error", realPath.c_str());
        return false;
    }
    return true;
}
} // namespace OHOS
