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
#include "js_convertxml.h"
#include "securec.h"
#include "utils/log.h"

ConvertXml::ConvertXml(napi_env env): env_(env)
{
    m_SpaceType = SpaceType::T_INIT;
    m_strSpace = "";
    m_iSpace = 0;
}
std::string ConvertXml::GetNodeType(xmlElementType enumType)
{
    std::string strResult = "";
    switch (enumType) {
        case xmlElementType::XML_ELEMENT_NODE:
            strResult = "element";
            break;
        case xmlElementType::XML_ATTRIBUTE_NODE:
            strResult = "attribute";
            break;
        case xmlElementType::XML_TEXT_NODE:
            strResult = "text";
            break;
        case xmlElementType::XML_CDATA_SECTION_NODE:
            strResult = "cdata";
            break;
        case xmlElementType::XML_ENTITY_REF_NODE:
            strResult = "entity_ref";
            break;
        case xmlElementType::XML_ENTITY_NODE:
            strResult = "entity";
            break;
        case xmlElementType::XML_PI_NODE:
            strResult = "instruction";
            break;
        case xmlElementType::XML_COMMENT_NODE:
            strResult = "comment";
            break;
        case xmlElementType::XML_DOCUMENT_NODE:
            strResult = "document";
            break;
        case xmlElementType::XML_DOCUMENT_TYPE_NODE:
            strResult = "document_type";
            break;
        case xmlElementType::XML_DOCUMENT_FRAG_NODE:
            strResult = "document_frag";
            break;
        case xmlElementType::XML_DTD_NODE:
            strResult = "doctype";
            break;
#ifdef LIBXML_DOCB_ENABLED
        case xmlElementType::XML_DOCB_DOCUMENT_NODE:
            strResult =  "docb_document";
            break;
#endif
        default:
            break;
    }
    return strResult;
}

void ConvertXml::SetKeyValue(napi_value &object, std::string strKey, std::string strValue)
{
    napi_value attrValue = nullptr;
    napi_create_string_utf8(env_, strValue.c_str(), NAPI_AUTO_LENGTH, &attrValue);
    napi_set_named_property(env_, object, strKey.c_str(), attrValue);
}
std::string ConvertXml::Trim(std::string strXmltrim)
{
    if (strXmltrim.empty()) {
        return "";
    }
    size_t i = 0;
    size_t strlen = strXmltrim.size();
    for (; i < strlen;) {
        if (strXmltrim[i] == ' ') {
            i++;
        } else {
            break;
        }
    }
    strXmltrim = strXmltrim.substr(i);
    strlen = strXmltrim.size();
    for (i = strlen - 1; i != 0; i--) {
        if (strXmltrim[i] == ' ') {
            strXmltrim.pop_back();
        } else {
            break;
        }
    }
    return strXmltrim;
}

void ConvertXml::GetPrevNodeList(xmlNodePtr curNode)
{
    while (curNode->prev != nullptr) {
        curNode = curNode->prev;
        napi_value elementsObject = nullptr;
        napi_create_object(env_, &elementsObject);
        if (curNode->type == xmlElementType::XML_PI_NODE && !m_Options.ignoreInstruction) {
            SetKeyValue(elementsObject, m_Options.type, GetNodeType(curNode->type));
            SetKeyValue(elementsObject, m_Options.name, (char*)curNode->name);
            if (xmlNodeGetContent(curNode) != nullptr) {
                SetKeyValue(elementsObject, m_Options.instruction, (const char*)xmlNodeGetContent(curNode));
            }
            m_prevObj.push_back(elementsObject);
        }
        if (curNode->type == xmlElementType::XML_COMMENT_NODE && !m_Options.ignoreComment) {
            SetKeyValue(elementsObject, m_Options.type, GetNodeType(curNode->type));
            if (xmlNodeGetContent(curNode) != nullptr) {
                SetKeyValue(elementsObject, m_Options.comment, (const char*)xmlNodeGetContent(curNode));
            }
            m_prevObj.push_back(elementsObject);
        }
        if (curNode->type == xmlElementType::XML_DTD_NODE && !m_Options.ignoreDoctype) {
            SetKeyValue(elementsObject, m_Options.type, GetNodeType(curNode->type));
            SetKeyValue(elementsObject, m_Options.doctype, (char*)curNode->name);
            m_prevObj.push_back(elementsObject);
        }
    }
}

