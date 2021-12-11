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

#include "image_packer.h"

#include "buffer_packer_stream.h"
#include "file_packer_stream.h"
#include "image/abs_image_encoder.h"
#include "image_utils.h"
#include "log_tags.h"
#include "media_errors.h"
#include "ostream_packer_stream.h"
#include "plugin_server.h"

namespace OHOS {
namespace Media {
using namespace OHOS::HiviewDFX;
using namespace ImagePlugin;
using namespace MultimediaPlugin;
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "ImagePacker" };
static constexpr uint8_t QUALITY_MAX = 100;

PluginServer &ImagePacker::pluginServer_ = ImageUtils::GetPluginServer();

uint32_t ImagePacker::GetSupportedFormats(std::set<std::string> &formats)
{
    formats.clear();
    std::vector<ClassInfo> classInfos;
    uint32_t ret =
        pluginServer_.PluginServerGetClassInfo<AbsImageEncoder>(AbsImageEncoder::SERVICE_DEFAULT, classInfos);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "get class info from plugin server,ret:%{public}u.", ret);
        return ret;
    }
    for (auto &info : classInfos) {
        std::map<std::string, AttrData> &capbility = info.capabilities;
        auto iter = capbility.find(IMAGE_ENCODE_FORMAT);
        if (iter == capbility.end()) {
            continue;
        }
        AttrData &attr = iter->second;
        std::string format;
        if (attr.GetValue(format) != SUCCESS) {
            HiLog::Error(LABEL, "attr data get format failed.");
            continue;
        }
        formats.insert(format);
    }
    return SUCCESS;
}

uint32_t ImagePacker::StartPackingImpl(const PackOption &option)
{
    if (!GetEncoderPlugin(option)) {
        HiLog::Error(LABEL, "StartPackingImpl get encoder plugin failed.");
        return ERR_IMAGE_MISMATCHED_FORMAT;
    }
    PlEncodeOptions plOpts;
    CopyOptionsToPlugin(option, plOpts);
    return encoder_->StartEncode(*packerStream_.get(), plOpts);
}

