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
#include "image_trace.h"
#include "bytrace.h"
#include "securec.h"

namespace OHOS {
namespace Media {
static constexpr int32_t FORMAT_BUF_SIZE = 128;

ImageTrace::ImageTrace(const std::string &title) : title_(title)
{
    StartTrace(BYTRACE_TAG_ZIMAGE, title);
}

ImageTrace::~ImageTrace()
{
    FinishTrace(BYTRACE_TAG_ZIMAGE, title_);
}

ImageTrace::ImageTrace(const char *fmt, ...)
{
    if (fmt == nullptr) {
        title_ = "ImageTraceFmt Param invalid";
    } else {
        char buf[FORMAT_BUF_SIZE] = { 0 };
        va_list args;
        va_start(args, fmt);
        int32_t ret = vsprintf_s(buf, FORMAT_BUF_SIZE, fmt, args);
        va_end(args);
        if (ret != -1) {
            title_ = buf;
        } else {
            title_ = "ImageTraceFmt Format Error";
        }
    }
    StartTrace(BYTRACE_TAG_ZIMAGE, title_);
}
} // namespace Media
} // namespace OHOS