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

#include "pixel_map_parcel.h"
#include <unistd.h>
#include "hilog/log.h"
#include "log_tags.h"
#include "media_errors.h"
#ifndef _WIN32
#include "securec.h"
#else
#include "memory.h"
#endif

#if !defined(_WIN32) && !defined(_APPLE)
#include <sys/mman.h>
#include "ashmem.h"
#endif

namespace OHOS {
namespace Media {
using namespace OHOS::HiviewDFX;
using namespace std;
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "PixelMapParcel" };

constexpr int32_t PIXEL_MAP_INFO_MAX_LENGTH = 128;

void PixelMapParcel::ReleaseMemory(AllocatorType allocType, void *addr, void *context, uint32_t size)
{
    if (allocType == AllocatorType::SHARE_MEM_ALLOC) {
#if !defined(_WIN32) && !defined(_APPLE)
        int *fd = static_cast<int *>(context);
        if (addr != nullptr) {
            ::munmap(addr, size);
        }
        if (fd != nullptr) {
            ::close(*fd);
            delete fd;
        }
#endif
    } else if (allocType == AllocatorType::HEAP_ALLOC) {
        if (addr != nullptr) {
            free(addr);
            addr = nullptr;
        }
    }
}

std::unique_ptr<PixelMap> PixelMapParcel::CreateFromParcel(OHOS::MessageParcel& data)
{
    unique_ptr<PixelMap> pixelMap = make_unique<PixelMap>();
    if (pixelMap == nullptr) {
        HiLog::Error(LABEL, "create pixelmap pointer fail");
        return nullptr;
    }

    ImageInfo imgInfo;
    imgInfo.size.width = data.ReadInt32();
    imgInfo.size.height = data.ReadInt32();
    imgInfo.pixelFormat = static_cast<PixelFormat>(data.ReadInt32());
    imgInfo.colorSpace = static_cast<ColorSpace>(data.ReadInt32());
    imgInfo.alphaType = static_cast<AlphaType>(data.ReadInt32());
    imgInfo.baseDensity = data.ReadInt32();
    int32_t bufferSize = data.ReadInt32();
    AllocatorType allocType = static_cast<AllocatorType>(data.ReadInt32());
    uint8_t *base = nullptr;
    void *context = nullptr;
    if (allocType == AllocatorType::SHARE_MEM_ALLOC) {
#if !defined(_WIN32) && !defined(_APPLE)
        int fd = data.ReadFileDescriptor();
        if (fd < 0) {
            HiLog::Error(LABEL, "fd < 0");
            return nullptr;
        }
        HiLog::Debug(LABEL, "ReadFileDescriptor fd %{public}d.", fd);
        void* ptr = ::mmap(nullptr, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (ptr == MAP_FAILED) {
            ::close(fd);
            HiLog::Error(LABEL, "shared memory map failed");
            return nullptr;
        }
        context = new int32_t();
        if (context == nullptr) {
            HiLog::Error(LABEL, "alloc context error.");
            ::munmap(ptr, bufferSize);
            ::close(fd);
            return nullptr;
        }
        *static_cast<int32_t *>(context) = fd;
        base = static_cast<uint8_t *>(ptr);
#endif
    } else {
        const uint8_t *addr = data.ReadBuffer(bufferSize);
        if (addr == nullptr) {
            HiLog::Error(LABEL, "read buffer from parcel failed, read buffer addr is null");
            return nullptr;
        }
        base = static_cast<uint8_t *>(malloc(bufferSize));
        if (base == nullptr) {
            HiLog::Error(LABEL, "alloc output pixel memory size:[%{public}d] error.", bufferSize);
            return nullptr;
        }
        if (memcpy_s(base, bufferSize, addr, bufferSize) != 0) {
            free(base);
            base = nullptr;
            HiLog::Error(LABEL, "memcpy pixel data size:[%{public}d] error.", bufferSize);
            return nullptr;
        }
    }

    uint32_t ret = pixelMap->SetImageInfo(imgInfo);
    if (ret != SUCCESS) {
        ReleaseMemory(allocType, base, context, bufferSize);
        HiLog::Error(LABEL, "create pixel map from parcel failed, set image info error.");
        return nullptr;
    }
    pixelMap->SetPixelsAddr(base, context, bufferSize, allocType, nullptr);
    return pixelMap;
}

bool PixelMapParcel::WriteToParcel(PixelMap* pixelMap, OHOS::MessageParcel& data)
{
    if (pixelMap == nullptr) {
        return false;
    }
    int32_t bufferSize = pixelMap->GetByteCount();
    if (static_cast<size_t>(bufferSize + PIXEL_MAP_INFO_MAX_LENGTH) > data.GetDataCapacity() &&
        !data.SetDataCapacity(bufferSize + PIXEL_MAP_INFO_MAX_LENGTH)) {
        HiLog::Error(LABEL, "set parcel max capacity:[%{public}d] failed.", bufferSize + PIXEL_MAP_INFO_MAX_LENGTH);
        return false;
    }
    if (!data.WriteInt32(pixelMap->GetWidth())) {
        HiLog::Error(LABEL, "write pixel map width:[%{public}d] to parcel failed.", pixelMap->GetWidth());
        return false;
    }
    if (!data.WriteInt32(pixelMap->GetHeight())) {
        HiLog::Error(LABEL, "write pixel map height:[%{public}d] to parcel failed.", pixelMap->GetHeight());
        return false;
    }
    if (!data.WriteInt32(static_cast<int32_t>(pixelMap->GetPixelFormat()))) {
        HiLog::Error(LABEL, "write pixel map pixel format:[%{public}d] to parcel failed.", pixelMap->GetPixelFormat());
        return false;
    }
    if (!data.WriteInt32(static_cast<int32_t>(pixelMap->GetColorSpace()))) {
        HiLog::Error(LABEL, "write pixel map color space:[%{public}d] to parcel failed.", pixelMap->GetColorSpace());
        return false;
    }
    if (!data.WriteInt32(static_cast<int32_t>(pixelMap->GetAlphaType()))) {
        HiLog::Error(LABEL, "write pixel map alpha type:[%{public}d] to parcel failed.", pixelMap->GetAlphaType());
        return false;
    }
    if (!data.WriteInt32(pixelMap->GetBaseDensity())) {
        HiLog::Error(LABEL, "write pixel map base density:[%{public}d] to parcel failed.", pixelMap->GetBaseDensity());
        return false;
    }
    if (!data.WriteInt32(bufferSize)) {
        HiLog::Error(LABEL, "write pixel map buffer size:[%{public}d] to parcel failed.", bufferSize);
        return false;
    }
    if (!data.WriteInt32(static_cast<int32_t>(pixelMap->GetAllocatorType()))) {
        HiLog::Error(LABEL, "write pixel map allocator type:[%{public}d] to parcel failed.",
                     pixelMap->GetAllocatorType());
        return false;
    }
    if (pixelMap->GetAllocatorType() == AllocatorType::SHARE_MEM_ALLOC) {
#if !defined(_WIN32) && !defined(_APPLE)
        int *fd = static_cast<int *>(pixelMap->GetFd());
        if (*fd < 0) {
            HiLog::Error(LABEL, "write pixel map failed, fd < 0.");
            return false;
        }
        if (!data.WriteFileDescriptor(*fd)) {
            HiLog::Error(LABEL, "write pixel map fd:[%{public}d] to parcel failed.", *fd);
            return false;
        }
#endif
    } else {
        const uint8_t *addr = pixelMap->GetPixels();
        if (addr == nullptr) {
            HiLog::Error(LABEL, "write to parcel failed, pixel memory is null.");
            return false;
        }
        if (!data.WriteBuffer(addr, bufferSize)) {
            HiLog::Error(LABEL, "write pixel map buffer to parcel failed.");
            return false;
        }
    }
    return true;
}
}  // namespace Media
}  // namespace OHOS