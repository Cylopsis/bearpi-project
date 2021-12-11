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

#include "plugin_class_base.h"
#include "hilog/log.h"
#include "log_tags.h"
#include "abs_impl_class_key.h"

namespace OHOS {
namespace MultimediaPlugin {
using namespace OHOS::HiviewDFX;

static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "PluginClassBase" };

PluginClassBase::~PluginClassBase()
{
    // this situation does not happen in design.
    // the process context can guarantee that this will not happen.
    // the judgment statement here is for protection and positioning purposes only.
    if (implClassKey_ == nullptr) {
        HiLog::Error(LABEL, "release class base, null implClassKey.");
        return;
    }

    implClassKey_->OnObjectDestroy();
}

// ------------------------------- private method -------------------------------
uint32_t PluginClassBase::SetImplClassKey(AbsImplClassKey &key)
{
    implClassKey_ = &key;
    return MAGIC_CODE;
}
} // namespace MultimediaPlugin
} // namespace OHOS
