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

#include "heif_decoder_wrapper.h"
#ifdef DUAL_ADAPTER
#include "heif_decoder_adapter.h"
#endif

namespace OHOS {
namespace ImagePlugin {
std::unique_ptr<HeifDecoderInterface> HeifDecoderWrapper::CreateHeifDecoderInterface(InputDataStream &stream)
{
#ifdef DUAL_ADAPTER
    return Media::HeifDecoderAdapter::MakeFromStream(stream);
#else
    return nullptr;
#endif
}
} // namespace ImagePlugin
};  // namespace OHOS