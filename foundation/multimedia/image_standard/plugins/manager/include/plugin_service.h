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

#ifndef PLUGIN_SERVICE_H
#define PLUGIN_SERVICE_H

namespace OHOS {
namespace MultimediaPlugin {
constexpr uint8_t SUBID_BIT_NUM = 12;
constexpr uint16_t INVALID_IID = 0xFFFF;

constexpr uint16_t IID_TYPE_GENERAL = 0;
constexpr uint16_t IID_TYPE_PIPELINE = 1;

constexpr uint16_t MakeInterfaceID(uint16_t interfaceIDType, uint16_t subID)
{
    return ((interfaceIDType << SUBID_BIT_NUM) | subID);
}

constexpr uint16_t GetInterfaceIDType(uint16_t interfaceID)
{
    return (interfaceID >> SUBID_BIT_NUM);
}

constexpr uint16_t PLUGIN_EXAMPLE_IID = MakeInterfaceID(IID_TYPE_GENERAL, 0);
constexpr uint16_t IMAGE_FORMAT_AGENT_IID = MakeInterfaceID(IID_TYPE_GENERAL, 1);
constexpr uint16_t IMAGE_DECODER_IID = MakeInterfaceID(IID_TYPE_GENERAL, 2);
constexpr uint16_t IMAGE_ENCODER_IID = MakeInterfaceID(IID_TYPE_GENERAL, 3);
constexpr uint16_t IMAGE_DECOMPRESS_COMP_IID = MakeInterfaceID(IID_TYPE_GENERAL, 4);
constexpr uint16_t IMAGE_EXIF_IID = MakeInterfaceID(IID_TYPE_GENERAL, 5);

template<class T> inline uint16_t GetInterfaceId()
{
    return INVALID_IID;
}
#define GET_INTERFACE_ID OHOS::MultimediaPlugin::GetInterfaceId
#define DECLARE_INTERFACE(Type, ID) template<> inline uint16_t GET_INTERFACE_ID<Type>() { return ID; }
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // PLUGIN_SERVICE_H