void ConvertXml::SetAttributes(xmlNodePtr curNode, napi_value &elementsObject)
{
    xmlAttr *attr = curNode->properties;
    if (attr && !m_Options.ignoreAttributes) {
        napi_value attrTitleObj = nullptr;
        napi_create_object(env_, &attrTitleObj);
        while (attr) {
            SetKeyValue(attrTitleObj, (const char*)attr->name, (const char*)attr->children->content);
            attr = attr->next;
        }
        napi_set_named_property(env_, elementsObject, m_Options.attributes.c_str(), attrTitleObj);
    }
}

void ConvertXml::SetXmlElementType(xmlNodePtr curNode, napi_value &elementsObject, bool &bFlag)
{
    if (curNode->type == xmlElementType::XML_PI_NODE && !m_Options.ignoreInstruction) {
        if (xmlNodeGetContent(curNode) != nullptr) {
            SetKeyValue(elementsObject, m_Options.instruction.c_str(), (const char*)xmlNodeGetContent(curNode));
            bFlag = true;
        }
    } else if (curNode->type == xmlElementType::XML_COMMENT_NODE && !m_Options.ignoreComment) {
        if (xmlNodeGetContent(curNode) != nullptr) {
            SetKeyValue(elementsObject, m_Options.comment.c_str(), (const char*)xmlNodeGetContent(curNode));
            bFlag = true;
        }
    } else if (curNode->type == xmlElementType::XML_CDATA_SECTION_NODE && !m_Options.ignoreCdata) {
        if (xmlNodeGetContent(curNode) != nullptr) {
            SetKeyValue(elementsObject, m_Options.cdata, (const char*)xmlNodeGetContent(curNode));
            bFlag = true;
        }
    }
}
void ConvertXml::SetNodeInfo(xmlNodePtr curNode, napi_value &elementsObject)
{
    if (curNode->type == xmlElementType::XML_TEXT_NODE) {
        return;
    } else {
        if (curNode->type == xmlElementType::XML_PI_NODE) {
            if (!m_Options.ignoreInstruction) {
                SetKeyValue(elementsObject, m_Options.type, GetNodeType(curNode->type));
            }
        } else {
                SetKeyValue(elementsObject, m_Options.type, GetNodeType(curNode->type));
        }
        if ((curNode->type != xmlElementType::XML_COMMENT_NODE) &&
            (curNode->type != xmlElementType::XML_CDATA_SECTION_NODE)) {
            if (!(curNode->type == xmlElementType::XML_PI_NODE && m_Options.ignoreInstruction)) {
                SetKeyValue(elementsObject, m_Options.name, (char*)curNode->name);
            }
        }
    }
}

void ConvertXml::SetEndInfo(xmlNodePtr curNode, napi_value &elementsObject, bool &bFlag)
{
    SetKeyValue(elementsObject, m_Options.type, GetNodeType(curNode->type));
    if (curNode->type == xmlElementType::XML_ELEMENT_NODE) {
        SetKeyValue(elementsObject, m_Options.name.c_str(), (const char*)curNode->name);
        bFlag = true;
    } else if (curNode->type == xmlElementType::XML_TEXT_NODE) {
        if (m_Options.trim) {
            if (xmlNodeGetContent(curNode) != nullptr) {
                SetKeyValue(elementsObject, m_Options.text, Trim((const char*)xmlNodeGetContent(curNode)));
            }
        } else {
            if (xmlNodeGetContent(curNode) != nullptr) {
                SetKeyValue(elementsObject, m_Options.text, (const char*)xmlNodeGetContent(curNode));
            }
        }
        if (!m_Options.ignoreText) {
            bFlag = true;
        }
    }
}

void ConvertXml::SetPrevInfo(napi_value &recvElement, int flag, int32_t &index1)
{
    if (!m_prevObj.empty() && !flag) {
        for (int i = (m_prevObj.size() - 1); i >= 0; --i) {
            napi_set_element(env_, recvElement, index1++, m_prevObj[i]);
        }
    }
}

