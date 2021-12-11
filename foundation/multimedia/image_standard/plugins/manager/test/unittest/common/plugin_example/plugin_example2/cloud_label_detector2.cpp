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

#include "cloud_label_detector2.h"
#include "hilog/log.h"
#include "log_tags.h"
#include "plugin_utils.h"

namespace OHOS {
namespace PluginExample {
using std::string;
using namespace OHOS::HiviewDFX;

static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "CloudLabelDetector2" };
const string CloudLabelDetector2::RESULT_STR = "CloudLabelDetector2";

CloudLabelDetector2::CloudLabelDetector2()
{
    HiLog::Debug(LABEL, "call CloudLabelDetector2().");
}

CloudLabelDetector2::~CloudLabelDetector2()
{
    HiLog::Debug(LABEL, "call ~CloudLabelDetector2().");
}

bool CloudLabelDetector2::Prepare()
{
    HiLog::Debug(LABEL, "call Prepare().");
    return true;
}

string CloudLabelDetector2::Process()
{
    HiLog::Debug(LABEL, "call Process().");
    return RESULT_STR;
}
} // namespace PluginExample
} // namespace OHOS
