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

#ifndef ST_XML_PARSER_H
#define ST_XML_PARSER_H

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "audio_config.h"
#include "iport_observer.h"
#include "parser.h"

namespace OHOS {
namespace AudioStandard {
class XMLParser : public Parser {
public:
    static constexpr char CONFIG_FILE[] = "/etc/audio/audio_policy_config.xml";

    bool LoadConfiguration() final;
    bool Parse() final;
    void Destroy() final;

    explicit XMLParser(IPortObserver &observer)
        : mPortObserver(observer),
          mDoc(nullptr)
    {
    }

    virtual ~XMLParser()
    {
        Destroy();
    }
private:
    bool ParseInternal(xmlNode &node);
    NodeName GetNodeNameAsInt(xmlNode &node);
    void ParseBuiltInDevices(xmlNode &node);
    void ParseDefaultOutputDevice(xmlNode &node);
    void ParseDefaultInputDevice(xmlNode &node);
    void ParseAudioPorts(xmlNode &node);
    void ParseAudioPortPins(xmlNode &node);
    InternalDeviceType GetDeviceType(xmlChar &device);

    IPortObserver &mPortObserver;
    xmlDoc *mDoc;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_XML_PARSER_H