void ConvertXml::GetXMLInfo(xmlNodePtr curNode, napi_value &object, int flag)
{
    napi_value elements = nullptr;
    napi_create_array(env_, &elements);
    napi_value recvElement = nullptr;
    napi_create_array(env_, &recvElement);
    xmlNodePtr pNode = curNode;
    int32_t index = 0;
    int32_t index1 = 0;
    bool bFlag = false;
    while (pNode != nullptr) {
        bFlag = false;
        napi_value elementsObject = nullptr;
        napi_create_object(env_, &elementsObject);
        SetNodeInfo(pNode, elementsObject);
        SetAttributes(pNode, elementsObject);
        napi_value tempElement = nullptr;
        napi_create_array(env_, &tempElement);
        napi_value elementObj = nullptr;
        napi_create_object(env_, &elementObj);
        if (xmlNodeGetContent(pNode) != nullptr) {
            if (pNode->children != nullptr) {
                curNode = pNode->children;
                GetXMLInfo(curNode, elementsObject, 1);
                bFlag = true;
            } else {
                SetXmlElementType(pNode, elementsObject, bFlag);
                SetEndInfo(pNode, elementsObject, bFlag);
            }
        }
        SetPrevInfo(recvElement, flag, index1);
        if (elementsObject != nullptr && bFlag) {
            napi_set_element(env_, recvElement, index1++, elementsObject);
            elementsObject = nullptr;
        }
        index++;
        pNode = pNode->next;
    }
    if (bFlag) {
        napi_set_named_property(env_, object, m_Options.elements.c_str(), recvElement);
    }
}

void ConvertXml::SetSpacesInfo(napi_value &object)
{
    napi_value iTemp = nullptr;
    switch (m_SpaceType) {
        case (SpaceType::T_INT32):
            napi_create_int32(env_, m_iSpace, &iTemp);
            napi_set_named_property(env_, object, "spaces", iTemp);
            break;
        case (SpaceType::T_STRING):
            SetKeyValue(object, "spaces", m_strSpace);
            break;
        case (SpaceType::T_INIT):
            SetKeyValue(object, "spaces", m_strSpace);
            break;
        default:
            break;
        }
}

napi_value ConvertXml::convert(std::string strXml)
{
    xmlDocPtr doc = nullptr;
    xmlNodePtr curNode = nullptr;
    napi_status status = napi_ok;
    napi_value object = nullptr;
    status = napi_create_object(env_, &object);
    if (status != napi_ok) {
        return nullptr;
    }
    Replace(strXml, "\\r", "\r");
    Replace(strXml, "\\n", "\n");
    Replace(strXml, "\\v", "\v");
    Replace(strXml, "]]><![CDATA", "]]> <![CDATA");
    size_t len = strXml.size();
    doc = xmlParseMemory(strXml.c_str(), len);
    if (!doc) {
        xmlFreeDoc(doc);
        DealSingleLine(strXml, object);
        return object;
    }
    napi_value subObject = nullptr;
    napi_value subSubObject = nullptr;
    napi_create_object(env_, &subSubObject);
    napi_create_object(env_, &subObject);
    if (doc != nullptr && doc->version != nullptr) {
        SetKeyValue(subSubObject, "version", (const char*)doc->version);
    }
    if (doc != nullptr && doc->encoding != nullptr) {
        SetKeyValue(subSubObject, "encoding", (const char*)doc->encoding);
    }
    if (!m_Options.ignoreDeclaration && strXml.find("xml") != std::string::npos) {
        napi_set_named_property(env_, subObject, m_Options.attributes.c_str(), subSubObject);
        napi_set_named_property(env_, object, m_Options.declaration.c_str(), subObject);
    }
    if (doc != nullptr) {
        curNode = xmlDocGetRootElement(doc);
        GetPrevNodeList(curNode);
        GetXMLInfo(curNode, object, 0);
    }
    SetSpacesInfo(object);
    return object;
}

napi_status ConvertXml::DealNapiStrValue(napi_value napi_StrValue, std::string &result)
{
    char *buffer = nullptr;
    size_t bufferSize = 0;
    napi_status status = napi_ok;
    status = napi_get_value_string_utf8(env_, napi_StrValue, nullptr, -1, &bufferSize);
    if (status != napi_ok) {
        return status;
    }
    if (bufferSize > 0) {
        buffer = new char[bufferSize + 1];
        napi_get_value_string_utf8(env_, napi_StrValue, buffer, bufferSize + 1, &bufferSize);
    }
    if (buffer != nullptr) {
        result = buffer;
        delete []buffer;
        buffer = nullptr;
    }
    return status;
}

