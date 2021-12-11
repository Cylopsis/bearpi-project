/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "zip_utils.h"

#include <zlib.h>
#include "securec.h"
#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "ZipUtils"};
}
using namespace std;
int32_t ZipUtils::CompressString(const string& inputStr, string& outputStr)
{
    z_stream zs;
    memset_s(&zs, sizeof zs, 0, sizeof zs);
    if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: deflateInit failed while compressing!", __func__);
        return ERROR;
    }
    zs.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(inputStr.data()));
    zs.avail_in = inputStr.size();

    int ret;
    char outBuffer[BUFFER_SIZE];
    string compressedStr;
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outBuffer);
        zs.avail_out = sizeof(outBuffer);
        ret = deflate(&zs, Z_FINISH);
        if (compressedStr.size() < zs.total_out) {
            compressedStr.append(outBuffer, zs.total_out - compressedStr.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);
    if (ret != Z_STREAM_END) {
        PERMISSION_LOG_ERROR(LABEL,
            "%{public}s: Exception during zlib compression! ret: %{public}d, message: %{public}s!", __func__, ret,
            zs.msg);
        return ERROR;
    }
    outputStr = compressedStr;
    return OK;
}

int32_t ZipUtils::DecompressString(const string& inputStr, string& outputStr)
{
    z_stream zs;
    memset_s(&zs, sizeof zs, 0, sizeof zs);
    if (inflateInit(&zs) != Z_OK) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: inflateInit failed while compressing!", __func__);
        return ERROR;
    }
    zs.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(inputStr.data()));
    zs.avail_in = inputStr.size();

    int ret;
    char outBuffer[BUFFER_SIZE];
    string decompressedStr;
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outBuffer);
        zs.avail_out = sizeof(outBuffer);
        ret = inflate(&zs, 0);
        if (decompressedStr.size() < zs.total_out) {
            decompressedStr.append(outBuffer, zs.total_out - decompressedStr.size());
        }
    } while (ret == Z_OK);

    inflateEnd(&zs);
    if (ret != Z_STREAM_END) {
        PERMISSION_LOG_ERROR(LABEL,
            "%{public}s: Exception during zlib decompression! ret: %{public}d, message: %{public}s!", __func__, ret,
            zs.msg);
        return ERROR;
    }
    outputStr = decompressedStr;
    return OK;
}
} // namespace Permission
} // namespace Security
} // namespace OHOS
