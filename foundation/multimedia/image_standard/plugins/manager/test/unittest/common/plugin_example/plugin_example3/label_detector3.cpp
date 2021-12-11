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

#include "label_detector3.h"
#include "hilog/log.h"
#include "log_tags.h"
#include "plugin_utils.h"

namespace OHOS {
namespace PluginExample {
using std::string;
using namespace OHOS::HiviewDFX;

static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "LabelDetector3" };
const string LabelDetector3::RESULT_STR = "LabelDetector3";

LabelDetector3::LabelDetector3()
{
    HiLog::Debug(LABEL, "call LabelDetector3().");
}

LabelDetector3::~LabelDetector3()
{
    HiLog::Debug(LABEL, "call ~LabelDetector3().");
}

bool LabelDetector3::Prepare()
{
    HiLog::Debug(LABEL, "call Prepare().");
    return true;
}

string LabelDetector3::Process()
{
    HiLog::Debug(LABEL, "call Process().");
    return RESULT_STR;
}
} // namespace PluginExample
} // namespace OHOS