void ConvertXml::DealSpaces(napi_value napi_obj)
{
    napi_value recvTemp = nullptr;
    napi_get_named_property(env_, napi_obj, "spaces", &recvTemp);
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env_, recvTemp, &valuetype);
    if (valuetype == napi_string) {
        DealNapiStrValue(recvTemp, m_strSpace);
        m_SpaceType = SpaceType::T_STRING;
    } else if (valuetype == napi_number) {
        int32_t iTemp;
        if (napi_get_value_int32(env_, recvTemp, &iTemp) == napi_ok) {
            m_iSpace = iTemp;
            m_SpaceType = SpaceType::T_INT32;
        }
    }
}

void ConvertXml::DealIgnore(napi_value napi_obj)
{
    std::vector<std::string> vctIgnore = {"compact", "trim", "ignoreDeclaration", "ignoreInstruction",
                                          "ignoreAttributes", "ignoreComment", "ignoreCdata",
                                          "ignoreDoctype", "ignoreText"};
    for (size_t i = 0; i < vctIgnore.size(); ++i) {
        napi_value recvTemp = nullptr;
        bool bRecv = false;
        napi_get_named_property(env_, napi_obj, vctIgnore[i].c_str(), &recvTemp);
        if ((napi_get_value_bool(env_, recvTemp, &bRecv)) == napi_ok) {
            switch (i) {
                case 0:
                    m_Options.compact = bRecv;
                    break;
                case 1: // 1:trim
                    m_Options.trim = bRecv;
                    break;
                case 2: // 2:ignoreDeclaration
                    m_Options.ignoreDeclaration = bRecv;
                    break;
                case 3: // 3:ignoreInstruction
                    m_Options.ignoreInstruction = bRecv;
                    break;
                case 4: // 4:ignoreAttributes
                    m_Options.ignoreAttributes = bRecv;
                    break;
                case 5: // 5:ignoreComment
                    m_Options.ignoreComment = bRecv;
                    break;
                case 6: // 6:ignoreCdata
                    m_Options.ignoreCdata = bRecv;
                    break;
                case 7: // 7:ignoreDoctype
                    m_Options.ignoreDoctype = bRecv;
                    break;
                case 8: // 8:ignoreText
                    m_Options.ignoreText = bRecv;
                    break;
                default:
                    break;
            }
        }
    }
}

void ConvertXml::SetDefaultKey(size_t i, std::string strRecv)
{
    switch (i) {
        case 0:
            m_Options.declaration = strRecv;
            break;
        case 1:
            m_Options.instruction = strRecv;
            break;
        case 2: // 2:attributes
            m_Options.attributes = strRecv;
            break;
        case 3: // 3:text
            m_Options.text = strRecv;
            break;
        case 4: // 4:cdata
            m_Options.cdata = strRecv;
            break;
        case 5: // 5:doctype
            m_Options.doctype = strRecv;
            break;
        case 6: // 6:comment
            m_Options.comment = strRecv;
            break;
        case 7: // 7:parent
            m_Options.parent = strRecv;
            break;
        case 8: // 8:type
            m_Options.type = strRecv;
            break;
        case 9: // 9:name
            m_Options.name = strRecv;
            break;
        case 10: // 10:elements
            m_Options.elements = strRecv;
            break;
        default:
            break;
    }
}

void ConvertXml::DealOptions(napi_value napi_obj)
{
    std::vector<std::string> vctOptions = {"declarationKey", "instructionKey", "attributesKey", "textKey",
                                           "cdataKey", "doctypeKey", "commentKey", "parentKey", "typeKey",
                                           "nameKey", "elementsKey"};
    for (size_t i = 0; i < vctOptions.size(); ++i) {
        napi_value recvTemp = nullptr;
        std::string strRecv = "";
        napi_get_named_property(env_, napi_obj, vctOptions[i].c_str(), &recvTemp);
        if ((DealNapiStrValue(recvTemp, strRecv)) == napi_ok) {
            SetDefaultKey(i, strRecv);
        }
    }
    DealIgnore(napi_obj);
    DealSpaces(napi_obj);
}

