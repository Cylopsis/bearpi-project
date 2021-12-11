 /*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef FOUNDATION_ACE_CCRUNTIME_CONVERT_XML_CLASS_H
#define FOUNDATION_ACE_CCRUNTIME_CONVERT_XML_CLASS_H

#include <string>
#include <vector>
#include <map>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "libxml/parser.h"
#include "libxml/tree.h"

enum class SpaceType {
    T_INT32,
    T_STRING,
    T_INIT = -1
};

struct Options {
    std::string declaration = "_declaration";
    std::string instruction = "_instruction";
    std::string attributes = "_attributes";
    std::string text = "_text";
    std::string cdata = "_cdata";
    std::string doctype = "_doctype";
    std::string comment = "_comment";
    std::string parent = "_parent";
    std::string type = "_type";
    std::string name = "_name";
    std::string elements = "_elements";
    bool compact = false;
    bool trim = false;
    bool nativetype = false;
    bool nativetypeattributes = false;
    bool addparent = false;
    bool alwaysArray = false;
    bool alwaysChildren = false;
    bool instructionHasAttributes = false;
    bool ignoreDeclaration = false;
    bool ignoreInstruction = false;
    bool ignoreAttributes = false;
    bool ignoreComment = false;
    bool ignoreCdata = false;
    bool ignoreDoctype = false;
    bool ignoreText = false;
    bool spaces = false;
};

struct XmlInfo {
    bool bXml = false;
    bool bVersion = false;
    std::string strVersion = "";
    bool bEncoding = false;
    std::string strEncoding = "";
};

class ConvertXml {
public:
        explicit ConvertXml(napi_env env);
        virtual ~ConvertXml() {}
        void SetAttributes(xmlNodePtr curNode, napi_value &elementsObject);
        void SetXmlElementType(xmlNodePtr curNode, napi_value &elementsObject, bool &bFlag);
        void SetNodeInfo(xmlNodePtr curNode, napi_value &elementsObject);
        void SetEndInfo(xmlNodePtr curNode, napi_value &elementsObject, bool &bFlag);
        void GetXMLInfo(xmlNodePtr curNode, napi_value &object, int flag = 0);
        napi_value convert(std::string strXml);
        std::string GetNodeType(xmlElementType enumType);
        napi_status DealNapiStrValue(napi_value napi_StrValue, std::string &result);
        void SetKeyValue(napi_value &object, std::string strKey, std::string strValue);
        void DealOptions(napi_value napi_obj);
        std::string Trim(std::string strXmltrim);
        void GetPrevNodeList(xmlNodePtr curNode);
        void DealSpaces(napi_value napi_obj);
        void DealIgnore(napi_value napi_obj);
        void SetPrevInfo(napi_value &recvElement, int flag, int32_t &index1);
        void SetDefaultKey(size_t i, std::string strRecv);
        void SetSpacesInfo(napi_value &object);
        void DealSingleLine(std::string &strXml, napi_value &object);
        void DealComplex(std::string &strXml, napi_value &object);
        void Replace(std::string &str, const std::string src, const std::string dst);
        void DealCDataInfo(bool bCData, xmlNodePtr &curNode);
private:
        napi_env env_;
        SpaceType m_SpaceType;
        int32_t m_iSpace;
        std::string m_strSpace;
        Options m_Options;
        std::vector<napi_value> m_prevObj;
        XmlInfo m_XmlInfo;
};
#endif