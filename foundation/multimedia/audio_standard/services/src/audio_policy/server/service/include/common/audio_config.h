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

#ifndef ST_AUDIO_CONFIG_H
#define ST_AUDIO_CONFIG_H

#include <string>

#include "audio_info.h"

#include <libxml/tree.h>
#include <libxml/parser.h>

namespace OHOS {
namespace AudioStandard {
enum NodeName {
    MODULES,
    MODULE,
    BUILT_IN_DEVICES,
    DEFAULT_OUTPUT_DEVICE,
    DEFAULT_INPUT_DEVICE,
    AUDIO_PORTS,
    AUDIO_PORT,
    AUDIO_PORT_PINS,
    AUDIO_PORT_PIN,
    UNKNOWN
};

enum PortType {
    TYPE_AUDIO_PORT,
    TYPE_AUDIO_PORT_PIN,
    TYPE_AUDIO_PORT_INVALID
};

class PortInfo {
public:
    PortType type;
    char *name;
    char *role;
    char *rate;
    char *channels;
    char *buffer_size;
    char *fileName;

    PortInfo()
        : type(TYPE_AUDIO_PORT_INVALID),
          name (nullptr),
          role (nullptr),
          rate (nullptr),
          channels (nullptr),
          buffer_size (nullptr),
          fileName (nullptr)
    {
    }

    ~PortInfo()
    {
        if (name != nullptr)
            xmlFree(reinterpret_cast<xmlChar*>(name));
        if (role != nullptr)
            xmlFree(reinterpret_cast<xmlChar*>(role));
        if (rate != nullptr)
            xmlFree(reinterpret_cast<xmlChar*>(rate));
        if (channels != nullptr)
            xmlFree(reinterpret_cast<xmlChar*>(channels));
        if (buffer_size != nullptr)
            xmlFree(reinterpret_cast<xmlChar*>(buffer_size));
        if (fileName != nullptr)
            xmlFree(reinterpret_cast<xmlChar*>(fileName));
    }
};

class AudioPortInfo : public PortInfo {
public:
    AudioPortInfo() {}

    virtual ~AudioPortInfo() {}
};

struct AudioPortPinInfo : public PortInfo {
public:
    char *pinType;
    AudioPortPinInfo()
        : pinType(nullptr)
    {
    }

    virtual ~AudioPortPinInfo() {}
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_CONFIG_H
