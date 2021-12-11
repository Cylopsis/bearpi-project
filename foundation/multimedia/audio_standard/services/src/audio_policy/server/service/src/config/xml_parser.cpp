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

#include <cstdlib>
#include "media_log.h"
#include "xml_parser.h"

namespace OHOS {
namespace AudioStandard {
bool XMLParser::LoadConfiguration()
{
    mDoc = xmlReadFile(CONFIG_FILE, NULL, 0);
    if (mDoc == NULL) {
        MEDIA_ERR_LOG("xmlReadFile Failed");
        return false;
    }

    return true;
}

bool XMLParser::Parse()
{
    xmlNode *root = xmlDocGetRootElement(mDoc);
    if (root == NULL) {
        MEDIA_ERR_LOG("xmlDocGetRootElement Failed");
        return false;
    }

    if (!ParseInternal(*root)) {
        return false;
    }

    return true;
}

void XMLParser::Destroy()
{
    if (mDoc != NULL) {
        xmlFreeDoc(mDoc);
    }

    return;
}

bool XMLParser::ParseInternal(xmlNode &node)
{
    xmlNode *currNode = &node;
    for (; currNode; currNode = currNode->next) {
        if (XML_ELEMENT_NODE == currNode->type) {
            switch (GetNodeNameAsInt(*currNode)) {
                case BUILT_IN_DEVICES:
                    ParseBuiltInDevices(*currNode);
                    break;
                case DEFAULT_OUTPUT_DEVICE:
                    ParseDefaultOutputDevice(*currNode);
                    break;
                case DEFAULT_INPUT_DEVICE:
                    ParseDefaultInputDevice(*currNode);
                    break;
                case AUDIO_PORTS:
                    ParseAudioPorts(*currNode);
                    break;
                case AUDIO_PORT_PINS:
                    ParseAudioPortPins(*currNode);
                    break;
                default:
                    ParseInternal(*(currNode->children));
                    break;
            }
        }
    }

    return true;
}

NodeName XMLParser::GetNodeNameAsInt(xmlNode &node)
{
    if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("BuiltInDevices"))) {
        return BUILT_IN_DEVICES;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("DefaultOutputDevice"))) {
        return DEFAULT_OUTPUT_DEVICE;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("DefaultInputDevice"))) {
        return DEFAULT_INPUT_DEVICE;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("AudioPorts"))) {
        return AUDIO_PORTS;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("AudioPort"))) {
        return AUDIO_PORT;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("AudioPortPins"))) {
        return AUDIO_PORT_PINS;
    } else if (!xmlStrcmp(node.name, reinterpret_cast<const xmlChar*>("AudioPortPin"))) {
        return AUDIO_PORT_PIN;
    } else {
        return UNKNOWN;
    }
}

void XMLParser::ParseBuiltInDevices(xmlNode &node)
{
    xmlNode *currNode = &node;
    while (currNode) {
        xmlNode *child = currNode->children;
        xmlChar *device = xmlNodeGetContent(child);

        if (device != NULL) {
            MEDIA_DEBUG_LOG("Trigger Cb");
        }

        currNode = currNode->next;
    }
    return;
}

void XMLParser::ParseDefaultOutputDevice(xmlNode &node)
{
    xmlNode *child = node.children;
    xmlChar *device = xmlNodeGetContent(child);

    if (device != NULL) {
        MEDIA_DEBUG_LOG("DefaultOutputDevice %{public}s", device);
        mPortObserver.OnDefaultOutputPortPin(GetDeviceType(*device));
    }
    return;
}

void XMLParser::ParseDefaultInputDevice(xmlNode &node)
{
    xmlNode *child = node.children;
    xmlChar *device = xmlNodeGetContent(child);

    MEDIA_DEBUG_LOG("DefaultInputDevice");
    if (device != NULL) {
        MEDIA_DEBUG_LOG("DefaultInputDevice %{public}s", device);
        mPortObserver.OnDefaultInputPortPin(GetDeviceType(*device));
    }
    return;
}