uint32_t ImagePacker::StartPacking(uint8_t *outputData, uint32_t maxSize, const PackOption &option)
{
    if (!IsPackOptionValid(option)) {
        HiLog::Error(LABEL, "array startPacking option invalid %{public}s, %{public}u.", option.format.c_str(),
                     option.quality);
        return ERR_IMAGE_INVALID_PARAMETER;
    }

    if (outputData == nullptr) {
        HiLog::Error(LABEL, "output buffer is null.");
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    BufferPackerStream *stream = new (std::nothrow) BufferPackerStream(outputData, maxSize);
    if (stream == nullptr) {
        HiLog::Error(LABEL, "make buffer packer stream failed.");
        return ERR_IMAGE_DATA_ABNORMAL;
    }
    FreeOldPackerStream();
    packerStream_ = std::unique_ptr<BufferPackerStream>(stream);
    return StartPackingImpl(option);
}

uint32_t ImagePacker::StartPacking(const std::string &filePath, const PackOption &option)
{
    if (!IsPackOptionValid(option)) {
        HiLog::Error(LABEL, "filepath startPacking option invalid %{public}s, %{public}u.", option.format.c_str(),
                     option.quality);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    FilePackerStream *stream = new (std::nothrow) FilePackerStream(filePath);
    if (stream == nullptr) {
        HiLog::Error(LABEL, "make file packer stream failed.");
        return ERR_IMAGE_DATA_ABNORMAL;
    }
    FreeOldPackerStream();
    packerStream_ = std::unique_ptr<FilePackerStream>(stream);
    return StartPackingImpl(option);
}

uint32_t ImagePacker::StartPacking(std::ostream &outputStream, const PackOption &option)
{
    if (!IsPackOptionValid(option)) {
        HiLog::Error(LABEL, "outputStream startPacking option invalid %{public}s, %{public}u.", option.format.c_str(),
                     option.quality);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    OstreamPackerStream *stream = new (std::nothrow) OstreamPackerStream(outputStream);
    if (stream == nullptr) {
        HiLog::Error(LABEL, "make ostream packer stream failed.");
        return ERR_IMAGE_DATA_ABNORMAL;
    }
    FreeOldPackerStream();
    packerStream_ = std::unique_ptr<OstreamPackerStream>(stream);
    return StartPackingImpl(option);
}

// JNI adapter method, this method be called by jni and the outputStream be created by jni, here we manage the lifecycle
// of the outputStream
uint32_t ImagePacker::StartPackingAdapter(PackerStream &outputStream, const PackOption &option)
{
    FreeOldPackerStream();
    packerStream_ = std::unique_ptr<PackerStream>(&outputStream);

    if (!IsPackOptionValid(option)) {
        HiLog::Error(LABEL, "packer stream option invalid %{public}s, %{public}u.", option.format.c_str(),
                     option.quality);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    return StartPackingImpl(option);
}

uint32_t ImagePacker::AddImage(PixelMap &pixelMap)
{
    if (encoder_ == nullptr) {
        HiLog::Error(LABEL, "AddImage get encoder plugin failed.");
        return ERR_IMAGE_MISMATCHED_FORMAT;
    }
    return encoder_->AddImage(pixelMap);
}

uint32_t ImagePacker::AddImage(ImageSource &source)
{
    DecodeOptions opts;
    uint32_t ret = SUCCESS;
    if (pixelMap_ != nullptr) {
        pixelMap_.reset();  // release old inner pixelmap
    }
    pixelMap_ = source.CreatePixelMap(opts, ret);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "image source create pixel map failed.");
        return ret;
    }
    return AddImage(*pixelMap_.get());
}

uint32_t ImagePacker::AddImage(ImageSource &source, uint32_t index)
{
    DecodeOptions opts;
    uint32_t ret = SUCCESS;
    if (pixelMap_ != nullptr) {
        pixelMap_.reset();  // release old inner pixelmap
    }
    pixelMap_ = source.CreatePixelMap(index, opts, ret);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "image source create pixel map failed.");
        return ret;
    }
    return AddImage(*pixelMap_.get());
}

uint32_t ImagePacker::FinalizePacking()
{
    if (encoder_ == nullptr) {
        HiLog::Error(LABEL, "FinalizePacking get encoder plugin failed.");
        return ERR_IMAGE_MISMATCHED_FORMAT;
    }
    return encoder_->FinalizeEncode();
}

uint32_t ImagePacker::FinalizePacking(int64_t &packedSize)
{
    uint32_t ret = FinalizePacking();
    packedSize = (packerStream_ != nullptr) ? packerStream_->BytesWritten() : 0;
    return ret;
}

bool ImagePacker::GetEncoderPlugin(const PackOption &option)
{
    std::map<std::string, AttrData> capabilities;
    capabilities.insert(std::map<std::string, AttrData>::value_type(IMAGE_ENCODE_FORMAT, AttrData(option.format)));
    if (encoder_ != nullptr) {
        encoder_.reset();
    }
    encoder_ = std::unique_ptr<ImagePlugin::AbsImageEncoder>(
        pluginServer_.CreateObject<AbsImageEncoder>(AbsImageEncoder::SERVICE_DEFAULT, capabilities));
    return (encoder_ != nullptr);
}

void ImagePacker::CopyOptionsToPlugin(const PackOption &opts, PlEncodeOptions &plOpts)
{
    plOpts.numberHint = opts.numberHint;
    plOpts.quality = opts.quality;
}

void ImagePacker::FreeOldPackerStream()
{
    if (packerStream_ != nullptr) {
        packerStream_.reset();
    }
}

bool ImagePacker::IsPackOptionValid(const PackOption &option)
{
    if (option.quality > QUALITY_MAX || option.format.empty()) {
        return false;
    }
    return true;
}

// class reference need explicit constructor and destructor, otherwise unique_ptr<T> use unnormal
ImagePacker::ImagePacker()
{}

ImagePacker::~ImagePacker()
{}
} // namespace Media
} // namespace OHOS