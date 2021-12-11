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


#ifndef IMAGE_SOURCE_UTIL_H
#define IMAGE_SOURCE_UTIL_H
#include <fstream>
#include "pixel_map.h"

int64_t PackImage(const std::string &filePath, std::unique_ptr<OHOS::Media::PixelMap> pixelMap);
bool ReadFileToBuffer(const std::string &filePath, uint8_t *buffer, size_t bufferSize);

#endif // IMAGE_SOURCE_UTIL_H