void ConvertXml::DealSingleLine(std::string &strXml, napi_value &object)
{
    size_t iXml = 0;
    if ((iXml = strXml.find("xml")) != std::string::npos) {
        m_XmlInfo.bXml = true;
        napi_value declObj = nullptr;
        napi_create_object(env_, &declObj);
        napi_value attrObj = nullptr;
        bool bFlag = false;
        napi_create_object(env_, &attrObj);
        if (strXml.find("version=") != std::string::npos) {
            m_XmlInfo.bVersion = true;
            SetKeyValue(attrObj, "version", "1.0");
            bFlag = true;
        }
        if (strXml.find("encoding=") != std::string::npos) {
            m_XmlInfo.bEncoding = false;
            SetKeyValue(attrObj, "encoding", "utf-8");
            bFlag = true;
        }
        if (bFlag) {
            napi_set_named_property(env_, declObj, m_Options.attributes.c_str(), attrObj);
            napi_set_named_property(env_, object, m_Options.declaration.c_str(), declObj);
        } else {
            napi_set_named_property(env_, object, m_Options.declaration.c_str(), declObj);
        }
        if (strXml.find(">", iXml) == strXml.size() - 1) {
            strXml = "";
        } else {
            strXml = strXml.substr(0, strXml.rfind("<", iXml)) + strXml.substr(strXml.find(">", iXml) + 1);
        }
    }
    size_t iCount = 0;
    size_t iLen = strXml.size();
    for (; iCount < iLen; ++iCount) {
        if (strXml[iCount] != ' ' && strXml[iCount] != '\v' &&
            strXml[iCount] != '\t' && strXml[iCount] != '\n') {
            break;
        }
    }
    if (iCount < iLen) {
        DealComplex(strXml, object);
    }
}

void ConvertXml::DealComplex(std::string &strXml, napi_value &object)
{
    if (strXml.find("<!DOCTYPE") != std::string::npos) {
        strXml = strXml + "<node></node>";
    } else {
        strXml = "<node>" + strXml + "</node>";
    }
    xmlDocPtr doc = nullptr;
    xmlNodePtr curNode = nullptr;
    size_t len = strXml.size();
    doc = xmlParseMemory(strXml.c_str(), len);
    if (!doc) {
        xmlFreeDoc(doc);
    }
    if (doc) {
        curNode = xmlDocGetRootElement(doc);
        curNode = curNode->children;
        napi_value elements = nullptr;
        napi_create_array(env_, &elements);
        bool bHasEle = false;
        int index = 0;
        bool bCData = false;
        if (strXml.find("<![CDATA") != strXml.rfind("<![CDATA")) {
            bCData = true;
        }
        while (curNode != nullptr) {
            napi_value elementsObject = nullptr;
            napi_create_object(env_, &elementsObject);
            SetNodeInfo(curNode, elementsObject);
            SetXmlElementType(curNode, elementsObject, bHasEle);
            SetEndInfo(curNode, elementsObject, bHasEle);
            napi_set_element(env_, elements, index++, elementsObject);
            DealCDataInfo(bCData, curNode);
        }
        if (bHasEle) {
            napi_set_named_property(env_, object, m_Options.elements.c_str(), elements);
        }
        xmlFreeDoc(doc);
    }
}

void ConvertXml::Replace(std::string &str, const std::string src, const std::string dst)
{
    size_t index = 0;
    while ((index = str.find(src)) != std::string::npos) {
        str.replace(index, src.size(), dst);
    }
}

void ConvertXml::DealCDataInfo(bool bCData, xmlNodePtr &curNode)
{
    if (bCData && curNode->type == xmlElementType::XML_CDATA_SECTION_NODE &&
        curNode->next && curNode->next->type == xmlElementType::XML_TEXT_NODE &&
        curNode->next->next && curNode->next->next->type == xmlElementType::XML_CDATA_SECTION_NODE) {
            if (xmlNodeGetContent(curNode->next) != nullptr) {
                std::string strTemp = (char*)xmlNodeGetContent(curNode->next);
                Replace(strTemp, " ", "");
                Replace(strTemp, "\v", "");
                Replace(strTemp, "\t", "");
                Replace(strTemp, "\n", "");
                if (strTemp == "") {
                    curNode = curNode->next->next;
                }
            }
        } else {
            curNode = curNode->next;
        }
}