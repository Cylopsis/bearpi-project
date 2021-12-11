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

#include "gif_decoder.h"

namespace OHOS {
namespace ImagePlugin {
using namespace OHOS::HiviewDFX;
using namespace MultimediaPlugin;
using namespace Media;
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "GifDecoder" };

namespace {
#if __BYTE_ORDER == __LITTLE_ENDIAN
constexpr uint8_t RGBA_R_SHIFT = 0;
constexpr uint8_t RGBA_G_SHIFT = 8;
constexpr uint8_t RGBA_B_SHIFT = 16;
constexpr uint8_t RGBA_A_SHIFT = 24;
#else
constexpr uint8_t RGBA_R_SHIFT = 24;
constexpr uint8_t RGBA_G_SHIFT = 16;
constexpr uint8_t RGBA_B_SHIFT = 8;
constexpr uint8_t RGBA_A_SHIFT = 0;
#endif
constexpr uint8_t NO_TRANSPARENT = 0xFF;
constexpr uint8_t DELAY_TIME_TO_MS_RATIO = 10;
constexpr uint8_t EXTENSION_LEN_INDEX = 0;
constexpr uint8_t EXTENSION_DATA_INDEX = 1;
constexpr int32_t INTERLACED_PASSES = 4;
constexpr int32_t INTERLACED_OFFSET[] = { 0, 4, 2, 1 };
constexpr int32_t INTERLACED_INTERVAL[] = { 8, 8, 4, 2 };
const std::string GIF_IMAGE_DELAY_TIME = "GIFDelayTime";
const std::string GIF_IMAGE_LOOP_COUNT = "GIFLoopCount";
constexpr int32_t NETSCAPE_EXTENSION_LENGTH = 11;
constexpr int32_t DELAY_TIME_LENGTH = 3;
constexpr int32_t DELAY_TIME_INDEX1 = 1;
constexpr int32_t DELAY_TIME_INDEX2 = 2;
constexpr int32_t DELAY_TIME_SHIFT = 8;
} // namespace

GifDecoder::GifDecoder()
{}

GifDecoder::~GifDecoder()
{
    Reset();
}

void GifDecoder::SetSource(InputDataStream &sourceStream)
{
    Reset();
    inputStreamPtr_ = &sourceStream;
}

// need decode all frame to get total number.
uint32_t GifDecoder::GetTopLevelImageNum(uint32_t &num)
{
    if (inputStreamPtr_ == nullptr) {
        HiLog::Error(LABEL, "[GetTopLevelImageNum]set source need firstly");
        return ERR_IMAGE_DATA_ABNORMAL;
    }
    if (!inputStreamPtr_->IsStreamCompleted()) {
        HiLog::Warn(LABEL, "[GetTopLevelImageNum]don't enough data to decode the frame number");
        return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
    }
    uint32_t errorCode = CreateGifFileTypeIfNotExist();
    if (errorCode != SUCCESS) {
        HiLog::Error(LABEL, "[GetTopLevelImageNum]create GifFileType pointer failed %{public}u", errorCode);
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    if (!isLoadAllFrame_) {
        errorCode = UpdateGifFileType(INT_MAX);
        if (errorCode != SUCCESS) {
            HiLog::Error(LABEL, "[GetTopLevelImageNum]update GifFileType pointer failed %{public}u", errorCode);
            return ERR_IMAGE_DECODE_ABNORMAL;
        }
    }
    num = gifPtr_->ImageCount;
    if (num <= 0) {
        HiLog::Error(LABEL, "[GetTopLevelImageNum]image frame number must be larger than 0");
        return ERR_IMAGE_DATA_ABNORMAL;
    }
    return SUCCESS;
}

// return background size but not specific frame size, cause of frame drawing on background.
uint32_t GifDecoder::GetImageSize(uint32_t index, PlSize &size)
{
    uint32_t errorCode = CheckIndex(index);
    if (errorCode != SUCCESS) {
        HiLog::Error(LABEL, "[GetImageSize]index %{public}u is invalid %{public}u", index, errorCode);
        return errorCode;
    }
    const int32_t bgWidth = gifPtr_->SWidth;
    const int32_t bgHeight = gifPtr_->SHeight;
    if (bgWidth <= 0 || bgHeight <= 0) {
        HiLog::Error(LABEL, "[GetImageSize]background size [%{public}d, %{public}d] is invalid", bgWidth, bgHeight);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    size.width = bgWidth;
    size.height = bgHeight;
    return SUCCESS;
}

uint32_t GifDecoder::SetDecodeOptions(uint32_t index, const PixelDecodeOptions &opts, PlImageInfo &info)
{
    uint32_t errorCode = GetImageSize(index, info.size);
    if (errorCode != SUCCESS) {
        HiLog::Error(LABEL, "[SetDecodeOptions]get image size failed %{public}u", errorCode);
        return errorCode;
    }
    info.alphaType = PlAlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    // only support RGBA pixel format for performance.
    info.pixelFormat = PlPixelFormat::RGBA_8888;
    return SUCCESS;
}

uint32_t GifDecoder::Decode(uint32_t index, DecodeContext &context)
{
    PlSize imageSize;
    uint32_t errorCode = GetImageSize(index, imageSize);
    if (errorCode != SUCCESS) {
        HiLog::Error(LABEL, "[Decode]index %{public}u is invalid %{public}u", index, errorCode);
        return errorCode;
    }
    // compute start index and end index.
    bool isOverlapped = false;
    uint32_t startIndex = 0;
    uint32_t endIndex = index;
    int32_t acquiredIndex = static_cast<int32_t>(index);
    if (acquiredIndex > lastPixelMapIndex_) {
        startIndex = lastPixelMapIndex_ + 1;
    } else if (acquiredIndex == lastPixelMapIndex_) {
        isOverlapped = true;
    }
    // avoid local pixelmap buffer was reset, start with first frame again.
    if (startIndex != 0 && localPixelMapBuffer_ == nullptr) {
        startIndex = 0;
        isOverlapped = false;
    }
    HiLog::Debug(LABEL, "[Decode]start frame: %{public}u, last frame: %{public}u,"
                 "last pixelMapIndex: %{public}d, isOverlapped: %{public}d",
                 startIndex, endIndex, lastPixelMapIndex_, isOverlapped);

    if (!isOverlapped) {
        errorCode = OverlapFrame(startIndex, endIndex);
        if (errorCode != SUCCESS) {
            HiLog::Error(LABEL, "[Decode]overlap frame failed %{public}u", errorCode);
            return errorCode;
        }
    }
    errorCode = RedirectOutputBuffer(context);
    if (errorCode != SUCCESS) {
        HiLog::Error(LABEL, "[Decode]redirect output stream failed %{public}u", errorCode);
        return errorCode;
    }
    return SUCCESS;
}

uint32_t GifDecoder::PromoteIncrementalDecode(uint32_t index, ProgDecodeContext &context)
{
    uint32_t errorCode = Decode(index, context.decodeContext);
    // get promote decode progress, in percentage: 0~100.
    context.totalProcessProgress = (errorCode == SUCCESS ? 100 : 0);
    return errorCode;
}

void GifDecoder::Reset()
{
    if (gifPtr_ != nullptr) {
        DGifCloseFile(gifPtr_, nullptr);
        gifPtr_ = nullptr;
    }
    FreeLocalPixelMapBuffer();  // free local pixelmap buffer
    inputStreamPtr_ = nullptr;
    isLoadAllFrame_ = false;
    lastPixelMapIndex_ = -1;
    savedFrameIndex_ = -1;
    bgColor_ = 0;
}

uint32_t GifDecoder::CreateGifFileTypeIfNotExist()
{
    if (gifPtr_ == nullptr) {
        int32_t errorCode = Media::ERROR;
        if (inputStreamPtr_ == nullptr) {
            HiLog::Error(LABEL, "[CreateGifFileTypeIfNotExist]set source need firstly");
            return ERR_IMAGE_GET_DATA_ABNORMAL;
        }
        // DGifOpen will create GifFileType pointer and set header and screen desc
        gifPtr_ = DGifOpen(inputStreamPtr_, InputStreamReader, &errorCode);
        if (gifPtr_ == nullptr) {
            HiLog::Error(LABEL, "[CreateGifFileTypeIfNotExist]open image error, %{public}d", errorCode);
            inputStreamPtr_->Seek(0);
            savedFrameIndex_ = -1;
            return ERR_IMAGE_SOURCE_DATA;
        }
        ParseBgColor();
    }
    return SUCCESS;
}

int32_t GifDecoder::InputStreamReader(GifFileType *gif, GifByteType *bytes, int32_t size)
{
    uint32_t dataSize = 0;
    if (gif == nullptr) {
        HiLog::Error(LABEL, "[InputStreamReader]GifFileType pointer is null");
        return dataSize;
    }
    InputDataStream *inputStream = static_cast<InputDataStream *>(gif->UserData);
    if (inputStream == nullptr) {
        HiLog::Error(LABEL, "[InputStreamReader]set source need firstly");
        return dataSize;
    }
    if (size <= 0) {
        HiLog::Error(LABEL, "[InputStreamReader]callback size %{public}d is invalid", size);
        return dataSize;
    }
    if (bytes == nullptr) {
        HiLog::Error(LABEL, "[InputStreamReader]callback buffer is null");
        return dataSize;
    }
    inputStream->Read(size, bytes, size, dataSize);
    return dataSize;
}

uint32_t GifDecoder::CheckIndex(uint32_t index)
{
    if (!inputStreamPtr_->IsStreamCompleted()) {
        HiLog::Warn(LABEL, "[CheckIndex]don't enough data to decode the frame number");
        return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
    }
    uint32_t errorCode = CreateGifFileTypeIfNotExist();
    if (errorCode != SUCCESS) {
        HiLog::Error(LABEL, "[CheckIndex]create GifFileType failed %{public}u", errorCode);
        return errorCode;
    }
    int32_t updateFrameIndex = static_cast<int32_t>(index);
    if (!isLoadAllFrame_ && updateFrameIndex > savedFrameIndex_) {
        errorCode = UpdateGifFileType(updateFrameIndex);
        if (errorCode != SUCCESS) {
            HiLog::Error(LABEL, "[CheckIndex]update saved frame to index %{public}u failed", index);
            return errorCode;
        }
    }
    uint32_t frameNum = gifPtr_->ImageCount;
    if (index >= frameNum) {
        HiLog::Error(LABEL, "[CheckIndex]index %{public}u out of frame range %{public}u", index, frameNum);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    return SUCCESS;
}

uint32_t GifDecoder::OverlapFrame(uint32_t startIndex, uint32_t endIndex)
{
    for (uint32_t frameIndex = startIndex; frameIndex <= endIndex; frameIndex++) {
        const SavedImage *savedImage = gifPtr_->SavedImages + frameIndex;
        if (savedImage == nullptr) {
            HiLog::Error(LABEL, "[OverlapFrame]image frame %{public}u data is invalid", frameIndex);
            return ERR_IMAGE_DECODE_ABNORMAL;
        }
        // acquire the frame graphices control information
        int32_t transColor = NO_TRANSPARENT_COLOR;
        int32_t disposalMode = DISPOSAL_UNSPECIFIED;
        GetTransparentAndDisposal(frameIndex, transColor, disposalMode);
        HiLog::Debug(LABEL,
                     "[OverlapFrame]frameIndex = %{public}u, transColor = %{public}d, "
                     "disposalMode = %{public}d",
                     frameIndex, transColor, disposalMode);

        if (frameIndex == 0 && AllocateLocalPixelMapBuffer() != SUCCESS) {
            HiLog::Error(LABEL, "[OverlapFrame]first frame allocate local pixelmap buffer failed");
            return ERR_IMAGE_DECODE_ABNORMAL;
        }
        if (localPixelMapBuffer_ == nullptr) {
            HiLog::Error(LABEL, "[OverlapFrame]local pixelmap is null, next frame can't overlap");
            return ERR_IMAGE_DECODE_ABNORMAL;
        }
        // current frame recover background
        if (frameIndex != 0 && disposalMode == DISPOSE_BACKGROUND &&
            DisposeBackground(frameIndex, savedImage) != SUCCESS) {
            HiLog::Error(LABEL, "[OverlapFrame]dispose frame %{public}d background failed", frameIndex);
            return ERR_IMAGE_DECODE_ABNORMAL;
        }
        if (disposalMode != DISPOSE_PREVIOUS &&
            PaddingData(savedImage, transColor) != SUCCESS) {
            HiLog::Error(LABEL, "[OverlapFrame]dispose frame %{public}u data color failed", frameIndex);
            return ERR_IMAGE_DECODE_ABNORMAL;
        }
    }
    lastPixelMapIndex_ = endIndex;
    return SUCCESS;
}

uint32_t GifDecoder::DisposeBackground(uint32_t frameIndex, const SavedImage *curSavedImage)
{
    int32_t preTransColor = NO_TRANSPARENT_COLOR;
    int32_t preDisposalMode = DISPOSAL_UNSPECIFIED;
    GetTransparentAndDisposal(frameIndex - 1, preTransColor, preDisposalMode);
    SavedImage *preSavedImage = gifPtr_->SavedImages + frameIndex - 1;
    if (preDisposalMode == DISPOSE_BACKGROUND && IsFramePreviousCoveredCurrent(preSavedImage, curSavedImage)) {
        return SUCCESS;
    }
    if (PaddingBgColor(curSavedImage) != SUCCESS) {
        HiLog::Error(LABEL, "[DisposeBackground]padding frame %{public}u background color failed", frameIndex);
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    return SUCCESS;
}

bool GifDecoder::IsFramePreviousCoveredCurrent(const SavedImage *preSavedImage, const SavedImage *curSavedImage)
{
    return ((preSavedImage->ImageDesc.Left <= curSavedImage->ImageDesc.Left) &&
            (preSavedImage->ImageDesc.Left + preSavedImage->ImageDesc.Width >=
             curSavedImage->ImageDesc.Left + curSavedImage->ImageDesc.Width) &&
            (preSavedImage->ImageDesc.Top <= curSavedImage->ImageDesc.Top) &&
            (preSavedImage->ImageDesc.Top + preSavedImage->ImageDesc.Height >=
             curSavedImage->ImageDesc.Top + curSavedImage->ImageDesc.Height));
}

uint32_t GifDecoder::AllocateLocalPixelMapBuffer()
{
    if (localPixelMapBuffer_ == nullptr) {
        int32_t bgWidth = gifPtr_->SWidth;
        int32_t bgHeight = gifPtr_->SHeight;
        uint64_t pixelMapBufferSize = static_cast<uint64_t>(bgWidth) * bgHeight * sizeof(uint32_t);
        // create local pixelmap buffer, next frame depends on the previous
        if (pixelMapBufferSize > PIXEL_MAP_MAX_RAM_SIZE) {
            HiLog::Error(LABEL, "[AllocateLocalPixelMapBuffer]pixelmap buffer size %{public}llu out of max size",
                         static_cast<unsigned long long>(pixelMapBufferSize));
            return ERR_IMAGE_TOO_LARGE;
        }
        localPixelMapBuffer_ = reinterpret_cast<uint32_t *>(malloc(pixelMapBufferSize));
        if (localPixelMapBuffer_ == nullptr) {
            HiLog::Error(LABEL, "[AllocateLocalPixelMapBuffer]allocate local pixelmap buffer memory error");
            return ERR_IMAGE_MALLOC_ABNORMAL;
        }
#ifdef _WIN32
        memset(localPixelMapBuffer_, bgColor_, pixelMapBufferSize);
#else
        if (memset_s(localPixelMapBuffer_, pixelMapBufferSize, bgColor_, pixelMapBufferSize) != EOK) {
            HiLog::Error(LABEL, "[DisposeFirstPixelMap]memset local pixelmap buffer background failed");
            FreeLocalPixelMapBuffer();
            return ERR_IMAGE_MALLOC_ABNORMAL;
        }
#endif
    }
    return SUCCESS;
}

void GifDecoder::FreeLocalPixelMapBuffer()
{
    if (localPixelMapBuffer_ != nullptr) {
        free(localPixelMapBuffer_);
        localPixelMapBuffer_ = nullptr;
    }
}

uint32_t GifDecoder::PaddingBgColor(const SavedImage *savedImage)
{
    int32_t bgWidth = gifPtr_->SWidth;
    int32_t bgHeight = gifPtr_->SHeight;
    int32_t frameLeft = savedImage->ImageDesc.Left;
    int32_t frameTop = savedImage->ImageDesc.Top;
    int32_t frameWidth = savedImage->ImageDesc.Width;
    int32_t frameHeight = savedImage->ImageDesc.Height;
    if (frameLeft + frameWidth > bgWidth) {
        frameWidth = bgWidth - frameLeft;
    }
    if (frameTop + frameHeight > bgHeight) {
        frameHeight = bgHeight - frameTop;
    }
    if (frameWidth < 0 || frameHeight < 0) {
        HiLog::Error(LABEL, "[PaddingBgColor]frameWidth || frameHeight is abnormal,"
                     "bgWidth:%{public}d, bgHeight:%{public}d, "
                     "frameTop:%{public}d, frameLeft:%{public}d",
                     bgWidth, bgHeight, frameTop, frameLeft);
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    uint32_t *dstPixelMapBuffer = localPixelMapBuffer_ + frameTop * bgWidth + frameLeft;
    uint32_t lineBufferSize = frameWidth * sizeof(uint32_t);
    for (int32_t row = 0; row < frameHeight; row++) {
#ifdef _WIN32
        memset(dstPixelMapBuffer, bgColor_, lineBufferSize);
#else
        if (memset_s(dstPixelMapBuffer, lineBufferSize, bgColor_, lineBufferSize) != EOK) {
            HiLog::Error(LABEL, "[PaddingBgColor]memset local pixelmap buffer failed");
            return ERR_IMAGE_MALLOC_ABNORMAL;
        }
#endif
        dstPixelMapBuffer += bgWidth;
    }
    return SUCCESS;
}

uint32_t GifDecoder::PaddingData(const SavedImage *savedImage, int32_t transparentColor)
{
    const ColorMapObject *colorMap = gifPtr_->SColorMap;
    if (savedImage->ImageDesc.ColorMap != nullptr) {
        colorMap = savedImage->ImageDesc.ColorMap;  // local color map
    }
    if (colorMap == nullptr) {
        HiLog::Error(LABEL, "[PaddingData]color map is null");
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    int32_t colorCount = colorMap->ColorCount;
    int32_t bitsPerPixel = colorMap->BitsPerPixel;
    if ((bitsPerPixel < 0) || (colorCount != (1 << static_cast<uint32_t>(bitsPerPixel)))) {
        HiLog::Error(LABEL, "[PaddingData]colormap is invalid, bitsPerPixel: %{public}d, colorCount: %{public}d",
                     bitsPerPixel, colorCount);
        return ERR_IMAGE_DECODE_ABNORMAL;
    }

    int32_t bgWidth = gifPtr_->SWidth;
    int32_t bgHeight = gifPtr_->SHeight;
    int32_t frameLeft = savedImage->ImageDesc.Left;
    int32_t frameTop = savedImage->ImageDesc.Top;
    int32_t frameWidth = savedImage->ImageDesc.Width;
    int32_t frameHeight = savedImage->ImageDesc.Height;
    if (frameLeft + frameWidth > bgWidth) {
        frameWidth = bgWidth - frameLeft;
    }
    if (frameTop + frameHeight > bgHeight) {
        frameHeight = bgHeight - frameTop;
    }
    const GifByteType *srcFrame = savedImage->RasterBits;
    uint32_t *dstPixelMapBuffer = localPixelMapBuffer_ + frameTop * bgWidth + frameLeft;
    for (int32_t row = 0; row < frameHeight; row++) {
        CopyLine(srcFrame, dstPixelMapBuffer, frameWidth, transparentColor, colorMap);
        srcFrame += savedImage->ImageDesc.Width;
        dstPixelMapBuffer += bgWidth;
    }
    return SUCCESS;
}

void GifDecoder::CopyLine(const GifByteType *srcFrame, uint32_t *dstPixelMapBuffer, int32_t frameWidth,
                          int32_t transparentColor, const ColorMapObject *colorMap)
{
    for (int32_t col = 0; col < frameWidth; col++, srcFrame++, dstPixelMapBuffer++) {
        if ((*srcFrame != transparentColor) && (*srcFrame < colorMap->ColorCount)) {
            const GifColorType &colorType = colorMap->Colors[*srcFrame];
            *dstPixelMapBuffer = GetPixelColor(colorType.Red, colorType.Green, colorType.Blue, NO_TRANSPARENT);
        }
    }
}

void GifDecoder::GetTransparentAndDisposal(uint32_t index, int32_t &transparentColor, int32_t &disposalMode)
{
    GraphicsControlBlock graphicsControlBlock = GetGraphicsControlBlock(index);
    transparentColor = graphicsControlBlock.TransparentColor;
    disposalMode = graphicsControlBlock.DisposalMode;
}

GraphicsControlBlock GifDecoder::GetGraphicsControlBlock(uint32_t index)
{
    GraphicsControlBlock graphicsControlBlock = { DISPOSAL_UNSPECIFIED, false, 0, NO_TRANSPARENT_COLOR };
    DGifSavedExtensionToGCB(gifPtr_, index, &graphicsControlBlock);
    return graphicsControlBlock;
}

uint32_t GifDecoder::GetPixelColor(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha)
{
    return (red << RGBA_R_SHIFT) | (green << RGBA_G_SHIFT) | (blue << RGBA_B_SHIFT) | (alpha << RGBA_A_SHIFT);
}

void GifDecoder::ParseBgColor()
{
    const int32_t bgColorIndex = gifPtr_->SBackGroundColor;
    if (bgColorIndex < 0) {
        HiLog::Warn(LABEL, "[ParseBgColor]bgColor index %{public}d is invalid, use default bgColor", bgColorIndex);
        return;
    }
    const ColorMapObject *bgColorMap = gifPtr_->SColorMap;
    if ((bgColorMap != nullptr) && (bgColorIndex < bgColorMap->ColorCount)) {
        const GifColorType bgColorType = bgColorMap->Colors[bgColorIndex];
        bgColor_ = GetPixelColor(bgColorType.Red, bgColorType.Green, bgColorType.Blue, NO_TRANSPARENT);
    }
}

uint32_t GifDecoder::RedirectOutputBuffer(DecodeContext &context)
{
    if (localPixelMapBuffer_ == nullptr) {
        HiLog::Error(LABEL, "[RedirectOutputBuffer]local pixelmap buffer is null, redirect failed");
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    int32_t bgWidth = gifPtr_->SWidth;
    int32_t bgHeight = gifPtr_->SHeight;
    uint64_t imageBufferSize = static_cast<uint64_t>(bgWidth) * bgHeight * sizeof(uint32_t);

    if (context.allocatorType == Media::AllocatorType::SHARE_MEM_ALLOC) {
        if (context.pixelsBuffer.buffer == nullptr) {
#if !defined(_WIN32) && !defined(_APPLE)
            int fd = AshmemCreate("GIF RawData", imageBufferSize);
            if (fd < 0) {
                return ERR_SHAMEM_DATA_ABNORMAL;
            }
            int result = AshmemSetProt(fd, PROT_READ | PROT_WRITE);
            if (result < 0) {
                ::close(fd);
                return ERR_SHAMEM_DATA_ABNORMAL;
            }
            void* ptr = ::mmap(nullptr, imageBufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (ptr == MAP_FAILED) {
                ::close(fd);
                return ERR_SHAMEM_DATA_ABNORMAL;
            }
            context.pixelsBuffer.buffer = ptr;
            void *fdBuffer = new int32_t();
            if (fdBuffer == nullptr) {
                HiLog::Error(LABEL, "new fdBuffer fail");
                ::munmap(ptr, imageBufferSize);
                ::close(fd);
                context.pixelsBuffer.buffer = nullptr;
                return ERR_SHAMEM_DATA_ABNORMAL;
            }
            *static_cast<int32_t *>(fdBuffer) = fd;
            context.pixelsBuffer.context = fdBuffer;
            context.pixelsBuffer.bufferSize = imageBufferSize;
            context.pixelsBuffer.dataSize = imageBufferSize;
            context.allocatorType = AllocatorType::SHARE_MEM_ALLOC;
            context.freeFunc = nullptr;
#endif
        }
    } else {
        bool isPluginAllocateMemory = false;
        if (context.pixelsBuffer.buffer == nullptr) {
            // outer manage the buffer.
            void *outputBuffer = malloc(imageBufferSize);
            if (outputBuffer == nullptr) {
                HiLog::Error(LABEL, "[RedirectOutputBuffer]alloc output buffer size %{public}llu failed",
                             static_cast<unsigned long long>(imageBufferSize));
                return ERR_IMAGE_MALLOC_ABNORMAL;
            }
            context.pixelsBuffer.buffer = outputBuffer;
            context.pixelsBuffer.bufferSize = imageBufferSize;
            isPluginAllocateMemory = true;
        }
        if (memcpy_s(context.pixelsBuffer.buffer, context.pixelsBuffer.bufferSize,
            localPixelMapBuffer_, imageBufferSize) != 0) {
            HiLog::Error(LABEL, "[RedirectOutputBuffer]memory copy size %{public}llu failed",
                         static_cast<unsigned long long>(imageBufferSize));
            if (isPluginAllocateMemory) {
                context.pixelsBuffer.bufferSize = 0;
                free(context.pixelsBuffer.buffer);
                context.pixelsBuffer.buffer = nullptr;
            }
            return ERR_IMAGE_DECODE_ABNORMAL;
        }
        context.pixelsBuffer.dataSize = imageBufferSize;
        context.allocatorType = AllocatorType::HEAP_ALLOC;
    }
    return SUCCESS;
}

uint32_t GifDecoder::GetImageDelayTime(uint32_t index, int32_t &value)
{
    uint32_t errorCode = CheckIndex(index);
    if (errorCode != SUCCESS) {
        HiLog::Error(LABEL, "[GetImageDelayTime]index %{public}u is invalid", index);
        return errorCode;
    }

    GraphicsControlBlock graphicsControlBlock = GetGraphicsControlBlock(index);
    // 0.01 sec in standard, update to ms
    value = graphicsControlBlock.DelayTime * DELAY_TIME_TO_MS_RATIO;
    return SUCCESS;
}

uint32_t GifDecoder::GetImageLoopCount(uint32_t index, int32_t &value)
{
    for (int i = 0; i < gifPtr_->SavedImages[index].ExtensionBlockCount; i++) {
        ExtensionBlock *ep = &gifPtr_->SavedImages[index].ExtensionBlocks[i];
        if (ep == nullptr) {
            continue;
        }
        if ((ep->Function == APPLICATION_EXT_FUNC_CODE) && (ep->ByteCount >= NETSCAPE_EXTENSION_LENGTH) &&
            (memcmp(ep->Bytes, "NETSCAPE2.0", NETSCAPE_EXTENSION_LENGTH) == 0)) {
            ep++;
            if (ep->ByteCount >= DELAY_TIME_LENGTH) {
                unsigned char *params = ep->Bytes;
                value = params[DELAY_TIME_INDEX1] | (params[DELAY_TIME_INDEX2] << DELAY_TIME_SHIFT);
                return SUCCESS;
            }
        }
    }
    return ERR_IMAGE_PROPERTY_NOT_EXIST;
}

uint32_t GifDecoder::GetImagePropertyInt(uint32_t index, const std::string &key, int32_t &value)
{
    HiLog::Error(LABEL, "[GetImagePropertyInt] enter gif plugin, key:%{public}s", key.c_str());
    uint32_t errorCode = CheckIndex(0);
    if (errorCode != SUCCESS) {
        HiLog::Error(LABEL, "[GetImagePropertyInt]index %{public}u is invalid", index);
        return errorCode;
    }

    if (key == GIF_IMAGE_DELAY_TIME) {
        errorCode = GetImageDelayTime(index, value);
    } else if (key == GIF_IMAGE_LOOP_COUNT) {
        errorCode = GetImageLoopCount(0, value);
    } else {
        HiLog::Error(LABEL, "[GetImagePropertyInt]key(%{public}s) not supported", key.c_str());
        return ERR_IMAGE_INVALID_PARAMETER;
    }

    return errorCode;
}

uint32_t GifDecoder::ParseFrameDetail()
{
    if (DGifGetImageDesc(gifPtr_) == GIF_ERROR) {
        HiLog::Error(LABEL, "[ParseFrameDetail]parse frame desc to gif pointer failed %{public}d", gifPtr_->Error);
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    // DGifGetImageDesc use malloc or reallocarray allocate savedImages memory and increase imageCount.
    // If error, we don't free the memory, next time decode will retry the allocated memory.
    // The total memory free will be called DGifCloseFile.
    int32_t frameIndex = gifPtr_->ImageCount - 1;
    SavedImage *saveImagePtr = &gifPtr_->SavedImages[frameIndex];
    int32_t imageWidth = saveImagePtr->ImageDesc.Width;
    int32_t imageHeight = saveImagePtr->ImageDesc.Height;
    uint64_t imageSize = static_cast<uint64_t>(imageWidth) * imageHeight;
    if (imageWidth <= 0 || imageHeight <= 0 || imageSize > SIZE_MAX) {
        HiLog::Error(LABEL, "[ParseFrameDetail]check frame size[%{public}d, %{public}d] failed", imageWidth,
                     imageHeight);
        // if error, imageCount go back and next time DGifGetImageDesc will retry.
        gifPtr_->ImageCount--;
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    // set savedImage extension
    if (gifPtr_->ExtensionBlocks != nullptr) {
        saveImagePtr->ExtensionBlocks = gifPtr_->ExtensionBlocks;
        saveImagePtr->ExtensionBlockCount = gifPtr_->ExtensionBlockCount;
        gifPtr_->ExtensionBlocks = nullptr;
        gifPtr_->ExtensionBlockCount = 0;
    }
    // set savedImage rasterBits
    if (SetSavedImageRasterBits(saveImagePtr, frameIndex, imageSize, imageWidth, imageHeight) != SUCCESS) {
        HiLog::Error(LABEL, "[ParseFrameDetail] set saved image data failed");
        GifFreeExtensions(&saveImagePtr->ExtensionBlockCount, &saveImagePtr->ExtensionBlocks);
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    return SUCCESS;
}

uint32_t GifDecoder::SetSavedImageRasterBits(SavedImage *saveImagePtr, int32_t frameIndex, uint64_t imageSize,
                                             int32_t imageWidth, int32_t imageHeight)
{
    if (saveImagePtr->RasterBits == nullptr) {
        saveImagePtr->RasterBits = static_cast<GifPixelType *>(malloc(imageSize * sizeof(GifPixelType)));
        if (saveImagePtr->RasterBits == nullptr) {
            HiLog::Error(LABEL, "[SetSavedImageData]malloc frame %{public}d rasterBits failed", frameIndex);
            return ERR_IMAGE_MALLOC_ABNORMAL;
        }
    }
    // if error next time will retry the rasterBits and the pointer free will be called DGifCloseFile.
    if (saveImagePtr->ImageDesc.Interlace) {
        for (int32_t i = 0; i < INTERLACED_PASSES; i++) {
            for (int32_t j = INTERLACED_OFFSET[i]; j < imageHeight; j += INTERLACED_INTERVAL[i]) {
                if (DGifGetLine(gifPtr_, saveImagePtr->RasterBits + j * imageWidth, imageWidth) == GIF_ERROR) {
                    HiLog::Error(LABEL, "[SetSavedImageData]interlace set frame %{public}d bits failed %{public}d",
                                 frameIndex, gifPtr_->Error);
                    return ERR_IMAGE_DECODE_ABNORMAL;
                }
            }
        }
    } else {
        if (DGifGetLine(gifPtr_, saveImagePtr->RasterBits, imageSize) == GIF_ERROR) {
            HiLog::Error(LABEL, "[SetSavedImageData]normal set frame %{public}d bits failed %{public}d", frameIndex,
                         gifPtr_->Error);
            return ERR_IMAGE_DECODE_ABNORMAL;
        }
    }
    return SUCCESS;
}

uint32_t GifDecoder::ParseFrameExtension()
{
    GifByteType *extData = nullptr;
    int32_t extFunc = 0;
    if (DGifGetExtension(gifPtr_, &extFunc, &extData) == GIF_ERROR) {
        HiLog::Error(LABEL, "[ParseFrameExtension]get extension failed %{public}d", gifPtr_->Error);
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    if (extData == nullptr) {
        return SUCCESS;
    }

    HiLog::Debug(LABEL, "[ParseFrameExtension] get extension:0x%{public}x", extFunc);

    if (GifAddExtensionBlock(&gifPtr_->ExtensionBlockCount, &gifPtr_->ExtensionBlocks, extFunc,
        extData[EXTENSION_LEN_INDEX], &extData[EXTENSION_DATA_INDEX]) == GIF_ERROR) {
        HiLog::Error(LABEL, "[ParseFrameExtension]set extension to gif pointer failed");
        // GifAddExtensionBlock will allocate memory, if error, free extension ready to retry
        GifFreeExtensions(&gifPtr_->ExtensionBlockCount, &gifPtr_->ExtensionBlocks);
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    while (true) {
        if (DGifGetExtensionNext(gifPtr_, &extData) == GIF_ERROR) {
            HiLog::Error(LABEL, "[ParseFrameExtension]get next extension failed %{public}d", gifPtr_->Error);
            return ERR_IMAGE_DECODE_ABNORMAL;
        }
        if (extData == nullptr) {
            return SUCCESS;
        }

        if (GifAddExtensionBlock(&gifPtr_->ExtensionBlockCount, &gifPtr_->ExtensionBlocks, CONTINUE_EXT_FUNC_CODE,
            extData[EXTENSION_LEN_INDEX], &extData[EXTENSION_DATA_INDEX]) == GIF_ERROR) {
            HiLog::Error(LABEL, "[ParseFrameExtension]set next extension to gif pointer failed");
            GifFreeExtensions(&gifPtr_->ExtensionBlockCount, &gifPtr_->ExtensionBlocks);
            return ERR_IMAGE_DECODE_ABNORMAL;
        }
    }
    return SUCCESS;
}

uint32_t GifDecoder::UpdateGifFileType(int32_t updateFrameIndex)
{
    HiLog::Debug(LABEL, "[UpdateGifFileType]update %{public}d to %{public}d", savedFrameIndex_, updateFrameIndex);
    uint32_t startPosition = inputStreamPtr_->Tell();
    GifRecordType recordType;
    gifPtr_->ExtensionBlocks = nullptr;
    gifPtr_->ExtensionBlockCount = 0;
    do {
        if (DGifGetRecordType(gifPtr_, &recordType) == GIF_ERROR) {
            HiLog::Error(LABEL, "[UpdateGifFileType]parse file record type failed %{public}d", gifPtr_->Error);
            inputStreamPtr_->Seek(startPosition);
            return ERR_IMAGE_DECODE_ABNORMAL;
        }

        switch (recordType) {
            case EXTENSION_RECORD_TYPE:
                if (ParseFrameExtension() != SUCCESS) {
                    HiLog::Error(LABEL, "[UpdateGifFileType]parse frame extension failed");
                    inputStreamPtr_->Seek(startPosition);
                    return ERR_IMAGE_DECODE_ABNORMAL;
                }
                break;
            case IMAGE_DESC_RECORD_TYPE:
                if (ParseFrameDetail() != SUCCESS) {
                    HiLog::Error(LABEL, "[UpdateGifFileType]parse frame detail failed");
                    inputStreamPtr_->Seek(startPosition);
                    return ERR_IMAGE_DECODE_ABNORMAL;
                }
                savedFrameIndex_ = gifPtr_->ImageCount - 1;
                startPosition = inputStreamPtr_->Tell();
                break;
            case TERMINATE_RECORD_TYPE:
                HiLog::Debug(LABEL, "[UpdateGifFileType]parse gif completed");
                isLoadAllFrame_ = true;
                break;
            default:
                break;
        }

        if (isLoadAllFrame_ || savedFrameIndex_ == updateFrameIndex) {
            break;
        }
    } while (recordType != TERMINATE_RECORD_TYPE);

    if (gifPtr_->ImageCount <= 0) {
        gifPtr_->Error = D_GIF_ERR_NO_IMAG_DSCR;
        HiLog::Error(LABEL, "[UpdateGifFileType]has no frame in gif block");
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    return SUCCESS;
}
} // namespace ImagePlugin
} // namespace OHOS
