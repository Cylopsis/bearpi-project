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

#include "image_source.h"

#include <algorithm>
#include <vector>
#include "buffer_source_stream.h"
#if !defined(_WIN32) && !defined(_APPLE)
#include "bytrace.h"
#endif
#include "file_source_stream.h"
#include "image/abs_image_decoder.h"
#include "image/abs_image_format_agent.h"
#include "image/image_plugin_type.h"
#include "image_log.h"
#include "image_utils.h"
#include "incremental_source_stream.h"
#include "istream_source_stream.h"
#include "media_errors.h"
#include "pixel_map.h"
#include "plugin_server.h"
#include "post_proc.h"
#include "source_stream.h"

namespace OHOS {
namespace Media {
using namespace OHOS::HiviewDFX;
using namespace std;
using namespace ImagePlugin;
using namespace MultimediaPlugin;

static const map<PixelFormat, PlPixelFormat> PIXEL_FORMAT_MAP = {
    { PixelFormat::UNKNOWN, PlPixelFormat::UNKNOWN },     { PixelFormat::ARGB_8888, PlPixelFormat::ARGB_8888 },
    { PixelFormat::ALPHA_8, PlPixelFormat::ALPHA_8 },     { PixelFormat::RGB_565, PlPixelFormat::RGB_565 },
    { PixelFormat::RGBA_F16, PlPixelFormat::RGBA_F16 },   { PixelFormat::RGBA_8888, PlPixelFormat::RGBA_8888 },
    { PixelFormat::BGRA_8888, PlPixelFormat::BGRA_8888 }, { PixelFormat::RGB_888, PlPixelFormat::RGB_888 },
    { PixelFormat::NV21, PlPixelFormat::NV21 },           { PixelFormat::NV12, PlPixelFormat::NV12 },
    { PixelFormat::CMYK, PlPixelFormat::CMYK }
};

static const map<ColorSpace, PlColorSpace> COLOR_SPACE_MAP = {
    { ColorSpace::UNKNOWN, PlColorSpace::UNKNOWN },
    { ColorSpace::DISPLAY_P3, PlColorSpace::DISPLAY_P3 },
    { ColorSpace::SRGB, PlColorSpace::SRGB },
    { ColorSpace::LINEAR_SRGB, PlColorSpace::LINEAR_SRGB },
    { ColorSpace::EXTENDED_SRGB, PlColorSpace::EXTENDED_SRGB },
    { ColorSpace::LINEAR_EXTENDED_SRGB, PlColorSpace::LINEAR_EXTENDED_SRGB },
    { ColorSpace::GENERIC_XYZ, PlColorSpace::GENERIC_XYZ },
    { ColorSpace::GENERIC_LAB, PlColorSpace::GENERIC_LAB },
    { ColorSpace::ACES, PlColorSpace::ACES },
    { ColorSpace::ACES_CG, PlColorSpace::ACES_CG },
    { ColorSpace::ADOBE_RGB_1998, PlColorSpace::ADOBE_RGB_1998 },
    { ColorSpace::DCI_P3, PlColorSpace::DCI_P3 },
    { ColorSpace::ITU_709, PlColorSpace::ITU_709 },
    { ColorSpace::ITU_2020, PlColorSpace::ITU_2020 },
    { ColorSpace::ROMM_RGB, PlColorSpace::ROMM_RGB },
    { ColorSpace::NTSC_1953, PlColorSpace::NTSC_1953 },
    { ColorSpace::SMPTE_C, PlColorSpace::SMPTE_C }
};

namespace InnerFormat {
    const string RAW_FORMAT = "image/x-raw";
    const string EXTENDED_FORMAT = "image/x-skia";
    const string RAW_EXTENDED_FORMATS[] = {
        "image/x-sony-arw",
        "image/x-canon-cr2",
        "image/x-adobe-dng",
        "image/x-nikon-nef",
        "image/x-nikon-nrw",
        "image/x-olympus-orf",
        "image/x-fuji-raf",
        "image/x-panasonic-rw2",
        "image/x-pentax-pef",
        "image/x-samsung-srw",
    };
}

PluginServer &ImageSource::pluginServer_ = ImageUtils::GetPluginServer();
ImageSource::FormatAgentMap ImageSource::formatAgentMap_ = InitClass();

uint32_t ImageSource::GetSupportedFormats(set<string> &formats)
{
    IMAGE_LOGD("[ImageSource]get supported image type.");

    formats.clear();
    vector<ClassInfo> classInfos;
    uint32_t ret = pluginServer_.PluginServerGetClassInfo<AbsImageDecoder>(AbsImageDecoder::SERVICE_DEFAULT,
                                                                           classInfos);
    if (ret != SUCCESS) {
        IMAGE_LOGE("[ImageSource]get class info from plugin server,ret:%{public}u.", ret);
        return ret;
    }

    for (auto &info : classInfos) {
        map<string, AttrData> &capbility = info.capabilities;
        auto iter = capbility.find(IMAGE_ENCODE_FORMAT);
        if (iter == capbility.end()) {
            continue;
        }

        AttrData &attr = iter->second;
        const string *format = nullptr;
        if (attr.GetValue(format) != SUCCESS) {
            IMAGE_LOGE("[ImageSource]attr data get format failed.");
            continue;
        }

        if (*format == InnerFormat::RAW_FORMAT) {
            formats.insert(std::begin(InnerFormat::RAW_EXTENDED_FORMATS), std::end(InnerFormat::RAW_EXTENDED_FORMATS));
        } else {
            formats.insert(*format);
        }
    }
    return SUCCESS;
}

unique_ptr<ImageSource> ImageSource::CreateImageSource(unique_ptr<istream> is, const SourceOptions &opts,
                                                       uint32_t &errorCode)
{
#if !defined(_WIN32) && !defined(_APPLE)
    StartTrace(BYTRACE_TAG_ZIMAGE, "CreateImageSource by istream");
#endif
    IMAGE_LOGD("[ImageSource]create Imagesource with stream.");

    unique_ptr<SourceStream> streamPtr = IstreamSourceStream::CreateSourceStream(move(is));
    if (streamPtr == nullptr) {
        IMAGE_LOGE("[ImageSource]failed to create istream source stream.");
        errorCode = ERR_IMAGE_SOURCE_DATA;
        return nullptr;
    }

    ImageSource *sourcePtr = new (std::nothrow) ImageSource(std::move(streamPtr), opts);
    if (sourcePtr == nullptr) {
        IMAGE_LOGE("[ImageSource]failed to create ImageSource with stream.");
        errorCode = ERR_IMAGE_SOURCE_DATA;
        return nullptr;
    }
    errorCode = SUCCESS;
#if !defined(_WIN32) && !defined(_APPLE)
    FinishTrace(BYTRACE_TAG_ZIMAGE, "CreateImageSource by istream");
#endif
    return unique_ptr<ImageSource>(sourcePtr);
}

unique_ptr<ImageSource> ImageSource::CreateImageSource(const uint8_t *data, uint32_t size, const SourceOptions &opts,
                                                       uint32_t &errorCode)
{
#if !defined(_WIN32) && !defined(_APPLE)
    StartTrace(BYTRACE_TAG_ZIMAGE, "CreateImageSource by data");
#endif
    IMAGE_LOGD("[ImageSource]create Imagesource with buffer.");

    if (data == nullptr || size == 0) {
        IMAGE_LOGE("[ImageSource]parameter error.");
        errorCode = ERR_IMAGE_DATA_ABNORMAL;
        return nullptr;
    }

    unique_ptr<SourceStream> streamPtr = BufferSourceStream::CreateSourceStream(data, size);
    if (streamPtr == nullptr) {
        IMAGE_LOGE("[ImageSource]failed to create buffer source stream.");
        errorCode = ERR_IMAGE_SOURCE_DATA;
        return nullptr;
    }

    ImageSource *sourcePtr = new (std::nothrow) ImageSource(std::move(streamPtr), opts);
    if (sourcePtr == nullptr) {
        IMAGE_LOGE("[ImageSource]failed to create ImageSource with buffer.");
        errorCode = ERR_IMAGE_SOURCE_DATA;
        return nullptr;
    }
    errorCode = SUCCESS;
#if !defined(_WIN32) && !defined(_APPLE)
    FinishTrace(BYTRACE_TAG_ZIMAGE, "CreateImageSource by data");
#endif
    return unique_ptr<ImageSource>(sourcePtr);
}

unique_ptr<ImageSource> ImageSource::CreateImageSource(const std::string &pathName, const SourceOptions &opts,
                                                       uint32_t &errorCode)
{
#if !defined(_WIN32) && !defined(_APPLE)
    StartTrace(BYTRACE_TAG_ZIMAGE, "CreateImageSource by path");
#endif
    IMAGE_LOGD("[ImageSource]create Imagesource with pathName.");

    unique_ptr<SourceStream> streamPtr = FileSourceStream::CreateSourceStream(pathName);
    if (streamPtr == nullptr) {
        IMAGE_LOGE("[ImageSource]failed to create file source stream.");
        errorCode = ERR_IMAGE_SOURCE_DATA;
        return nullptr;
    }

    ImageSource *sourcePtr = new (std::nothrow) ImageSource(std::move(streamPtr), opts);
    if (sourcePtr == nullptr) {
        IMAGE_LOGE("[ImageSource]failed to create ImageSource with pathName.");
        errorCode = ERR_IMAGE_SOURCE_DATA;
        return nullptr;
    }
    errorCode = SUCCESS;
#if !defined(_WIN32) && !defined(_APPLE)
    FinishTrace(BYTRACE_TAG_ZIMAGE, "CreateImageSource by path");
#endif
    return unique_ptr<ImageSource>(sourcePtr);
}

unique_ptr<ImageSource> ImageSource::CreateIncrementalImageSource(const IncrementalSourceOptions &opts,
                                                                  uint32_t &errorCode)
{
    IMAGE_LOGD("[ImageSource]create incremental ImageSource.");

    unique_ptr<SourceStream> streamPtr = IncrementalSourceStream::CreateSourceStream(opts.incrementalMode);
    if (streamPtr == nullptr) {
        IMAGE_LOGE("[ImageSource]failed to create incremental source stream.");
        errorCode = ERR_IMAGE_SOURCE_DATA;
        return nullptr;
    }

    ImageSource *sourcePtr = new (std::nothrow) ImageSource(std::move(streamPtr), opts.sourceOptions);
    if (sourcePtr == nullptr) {
        IMAGE_LOGE("[ImageSource]failed to create incremental ImageSource.");
        errorCode = ERR_IMAGE_SOURCE_DATA;
        return nullptr;
    }
    sourcePtr->SetIncrementalSource(true);
    errorCode = SUCCESS;
    return unique_ptr<ImageSource>(sourcePtr);
}

void ImageSource::Reset()
{
    // if use skia now, no need reset
    if (mainDecoder_ != nullptr && mainDecoder_->HasProperty(SKIA_DECODER)) {
        return;
    }
    imageStatusMap_.clear();
    decodeState_ = SourceDecodingState::UNRESOLVED;
    sourceStreamPtr_->Seek(0);
    mainDecoder_ = nullptr;
}

unique_ptr<PixelMap> ImageSource::CreatePixelMap(uint32_t index, const DecodeOptions &opts, uint32_t &errorCode)
{
#if !defined(_WIN32) && !defined(_APPLE)
    StartTrace(BYTRACE_TAG_ZIMAGE, "CreatePixelMap");
#endif
    std::unique_lock<std::mutex> guard(decodingMutex_);
    opts_ = opts;
    bool useSkia = opts_.sampleSize != 1;
    if (useSkia) {
        // we need reset to initial state to choose correct decoder
        Reset();
    }
    auto iter = GetValidImageStatus(index, errorCode);
    if (iter == imageStatusMap_.end()) {
        IMAGE_LOGE("[ImageSource]get valid image status fail on create pixel map, ret:%{public}u.", errorCode);
        return nullptr;
    }
    // the mainDecoder_ may be borrowed by Incremental decoding, so needs to be checked.
    if (InitMainDecoder() != SUCCESS) {
        IMAGE_LOGE("[ImageSource]image decode plugin is null.");
        errorCode = ERR_IMAGE_PLUGIN_CREATE_FAILED;
        return nullptr;
    }
    unique_ptr<PixelMap> pixelMap = make_unique<PixelMap>();
    if (pixelMap == nullptr) {
        IMAGE_LOGE("[ImageSource]create the pixel map unique_ptr fail.");
        errorCode = ERR_IMAGE_MALLOC_ABNORMAL;
        return nullptr;
    }

    ImagePlugin::PlImageInfo plInfo;
    errorCode = SetDecodeOptions(mainDecoder_, index, opts, plInfo);
    if (errorCode != SUCCESS) {
        IMAGE_LOGE("[ImageSource]set decode options error (index:%{public}u), ret:%{public}u.", index, errorCode);
        return nullptr;
    }

    for (auto listener : decodeListeners_) {
        guard.unlock();
        listener->OnEvent((int)DecodeEvent::EVENT_HEADER_DECODE);
        guard.lock();
    }

    errorCode = UpdatePixelMapInfo(opts, plInfo, *(pixelMap.get()));
    if (errorCode != SUCCESS) {
        IMAGE_LOGE("[ImageSource]update pixelmap info error ret:%{public}u.", errorCode);
        return nullptr;
    }

    DecodeContext context;
    FinalOutputStep finalOutputStep;
    if (!useSkia) {
        bool hasNinePatch = mainDecoder_->HasProperty(NINE_PATCH);
        finalOutputStep = GetFinalOutputStep(opts, *(pixelMap.get()), hasNinePatch);
        IMAGE_LOGD("[ImageSource]finalOutputStep:%{public}d. opts.allocatorType %{public}d",
            finalOutputStep, opts.allocatorType);

        if (finalOutputStep == FinalOutputStep::NO_CHANGE) {
            context.allocatorType = opts.allocatorType;
        } else {
            context.allocatorType = AllocatorType::HEAP_ALLOC;
        }
    }

    errorCode = mainDecoder_->Decode(index, context);
    if (context.ifPartialOutput) {
        for (auto listener : decodeListeners_) {
            guard.unlock();
            listener->OnEvent((int)DecodeEvent::EVENT_PARTIAL_DECODE);
            guard.lock();
        }
    }
    if (!useSkia) {
        ninePatchInfo_.ninePatch = context.ninePatchContext.ninePatch;
        ninePatchInfo_.patchSize = context.ninePatchContext.patchSize;
    }
    guard.unlock();
    if (errorCode != SUCCESS) {
        IMAGE_LOGE("[ImageSource]decode source fail, ret:%{public}u.", errorCode);
        if (context.pixelsBuffer.buffer != nullptr) {
            if (context.freeFunc != nullptr) {
                context.freeFunc(context.pixelsBuffer.buffer, context.pixelsBuffer.context,
                                 context.pixelsBuffer.bufferSize);
            } else {
                free(context.pixelsBuffer.buffer);
            }
        }
        return nullptr;
    }
    pixelMap->SetPixelsAddr(context.pixelsBuffer.buffer, context.pixelsBuffer.context, context.pixelsBuffer.bufferSize,
                            context.allocatorType, context.freeFunc);
    DecodeOptions procOpts;
    CopyOptionsToProcOpts(opts, procOpts, *(pixelMap.get()));
    PostProc postProc;
    errorCode = postProc.DecodePostProc(procOpts, *(pixelMap.get()), finalOutputStep);
    if (errorCode != SUCCESS) {
        return nullptr;
    }

    if (!context.ifPartialOutput) {
        for (auto listener : decodeListeners_) {
            listener->OnEvent((int)DecodeEvent::EVENT_COMPLETE_DECODE);
        }
    }
#if !defined(_WIN32) && !defined(_APPLE)
    FinishTrace(BYTRACE_TAG_ZIMAGE, "CreatePixelMap");
#endif
    return pixelMap;
}

unique_ptr<IncrementalPixelMap> ImageSource::CreateIncrementalPixelMap(uint32_t index, const DecodeOptions &opts,
                                                                       uint32_t &errorCode)
{
    IncrementalPixelMap *incPixelMapPtr = new (std::nothrow) IncrementalPixelMap(index, opts, this);
    if (incPixelMapPtr == nullptr) {
        IMAGE_LOGE("[ImageSource]create the incremental pixel map unique_ptr fail.");
        errorCode = ERR_IMAGE_MALLOC_ABNORMAL;
        return nullptr;
    }
    errorCode = SUCCESS;
    return unique_ptr<IncrementalPixelMap>(incPixelMapPtr);
}

uint32_t ImageSource::PromoteDecoding(uint32_t index, const DecodeOptions &opts, PixelMap &pixelMap,
                                      ImageDecodingState &state, uint8_t &decodeProgress)
{
    state = ImageDecodingState::UNRESOLVED;
    decodeProgress = 0;
    uint32_t ret = SUCCESS;
    std::unique_lock<std::mutex> guard(decodingMutex_);
    auto imageStatusIter = GetValidImageStatus(index, ret);
    if (imageStatusIter == imageStatusMap_.end()) {
        IMAGE_LOGE("[ImageSource]get valid image status fail on promote decoding, ret:%{public}u.", ret);
        return ret;
    }
    auto incrementalRecordIter = incDecodingMap_.find(&pixelMap);
    if (incrementalRecordIter == incDecodingMap_.end()) {
        ret = AddIncrementalContext(pixelMap, incrementalRecordIter);
        if (ret != SUCCESS) {
            IMAGE_LOGE("[ImageSource]failed to add context on incremental decoding, ret:%{public}u.", ret);
            return ret;
        }
    }
    if (incrementalRecordIter->second.IncrementalState == ImageDecodingState::BASE_INFO_PARSED) {
        IMAGE_LOGD("[ImageSource]promote decode : set decode options.");
        ImagePlugin::PlImageInfo plInfo;
        ret = SetDecodeOptions(incrementalRecordIter->second.decoder, index, opts, plInfo);
        if (ret != SUCCESS) {
            IMAGE_LOGE("[ImageSource]set decode options error (image index:%{public}u), ret:%{public}u.", index, ret);
            return ret;
        }

        auto iter = decodeEventMap_.find((int)DecodeEvent::EVENT_HEADER_DECODE);
        if (iter == decodeEventMap_.end()) {
            decodeEventMap_.insert(std::pair<int32_t, int32_t>((int)DecodeEvent::EVENT_HEADER_DECODE, 1));
            for (auto listener : decodeListeners_) {
                guard.unlock();
                listener->OnEvent((int)DecodeEvent::EVENT_HEADER_DECODE);
                guard.lock();
            }
        }
        ret = UpdatePixelMapInfo(opts, plInfo, pixelMap);
        if (ret != SUCCESS) {
            IMAGE_LOGE("[ImageSource]update pixelmap info error (image index:%{public}u), ret:%{public}u.", index, ret);
            return ret;
        }
        incrementalRecordIter->second.IncrementalState = ImageDecodingState::IMAGE_DECODING;
    }
    if (incrementalRecordIter->second.IncrementalState == ImageDecodingState::IMAGE_DECODING) {
        ret = DoIncrementalDecoding(index, opts, pixelMap, incrementalRecordIter->second);
        decodeProgress = incrementalRecordIter->second.decodingProgress;
        state = incrementalRecordIter->second.IncrementalState;
        if (isIncrementalCompleted_) {
            PostProc postProc;
            ret = postProc.DecodePostProc(opts, pixelMap);
            if (state == ImageDecodingState::IMAGE_DECODED) {
                auto iter = decodeEventMap_.find((int)DecodeEvent::EVENT_COMPLETE_DECODE);
                if (iter == decodeEventMap_.end()) {
                    decodeEventMap_.insert(std::pair<int32_t, int32_t>((int)DecodeEvent::EVENT_COMPLETE_DECODE, 1));
                    for (auto listener : decodeListeners_) {
                        guard.unlock();
                        listener->OnEvent((int)DecodeEvent::EVENT_COMPLETE_DECODE);
                        guard.lock();
                    }
                }
            }
        }
        return ret;
    }

    // IMAGE_ERROR or IMAGE_DECODED.
    state = incrementalRecordIter->second.IncrementalState;
    decodeProgress = incrementalRecordIter->second.decodingProgress;
    if (incrementalRecordIter->second.IncrementalState == ImageDecodingState::IMAGE_ERROR) {
        IMAGE_LOGE("[ImageSource]invalid imageState %{public}d on incremental decoding.",
                   incrementalRecordIter->second.IncrementalState);
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    return SUCCESS;
}

void ImageSource::DetachIncrementalDecoding(PixelMap &pixelMap)
{
    std::lock_guard<std::mutex> guard(decodingMutex_);
    auto iter = incDecodingMap_.find(&pixelMap);
    if (iter == incDecodingMap_.end()) {
        return;
    }

    if (mainDecoder_ == nullptr) {
        // return back the decoder to mainDecoder_.
        mainDecoder_ = std::move(iter->second.decoder);
        iter->second.decoder = nullptr;
    }
    incDecodingMap_.erase(iter);
}

uint32_t ImageSource::UpdateData(const uint8_t *data, uint32_t size, bool isCompleted)
{
    if (sourceStreamPtr_ == nullptr) {
        IMAGE_LOGE("[ImageSource]image source update data, source stream is null.");
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    std::lock_guard<std::mutex> guard(decodingMutex_);
    if (isCompleted) {
        isIncrementalCompleted_ = isCompleted;
    }
    return sourceStreamPtr_->UpdateData(data, size, isCompleted);
}

DecodeEvent ImageSource::GetDecodeEvent()
{
    return decodeEvent_;
}

uint32_t ImageSource::GetImageInfo(uint32_t index, ImageInfo &imageInfo)
{
    uint32_t ret = SUCCESS;
    std::unique_lock<std::mutex> guard(decodingMutex_);
    auto iter = GetValidImageStatus(index, ret);
    if (iter == imageStatusMap_.end()) {
        guard.unlock();
        IMAGE_LOGE("[ImageSource]get valid image status fail on get image info, ret:%{public}u.", ret);
        return ret;
    }
    ImageInfo &info = (iter->second).imageInfo;
    if (info.size.width == 0 || info.size.height == 0) {
        IMAGE_LOGE("[ImageSource]get the image size fail on get image info, width:%{public}d, height:%{public}d.",
                   info.size.width, info.size.height);
        return ERR_IMAGE_DECODE_FAILED;
    }

    imageInfo = info;
    return SUCCESS;
}

uint32_t ImageSource::GetImagePropertyInt(uint32_t index, const std::string &key, int32_t &value)
{
    std::unique_lock<std::mutex> guard(decodingMutex_);
    uint32_t ret;
    auto iter = GetValidImageStatus(0, ret);
    if (iter == imageStatusMap_.end()) {
        IMAGE_LOGE("[ImageSource]get valid image status fail on get image property, ret:%{public}u.", ret);
        return ret;
    }

    ret = mainDecoder_->GetImagePropertyInt(index, key, value);
    if (ret != SUCCESS) {
        IMAGE_LOGE("[ImageSource] GetImagePropertyInt fail, ret:%{public}u", ret);
        return ret;
    }

    return SUCCESS;
}

const SourceInfo &ImageSource::GetSourceInfo(uint32_t &errorCode)
{
    std::lock_guard<std::mutex> guard(decodingMutex_);
    errorCode = DecodeSourceInfo(true);
    return sourceInfo_;
}

void ImageSource::RegisterListener(PeerListener *listener)
{
    if (listener == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> guard(listenerMutex_);
    listeners_.insert(listener);
}

void ImageSource::UnRegisterListener(PeerListener *listener)
{
    if (listener == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> guard(listenerMutex_);
    auto iter = listeners_.find(listener);
    if (iter != listeners_.end()) {
        listeners_.erase(iter);
    }
}

void ImageSource::AddDecodeListener(DecodeListener *listener)
{
    if (listener == nullptr) {
        IMAGE_LOGE("AddDecodeListener listener null");
        return;
    }
    std::lock_guard<std::mutex> guard(listenerMutex_);
    decodeListeners_.insert(listener);
}

void ImageSource::RemoveDecodeListener(DecodeListener *listener)
{
    if (listener == nullptr) {
        IMAGE_LOGE("RemoveDecodeListener listener null");
        return;
    }
    std::lock_guard<std::mutex> guard(listenerMutex_);
    auto iter = decodeListeners_.find(listener);
    if (iter != decodeListeners_.end()) {
        decodeListeners_.erase(iter);
    }
}

ImageSource::~ImageSource()
{
    std::lock_guard<std::mutex> guard(listenerMutex_);
    for (const auto &listener : listeners_) {
        listener->OnPeerDestory();
    }
}

bool ImageSource::IsStreamCompleted()
{
    std::lock_guard<std::mutex> guard(decodingMutex_);
    return sourceStreamPtr_->IsStreamCompleted();
}

// ------------------------------- private method -------------------------------
ImageSource::ImageSource(unique_ptr<SourceStream> &&stream, const SourceOptions &opts)
    : sourceStreamPtr_(stream.release())
{
    sourceInfo_.encodedFormat = opts.formatHint;
    sourceInfo_.baseDensity = opts.baseDensity;
}

ImageSource::FormatAgentMap ImageSource::InitClass()
{
    vector<ClassInfo> classInfos;
    pluginServer_.PluginServerGetClassInfo<AbsImageFormatAgent>(AbsImageFormatAgent::SERVICE_DEFAULT, classInfos);
    set<string> formats;
    for (auto &info : classInfos) {
        auto &capabilities = info.capabilities;
        auto iter = capabilities.find(IMAGE_ENCODE_FORMAT);
        if (iter == capabilities.end()) {
            continue;
        }

        AttrData &attr = iter->second;
        string format;
        if (SUCCESS != attr.GetValue(format)) {
            IMAGE_LOGE("[ImageSource]attr data get format:[%{public}s] failed.", format.c_str());
            continue;
        }
        formats.insert(move(format));
    }

    FormatAgentMap tempAgentMap;
    AbsImageFormatAgent *formatAgent = nullptr;
    for (auto format : formats) {
        map<string, AttrData> capabilities = { { IMAGE_ENCODE_FORMAT, AttrData(format) } };
        formatAgent =
            pluginServer_.CreateObject<AbsImageFormatAgent>(AbsImageFormatAgent::SERVICE_DEFAULT, capabilities);
        if (formatAgent == nullptr) {
            continue;
        }
        tempAgentMap.insert(FormatAgentMap::value_type(std::move(format), formatAgent));
    }
    return tempAgentMap;
}

uint32_t ImageSource::CheckEncodedFormat(AbsImageFormatAgent &agent)
{
    uint32_t size = agent.GetHeaderSize();
    ImagePlugin::DataStreamBuffer outData;
    if (sourceStreamPtr_ == nullptr) {
        IMAGE_LOGE("[ImageSource]check image format, source stream is null.");
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    if (!sourceStreamPtr_->Peek(size, outData)) {
        IMAGE_LOGE("[ImageSource]stream peek the data fail.");
        return ERR_IMAGE_SOURCE_DATA;
    }

    if (outData.inputStreamBuffer == nullptr || outData.dataSize < size) {
        IMAGE_LOGE("[ImageSource]the ouData is incomplete.");
        return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
    }

    if (!agent.CheckFormat(outData.inputStreamBuffer, size)) {
        IMAGE_LOGE("[ImageSource]check mismatched format :%{public}s.", agent.GetFormatType().c_str());
        return ERR_IMAGE_MISMATCHED_FORMAT;
    }
    return SUCCESS;
}

uint32_t ImageSource::CheckFormatHint(const string &formatHint, FormatAgentMap::iterator &formatIter)
{
    uint32_t ret = ERROR;
    formatIter = formatAgentMap_.find(formatHint);
    if (formatIter == formatAgentMap_.end()) {
        IMAGE_LOGE("[ImageSource]check input format fail.");
        return ret;
    }
    AbsImageFormatAgent *agent = formatIter->second;
    ret = CheckEncodedFormat(*agent);
    if (ret != SUCCESS) {
        if (ret == ERR_IMAGE_SOURCE_DATA_INCOMPLETE) {
            IMAGE_LOGE("[ImageSource]image source incomplete.");
        }
        return ret;
    }
    return SUCCESS;
}

uint32_t ImageSource::GetEncodedFormat(const string &formatHint, string &format)
{
    bool streamIncomplete = false;
    auto hintIter = formatAgentMap_.end();
    if (!formatHint.empty()) {
        uint32_t ret = CheckFormatHint(formatHint, hintIter);
        if (ret == ERR_IMAGE_SOURCE_DATA) {
            IMAGE_LOGE("[ImageSource]image source data error.");
            return ret;
        } else if (ret == SUCCESS) {
            format = hintIter->first;
            IMAGE_LOGD("[ImageSource]check input image format success, format:%{public}s.", format.c_str());
            return SUCCESS;
        } else if (ret == ERR_IMAGE_SOURCE_DATA_INCOMPLETE) {
            streamIncomplete = true;
        }
    }

    for (auto iter = formatAgentMap_.begin(); iter != formatAgentMap_.end(); ++iter) {
        string curFormat = iter->first;
        if (iter == hintIter || curFormat == InnerFormat::RAW_FORMAT) {
            continue;  // has been checked before.
        }
        AbsImageFormatAgent *agent = iter->second;
        auto ret = CheckEncodedFormat(*agent);
        if (ret == ERR_IMAGE_MISMATCHED_FORMAT) {
            continue;
        } else if (ret == SUCCESS) {
            IMAGE_LOGI("[ImageSource]GetEncodedFormat success format :%{public}s.", iter->first.c_str());
            format = iter->first;
            return SUCCESS;
        } else if (ret == ERR_IMAGE_SOURCE_DATA_INCOMPLETE) {
            streamIncomplete = true;
        }
    }

    if (streamIncomplete) {
        IMAGE_LOGE("[ImageSource]image source incomplete.");
        return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
    }

    // default return raw image
    format = InnerFormat::RAW_FORMAT;
    IMAGE_LOGI("[ImageSource]image default to raw format.");
    return SUCCESS;
}

uint32_t ImageSource::OnSourceRecognized(bool isAcquiredImageNum)
{
    uint32_t ret = InitMainDecoder();
    if (ret != SUCCESS) {
        sourceInfo_.state = SourceInfoState::UNSUPPORTED_FORMAT;
        decodeState_ = SourceDecodingState::UNSUPPORTED_FORMAT;
        IMAGE_LOGE("[ImageSource]image decode error, ret:[%{public}u].", ret);
        return ret;
    }

    // for raw image, we need check the original format after decoder initialzation
    string value;
    ret = mainDecoder_->GetImagePropertyString(0, ACTUAL_IMAGE_ENCODED_FORMAT, value);
    if (ret == SUCCESS) {
        // update new format
        sourceInfo_.encodedFormat = value;
        IMAGE_LOGI("[ImageSource] update new format, value:%{public}s", value.c_str());
    } else {
        IMAGE_LOGD("[ImageSource] GetImagePropertyString fail, ret:%{public}u", ret);
    }

    if (isAcquiredImageNum) {
        ret = mainDecoder_->GetTopLevelImageNum(sourceInfo_.topLevelImageNum);
        if (ret != SUCCESS) {
            if (ret == ERR_IMAGE_SOURCE_DATA_INCOMPLETE) {
                sourceInfo_.state = SourceInfoState::SOURCE_INCOMPLETE;
                IMAGE_LOGE("[ImageSource]image source data incomplete.");
                return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
            }
            sourceInfo_.state = SourceInfoState::FILE_INFO_ERROR;
            decodeState_ = SourceDecodingState::FILE_INFO_ERROR;
            IMAGE_LOGE("[ImageSource]image source error.");
            return ret;
        }
    }
    sourceInfo_.state = SourceInfoState::FILE_INFO_PARSED;
    decodeState_ = SourceDecodingState::FILE_INFO_DECODED;
    return SUCCESS;
}

uint32_t ImageSource::OnSourceUnresolved()
{
    string formatResult;
    auto ret = GetEncodedFormat(sourceInfo_.encodedFormat, formatResult);
    if (ret != SUCCESS) {
        if (ret == ERR_IMAGE_SOURCE_DATA_INCOMPLETE) {
            IMAGE_LOGE("[ImageSource]image source incomplete.");
            sourceInfo_.state = SourceInfoState::SOURCE_INCOMPLETE;
            return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
        } else if (ret == ERR_IMAGE_UNKNOWN_FORMAT) {
            IMAGE_LOGE("[ImageSource]image unknown format.");
            sourceInfo_.state = SourceInfoState::UNKNOWN_FORMAT;
            decodeState_ = SourceDecodingState::UNKNOWN_FORMAT;
            return ERR_IMAGE_UNKNOWN_FORMAT;
        }
        sourceInfo_.state = SourceInfoState::SOURCE_ERROR;
        decodeState_ = SourceDecodingState::SOURCE_ERROR;
        IMAGE_LOGE("[ImageSource]image source error.");
        return ret;
    }
    sourceInfo_.encodedFormat = formatResult;
    decodeState_ = SourceDecodingState::FORMAT_RECOGNIZED;
    return SUCCESS;
}

uint32_t ImageSource::DecodeSourceInfo(bool isAcquiredImageNum)
{
    uint32_t ret = SUCCESS;
    if (decodeState_ >= SourceDecodingState::FILE_INFO_DECODED) {
        if (isAcquiredImageNum) {
            decodeState_ = SourceDecodingState::FORMAT_RECOGNIZED;
        } else {
            return SUCCESS;
        }
    }
    if (decodeState_ == SourceDecodingState::UNRESOLVED) {
        ret = OnSourceUnresolved();
        if (ret != SUCCESS) {
            IMAGE_LOGE("[ImageSource]unresolved source: check format failed, ret:[%{public}d].", ret);
            return ret;
        }
    }
    if (decodeState_ == SourceDecodingState::FORMAT_RECOGNIZED) {
        ret = OnSourceRecognized(isAcquiredImageNum);
        if (ret != SUCCESS) {
            IMAGE_LOGE("[ImageSource]recognized source: get source info failed, ret:[%{public}d].", ret);
            return ret;
        }
        return SUCCESS;
    }
    IMAGE_LOGE("[ImageSource]invalid source state %{public}d on decode source info.", decodeState_);
    switch (decodeState_) {
        case SourceDecodingState::SOURCE_ERROR: {
            ret = ERR_IMAGE_SOURCE_DATA;
            break;
        }
        case SourceDecodingState::UNKNOWN_FORMAT: {
            ret = ERR_IMAGE_UNKNOWN_FORMAT;
            break;
        }
        case SourceDecodingState::UNSUPPORTED_FORMAT: {
            ret = ERR_IMAGE_PLUGIN_CREATE_FAILED;
            break;
        }
        case SourceDecodingState::FILE_INFO_ERROR: {
            ret = ERR_IMAGE_DECODE_FAILED;
            break;
        }
        default: {
            ret = ERROR;
            break;
        }
    }
    return ret;
}

uint32_t ImageSource::DecodeImageInfo(uint32_t index, ImageStatusMap::iterator &iter)
{
    uint32_t ret = DecodeSourceInfo(false);
    if (ret != SUCCESS) {
        IMAGE_LOGE("[ImageSource]decode the image fail, ret:%{public}d.", ret);
        return ret;
    }
    if (mainDecoder_ == nullptr) {
        IMAGE_LOGE("[ImageSource]get image size, image decode plugin is null.");
        return ERR_IMAGE_PLUGIN_CREATE_FAILED;
    }
    ImagePlugin::PlSize size;
    ret = mainDecoder_->GetImageSize(index, size);
    if (ret == SUCCESS) {
        ImageDecodingStatus imageStatus;
        imageStatus.imageInfo.size.width = size.width;
        imageStatus.imageInfo.size.height = size.height;
        imageStatus.imageState = ImageDecodingState::BASE_INFO_PARSED;
        auto result = imageStatusMap_.insert(ImageStatusMap::value_type(index, imageStatus));
        iter = result.first;
        return SUCCESS;
    } else if (ret == ERR_IMAGE_SOURCE_DATA_INCOMPLETE) {
        IMAGE_LOGE("[ImageSource]source data incomplete.");
        return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
    } else {
        ImageDecodingStatus imageStatus;
        imageStatus.imageState = ImageDecodingState::BASE_INFO_ERROR;
        auto result = imageStatusMap_.insert(ImageStatusMap::value_type(index, imageStatus));
        iter = result.first;
        IMAGE_LOGE("[ImageSource]decode the image info fail.");
        return ERR_IMAGE_DECODE_FAILED;
    }
}

uint32_t ImageSource::InitMainDecoder()
{
    if (mainDecoder_ != nullptr) {
        return SUCCESS;
    }
    uint32_t result = SUCCESS;
    mainDecoder_ = std::unique_ptr<ImagePlugin::AbsImageDecoder>(CreateDecoder(result));
    return result;
}

AbsImageDecoder *ImageSource::CreateDecoder(uint32_t &errorCode)
{
    // in normal mode, we can get actual encoded format to the user
    // but we need transfer to skia codec for adaption, "image/x-skia"
    std::string encodedFormat = sourceInfo_.encodedFormat;
    if (opts_.sampleSize != 1) {
        encodedFormat = InnerFormat::EXTENDED_FORMAT;
    }
    map<string, AttrData> capabilities = { { IMAGE_ENCODE_FORMAT, AttrData(encodedFormat) } };
    auto decoder = pluginServer_.CreateObject<AbsImageDecoder>(AbsImageDecoder::SERVICE_DEFAULT, capabilities);
    if (decoder == nullptr) {
        IMAGE_LOGE("[ImageSource]failed to create decoder object.");
        errorCode = ERR_IMAGE_PLUGIN_CREATE_FAILED;
        return nullptr;
    }
    errorCode = SUCCESS;
    decoder->SetSource(*sourceStreamPtr_);
    return decoder;
}

uint32_t ImageSource::SetDecodeOptions(std::unique_ptr<AbsImageDecoder> &decoder, uint32_t index,
                                       const DecodeOptions &opts, ImagePlugin::PlImageInfo &plInfo)
{
    PixelDecodeOptions plOptions;
    CopyOptionsToPlugin(opts, plOptions);
    uint32_t ret = decoder->SetDecodeOptions(index, plOptions, plInfo);
    if (ret != SUCCESS) {
        IMAGE_LOGE("[ImageSource]decoder plugin set decode options fail (image index:%{public}u), ret:%{public}u.",
                   index, ret);
        return ret;
    }
    auto iter = imageStatusMap_.find(index);
    if (iter != imageStatusMap_.end()) {
        ImageInfo &info = (iter->second).imageInfo;
        IMAGE_LOGD("[ImageSource]SetDecodeOptions plInfo.pixelFormat %{public}d", plInfo.pixelFormat);

        PlPixelFormat format = plInfo.pixelFormat;
        auto find_item = std::find_if(PIXEL_FORMAT_MAP.begin(), PIXEL_FORMAT_MAP.end(),
            [format](const std::map<PixelFormat, PlPixelFormat>::value_type item) {
            return item.second == format;
        });
        if (find_item != PIXEL_FORMAT_MAP.end()) {
            info.pixelFormat = (*find_item).first;
        }
        IMAGE_LOGD("[ImageSource]SetDecodeOptions info.pixelFormat %{public}d", info.pixelFormat);
    }
    return SUCCESS;
}

uint32_t ImageSource::UpdatePixelMapInfo(const DecodeOptions &opts, ImagePlugin::PlImageInfo &plInfo,
                                         PixelMap &pixelMap)
{
    pixelMap.SetEditable(opts.editable);

    ImageInfo info;
    info.baseDensity = sourceInfo_.baseDensity;
    info.size.width = plInfo.size.width;
    info.size.height = plInfo.size.height;
    info.pixelFormat = static_cast<PixelFormat>(plInfo.pixelFormat);
    info.alphaType = static_cast<AlphaType>(plInfo.alphaType);
    return pixelMap.SetImageInfo(info);
}

void ImageSource::CopyOptionsToPlugin(const DecodeOptions &opts, PixelDecodeOptions &plOpts)
{
    plOpts.CropRect.left = opts.CropRect.left;
    plOpts.CropRect.top = opts.CropRect.top;
    plOpts.CropRect.width = opts.CropRect.width;
    plOpts.CropRect.height = opts.CropRect.height;
    plOpts.desiredSize.width = opts.desiredSize.width;
    plOpts.desiredSize.height = opts.desiredSize.height;
    plOpts.rotateDegrees = opts.rotateDegrees;
    plOpts.sampleSize = opts.sampleSize;
    auto formatSearch = PIXEL_FORMAT_MAP.find(opts.desiredPixelFormat);
    plOpts.desiredPixelFormat =
        (formatSearch != PIXEL_FORMAT_MAP.end()) ? formatSearch->second : PlPixelFormat::RGBA_8888;
    auto colorSearch = COLOR_SPACE_MAP.find(opts.desiredColorSpace);
    plOpts.desiredColorSpace = (colorSearch != COLOR_SPACE_MAP.end()) ? colorSearch->second : PlColorSpace::UNKNOWN;
    plOpts.allowPartialImage = opts.allowPartialImage;
    plOpts.editable = opts.editable;
}

void ImageSource::CopyOptionsToProcOpts(const DecodeOptions &opts, DecodeOptions &procOpts, PixelMap &pixelMap)
{
    procOpts.fitDensity = opts.fitDensity;
    procOpts.CropRect.left = opts.CropRect.left;
    procOpts.CropRect.top = opts.CropRect.top;
    procOpts.CropRect.width = opts.CropRect.width;
    procOpts.CropRect.height = opts.CropRect.height;
    procOpts.desiredSize.width = opts.desiredSize.width;
    procOpts.desiredSize.height = opts.desiredSize.height;
    procOpts.rotateDegrees = opts.rotateDegrees;
    procOpts.sampleSize = opts.sampleSize;
    procOpts.desiredPixelFormat = opts.desiredPixelFormat;
    if (opts.allocatorType == AllocatorType::DEFAULT) {
        procOpts.allocatorType = AllocatorType::HEAP_ALLOC;
    } else {
        procOpts.allocatorType = opts.allocatorType;
    }
    procOpts.desiredColorSpace = opts.desiredColorSpace;
    procOpts.allowPartialImage = opts.allowPartialImage;
    procOpts.editable = opts.editable;
    // we need preference_ when post processing
    procOpts.preference = preference_;
}

ImageSource::ImageStatusMap::iterator ImageSource::GetValidImageStatus(uint32_t index, uint32_t &errorCode)
{
    auto iter = imageStatusMap_.find(index);
    if (iter == imageStatusMap_.end()) {
        errorCode = DecodeImageInfo(index, iter);
        if (errorCode != SUCCESS) {
            IMAGE_LOGE("[ImageSource]image info decode fail, ret:%{public}u.", errorCode);
            return imageStatusMap_.end();
        }
    } else if (iter->second.imageState < ImageDecodingState::BASE_INFO_PARSED) {
        IMAGE_LOGE("[ImageSource]invalid imageState %{public}d on get image status.", iter->second.imageState);
        errorCode = ERR_IMAGE_DECODE_FAILED;
        return imageStatusMap_.end();
    }
    errorCode = SUCCESS;
    return iter;
}

uint32_t ImageSource::AddIncrementalContext(PixelMap &pixelMap, IncrementalRecordMap::iterator &iterator)
{
    uint32_t ret = SUCCESS;
    IncrementalDecodingContext context;
    if (mainDecoder_ != nullptr) {
        // borrowed decoder from the mainDecoder_.
        context.decoder = std::move(mainDecoder_);
    } else {
        context.decoder = std::unique_ptr<ImagePlugin::AbsImageDecoder>(CreateDecoder(ret));
    }
    if (context.decoder == nullptr) {
        IMAGE_LOGE("[ImageSource]failed to create decoder on add incremental context, ret:%{public}u.", ret);
        return ret;
    }
    // mainDecoder has parsed base info in DecodeImageInfo();
    context.IncrementalState = ImageDecodingState::BASE_INFO_PARSED;
    auto result = incDecodingMap_.insert(IncrementalRecordMap::value_type(&pixelMap, std::move(context)));
    iterator = result.first;
    return SUCCESS;
}

uint32_t ImageSource::DoIncrementalDecoding(uint32_t index, const DecodeOptions &opts, PixelMap &pixelMap,
                                            IncrementalDecodingContext &recordContext)
{
    IMAGE_LOGD("[ImageSource]do incremental decoding: begin.");
    uint8_t *pixelAddr = static_cast<uint8_t *>(pixelMap.GetWritablePixels());
    ProgDecodeContext context;
    context.decodeContext.pixelsBuffer.buffer = pixelAddr;
    uint32_t ret = recordContext.decoder->PromoteIncrementalDecode(index, context);
    if (context.decodeContext.pixelsBuffer.buffer != nullptr && pixelAddr == nullptr) {
        pixelMap.SetPixelsAddr(context.decodeContext.pixelsBuffer.buffer, context.decodeContext.pixelsBuffer.context,
                               context.decodeContext.pixelsBuffer.bufferSize, context.decodeContext.allocatorType,
                               context.decodeContext.freeFunc);
    }
    IMAGE_LOGD("[ImageSource]do incremental decoding progress:%{public}u.", context.totalProcessProgress);
    recordContext.decodingProgress = context.totalProcessProgress;
    if (ret != SUCCESS && ret != ERR_IMAGE_SOURCE_DATA_INCOMPLETE) {
        recordContext.IncrementalState = ImageDecodingState::IMAGE_ERROR;
        IMAGE_LOGE("[ImageSource]do incremental decoding source fail, ret:%{public}u.", ret);
        return ret;
    }
    if (ret == SUCCESS) {
        recordContext.IncrementalState = ImageDecodingState::IMAGE_DECODED;
        IMAGE_LOGI("[ImageSource]do incremental decoding success.");
    }
    return ret;
}

const NinePatchInfo &ImageSource::GetNinePatchInfo() const
{
    return ninePatchInfo_;
}

void ImageSource::SetMemoryUsagePreference(const MemoryUsagePreference preference)
{
    preference_ = preference;
}

MemoryUsagePreference ImageSource::GetMemoryUsagePreference()
{
    return preference_;
}

void ImageSource::SetIncrementalSource(const bool isIncrementalSource)
{
    isIncrementalSource_ = isIncrementalSource;
}

bool ImageSource::IsIncrementalSource()
{
    return isIncrementalSource_;
}

FinalOutputStep ImageSource::GetFinalOutputStep(const DecodeOptions &opts, PixelMap &pixelMap, bool hasNinePatch)
{
    ImageInfo info;
    pixelMap.GetImageInfo(info);
    ImageInfo dstImageInfo;
    dstImageInfo.size = opts.desiredSize;
    dstImageInfo.pixelFormat = opts.desiredPixelFormat;
    if (opts.desiredPixelFormat == PixelFormat::UNKNOWN) {
        if (preference_ == MemoryUsagePreference::LOW_RAM && info.alphaType == AlphaType::IMAGE_ALPHA_TYPE_OPAQUE) {
            dstImageInfo.pixelFormat = PixelFormat::RGB_565;
        } else {
            dstImageInfo.pixelFormat = PixelFormat::RGBA_8888;
        }
    }
    // decode use, this value may be changed by real pixelFormat
    if (pixelMap.GetAlphaType() == AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL) {
        dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    } else {
        dstImageInfo.alphaType = pixelMap.GetAlphaType();
    }
    bool densityChange = HasDensityChange(opts, info, hasNinePatch);
    bool sizeChange = ImageSizeChange(pixelMap.GetWidth(), pixelMap.GetHeight(),
                                      opts.desiredSize.width, opts.desiredSize.height);
    bool rotateChange = !ImageUtils::FloatCompareZero(opts.rotateDegrees);
    bool convertChange = ImageConverChange(opts.CropRect, dstImageInfo, info);
    if (sizeChange) {
        return FinalOutputStep::SIZE_CHANGE;
    } else if (densityChange) {
        return FinalOutputStep::DENSITY_CHANGE;
    }
    if (rotateChange) {
        return FinalOutputStep::ROTATE_CHANGE;
    }
    if (convertChange) {
        return FinalOutputStep::CONVERT_CHANGE;
    }
    return FinalOutputStep::NO_CHANGE;
}

bool ImageSource::HasDensityChange(const DecodeOptions &opts, ImageInfo &srcImageInfo, bool hasNinePatch)
{
    return !hasNinePatch && (srcImageInfo.baseDensity > 0) &&
           (opts.fitDensity > 0) && (srcImageInfo.baseDensity != opts.fitDensity);
}

bool ImageSource::ImageSizeChange(int32_t width, int32_t height, int32_t desiredWidth, int32_t desiredHeight)
{
    bool sizeChange = false;
    if (desiredWidth > 0 && desiredHeight > 0 && width > 0 && height > 0) {
        float scaleX = static_cast<float>(desiredWidth) / static_cast<float>(width);
        float scaleY = static_cast<float>(desiredHeight) / static_cast<float>(height);
        if ((fabs(scaleX - 1.0f) >= EPSILON) && (fabs(scaleY - 1.0f) >= EPSILON)) {
            sizeChange = true;
        }
    }
    return sizeChange;
}

bool ImageSource::ImageConverChange(const Rect &cropRect, ImageInfo &dstImageInfo, ImageInfo &srcImageInfo)
{
    bool hasPixelConvert = false;
    dstImageInfo.alphaType = ImageUtils::GetValidAlphaTypeByFormat(dstImageInfo.alphaType, dstImageInfo.pixelFormat);
    if (dstImageInfo.pixelFormat != srcImageInfo.pixelFormat || dstImageInfo.alphaType != srcImageInfo.alphaType) {
        hasPixelConvert = true;
    }
    CropValue value = PostProc::GetCropValue(cropRect, srcImageInfo.size);
    if (value == CropValue::NOCROP && !hasPixelConvert) {
        IMAGE_LOGD("[ImageSource]no need crop and pixel convert.");
        return false;
    } else if (value == CropValue::INVALID) {
        IMAGE_LOGE("[ImageSource]invalid corp region, top:%{public}d, left:%{public}d, "
                   "width:%{public}d, height:%{public}d",
                   cropRect.top, cropRect.left, cropRect.width, cropRect.height);
        return false;
    }
    return true;
}
} // namespace Media
} // namespace OHOS
