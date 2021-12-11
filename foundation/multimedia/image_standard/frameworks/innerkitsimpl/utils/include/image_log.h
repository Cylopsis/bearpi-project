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

#ifndef IMAGE_LOG_H
#define IMAGE_LOG_H

#include "hilog/log.h"
#include "log_tags.h"

static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "ImageCode" };

#define IMAGE_LOGF(...) (void)OHOS::HiviewDFX::HiLog::Fatal(LABEL, __VA_ARGS__)
#define IMAGE_LOGE(...) (void)OHOS::HiviewDFX::HiLog::Error(LABEL, __VA_ARGS__)
#define IMAGE_LOGW(...) (void)OHOS::HiviewDFX::HiLog::Warn(LABEL, __VA_ARGS__)
#define IMAGE_LOGI(...) (void)OHOS::HiviewDFX::HiLog::Info(LABEL, __VA_ARGS__)
#define IMAGE_LOGD(...) (void)OHOS::HiviewDFX::HiLog::Debug(LABEL, __VA_ARGS__)

#endif // IMAGE_LOG_H
