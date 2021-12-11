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

#ifndef MEDIA_FILE_UTILS_H
#define MEDIA_FILE_UTILS_H

#include <string>
#include <fstream>
#include <ftw.h>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <securec.h>

namespace OHOS {
namespace Media {
const std::string CHOWN_OWNER_NAME = "media_rw";
const std::string CHOWN_GROUP_NAME = "media_rw";
const mode_t CHOWN_RWX_USR_GRP = 0770;
const mode_t CHOWN_RW_USR_GRP = 0660;

/**
 * @brief Utility class for file operations
 *
 * @since 1.0
 * @version 1.0
 */
class MediaFileUtils {
public:
    static bool IsFileExists(const std::string& fileName);
    static bool CreateFile(const std::string& fileName);
    static bool DeleteFile(const std::string& fileName);
    static bool DeleteDir(const std::string& dirName);
    static std::string GetFilename(const std::string& filePath);
    static bool IsDirectory(const std::string& dirName);
    static bool MoveFile(const std::string& oldPath, const std::string& newPath);
    static bool CopyFile(const std::string& filePath, const std::string& newPath);
    static bool RenameDir(const std::string& oldPath, const std::string& newPath);
    static bool CreateDirectory(const std::string& dirPath);
};
} // namespace Media
} // namespace  OHOS
#endif  // MEDIA_FILE_UTILS_H