void XMLParser::ParseAudioPorts(xmlNode &node)
{
    xmlNode *child = node.xmlChildrenNode;

    for (; child; child = child->next) {
        if (!xmlStrcmp(child->name, reinterpret_cast<const xmlChar*>("AudioPort"))) {
            std::unique_ptr<AudioPortInfo> portInfo = std::make_unique<AudioPortInfo>();
            portInfo->type = TYPE_AUDIO_PORT;

            if (xmlHasProp(child, reinterpret_cast<xmlChar*>(const_cast<char*>("role")))) {
                portInfo->role = reinterpret_cast<char*>(xmlGetProp(
                    child,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("role"))));
            }

            if (xmlHasProp(child, reinterpret_cast<xmlChar*>(const_cast<char*>("name")))) {
                portInfo->name = reinterpret_cast<char*>(xmlGetProp(
                    child,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
            }

            if (xmlHasProp(child, reinterpret_cast<xmlChar*>(const_cast<char*>("channels")))) {
                portInfo->channels = reinterpret_cast<char*>(xmlGetProp(
                    child,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("channels"))));
            }

            if (xmlHasProp(child, reinterpret_cast<xmlChar*>(const_cast<char*>("buffer_size")))) {
                portInfo->buffer_size = reinterpret_cast<char*>(xmlGetProp(
                    child,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("buffer_size"))));
            }

            if (xmlHasProp(child, reinterpret_cast<xmlChar*>(const_cast<char*>("rate")))) {
                portInfo->rate = reinterpret_cast<char*>(xmlGetProp(
                    child,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("rate"))));
            }

            if (xmlHasProp(child, reinterpret_cast<xmlChar*>(const_cast<char*>("file")))) {
                portInfo->fileName = reinterpret_cast<char*>(xmlGetProp(
                    child,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("file"))));
            }

            mPortObserver.OnAudioPortAvailable(std::move(portInfo));
        }
    }

    return;
}

void XMLParser::ParseAudioPortPins(xmlNode &node)
{
    xmlNode *child = node.xmlChildrenNode;

    for (; child; child = child->next) {
        if (!xmlStrcmp(child->name, reinterpret_cast<const xmlChar*>("AudioPortPin"))) {
            std::unique_ptr<AudioPortPinInfo> portInfo = std::make_unique<AudioPortPinInfo>();
            portInfo->type = TYPE_AUDIO_PORT_PIN;

            if (xmlHasProp(child, reinterpret_cast<xmlChar*>(const_cast<char*>("role"))))
                portInfo->role = reinterpret_cast<char*>(xmlGetProp(
                    child,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("role"))));
            if (xmlHasProp(child, reinterpret_cast<xmlChar*>(const_cast<char*>("name"))))
                portInfo->name = reinterpret_cast<char*>(xmlGetProp(
                    child,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
            if (xmlHasProp(child, reinterpret_cast<xmlChar*>(const_cast<char*>("type"))))
                portInfo->pinType = reinterpret_cast<char*>(xmlGetProp(
                    child,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("type"))));

            MEDIA_INFO_LOG("AudioPort:Role: %s, Name: %s, Type: %s", portInfo->role, portInfo->name, portInfo->pinType);

            mPortObserver.OnAudioPortPinAvailable(std::move(portInfo));
        }
    }

    return;
}

InternalDeviceType XMLParser::GetDeviceType(xmlChar &device)
{
    if (!xmlStrcmp(&device, reinterpret_cast<const xmlChar*>("Speaker")))
        return InternalDeviceType::SPEAKER;
    if (!xmlStrcmp(&device, reinterpret_cast<const xmlChar*>("Built-In Mic")))
        return InternalDeviceType::MIC;

    return InternalDeviceType::DEVICE_TYPE_NONE;
}
} // namespace AudioStandard
} // namespace OHOS
