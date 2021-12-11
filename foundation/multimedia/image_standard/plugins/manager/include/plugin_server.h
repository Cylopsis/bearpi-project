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

#ifndef PLUGIN_SERVER_H
#define PLUGIN_SERVER_H

#include <map>
#include <string>
#include <vector>
#include <cstdint>
#include "singleton.h"
#include "attr_data.h"
#include "plugin_class_base.h"
#include "plugin_common_type.h"
#include "plugin_errors.h"
#include "plugin_service.h"
#include "priority_scheme.h"

namespace OHOS {
namespace MultimediaPlugin {
class PlatformAdp;
class PluginFw;
class GstPluginFw;

enum class PluginFWType : int32_t {
    PLUGIN_FW_GENERAL = 0,
    PLUGIN_FW_GSTREAMER
};

class PluginServer final : public NoCopyable {
public:
    uint32_t Register(std::vector<std::string> &&pluginPaths);

    template<typename T>
    inline T *CreateObject(const std::string &className, uint32_t &errorCode)
    {
        uint16_t interfaceID = GetInterfaceId<T>();
        return ConvertToServiceInterface<T>(CreateObject(interfaceID, className, errorCode));
    }

    template<typename T>
    inline T *CreateObject(const std::string &className)
    {
        uint16_t interfaceID = GetInterfaceId<T>();
        uint32_t errorCode = 0;
        return ConvertToServiceInterface<T>(CreateObject(interfaceID, className, errorCode));
    }

    template<typename T>
    inline T *CreateObject(uint16_t serviceType, uint32_t &errorCode)
    {
        uint16_t interfaceID = GetInterfaceId<T>();
        std::map<std::string, AttrData> emptyCapabilities;
        PriorityScheme emptyPriScheme;
        return ConvertToServiceInterface<T>(CreateObject(interfaceID, serviceType, emptyCapabilities,
                                                         emptyPriScheme, errorCode));
    }

    template<typename T>
    inline T *CreateObject(uint16_t serviceType)
    {
        uint16_t interfaceID = GetInterfaceId<T>();
        std::map<std::string, AttrData> emptyCapabilities;
        PriorityScheme emptyPriScheme;
        uint32_t errorCode = 0;
        return ConvertToServiceInterface<T>(CreateObject(interfaceID, serviceType, emptyCapabilities,
                                                         emptyPriScheme, errorCode));
    }

    template<typename T>
    inline T *CreateObject(uint16_t serviceType, const PriorityScheme &priorityScheme, uint32_t &errorCode)
    {
        uint16_t interfaceID = GetInterfaceId<T>();
        std::map<std::string, AttrData> emptyCapabilities;
        return ConvertToServiceInterface<T>(CreateObject(interfaceID, serviceType, emptyCapabilities,
                                                         priorityScheme, errorCode));
    }

    template<typename T>
    inline T *CreateObject(uint16_t serviceType, const PriorityScheme &priorityScheme)
    {
        uint16_t interfaceID = GetInterfaceId<T>();
        std::map<std::string, AttrData> emptyCapabilities;
        uint32_t errorCode = 0;
        return ConvertToServiceInterface<T>(CreateObject(interfaceID, serviceType, emptyCapabilities,
                                                         priorityScheme, errorCode));
    }

    template<typename T>
    inline T *CreateObject(uint16_t serviceType, const std::map<std::string, AttrData> &capabilities,
                           uint32_t &errorCode)
    {
        uint16_t interfaceID = GetInterfaceId<T>();
        PriorityScheme emptyPriScheme;
        return ConvertToServiceInterface<T>(CreateObject(interfaceID, serviceType, capabilities,
                                                         emptyPriScheme, errorCode));
    }

    template<typename T>
    inline T *CreateObject(uint16_t serviceType, const std::map<std::string, AttrData> &capabilities)
    {
        uint16_t interfaceID = GetInterfaceId<T>();
        PriorityScheme emptyPriScheme;
        uint32_t errorCode = 0;
        return ConvertToServiceInterface<T>(CreateObject(interfaceID, serviceType, capabilities,
                                                         emptyPriScheme, errorCode));
    }

    template<typename T>
    inline T *CreateObject(uint16_t serviceType, const std::map<std::string, AttrData> &capabilities,
                           const PriorityScheme &priorityScheme, uint32_t &errorCode)
    {
        uint16_t interfaceID = GetInterfaceId<T>();
        return ConvertToServiceInterface<T>(CreateObject(interfaceID, serviceType, capabilities,
                                                         priorityScheme, errorCode));
    }

    template<typename T>
    inline T *CreateObject(uint16_t serviceType, const std::map<std::string, AttrData> &capabilities,
                           const PriorityScheme &priorityScheme)
    {
        uint16_t interfaceID = GetInterfaceId<T>();
        uint32_t errorCode = 0;
        return ConvertToServiceInterface<T>(CreateObject(interfaceID, serviceType, capabilities,
                                                         priorityScheme, errorCode));
    }

    template<typename T>
    inline uint32_t PluginServerGetClassInfo(uint16_t serviceType, std::vector<ClassInfo> &classesInfo)
    {
        uint16_t interfaceID = GetInterfaceId<T>();
        std::map<std::string, AttrData> emptyCapabilities;
        return PluginServerGetClassInfo(interfaceID, serviceType, emptyCapabilities, classesInfo);
    }

    template<typename T>
    inline uint32_t PluginServerGetClassInfo(uint16_t serviceType, const std::map<std::string, AttrData> &capabilities,
                                 std::vector<ClassInfo> &classesInfo)
    {
        uint16_t interfaceID = GetInterfaceId<T>();
        return PluginServerGetClassInfo(interfaceID, serviceType, capabilities, classesInfo);
    }

    DECLARE_DELAYED_REF_SINGLETON(PluginServer);

private:
    template<typename T>
    inline T *ConvertToServiceInterface(PluginClassBase *pluginBase)
    {
#ifdef PLUGIN_FLAG_RTTI_ENABLE
        // when -frtti is enable, we use dynamic cast directly
        // to achieve the correct base class side-to-side conversion.
        T *serviceObj = dynamic_cast<T *>(pluginBase);
        if (serviceObj == nullptr && pluginBase != nullptr) {
            // type mismatch.
            delete pluginBase;
        }
#else
        // adjust pointer position when multiple inheritance.
        void *obj = dynamic_cast<void *>(pluginBase);
        // when -frtti is not enable, we use static cast.
        // static cast is not safe enough, but we have checked before we get here.
        T *serviceObj = static_cast<T *>(obj);
#endif
        return serviceObj;
    }

    PluginClassBase *CreateObject(uint16_t interfaceID, const std::string &className, uint32_t &errorCode);
    PluginClassBase *CreateObject(uint16_t interfaceID, uint16_t serviceType,
                                  const std::map<std::string, AttrData> &capabilities,
                                  const PriorityScheme &priorityScheme, uint32_t &errorCode);
    uint32_t PluginServerGetClassInfo(uint16_t interfaceID, uint16_t serviceType,
                          const std::map<std::string, AttrData> &capabilities,
                          std::vector<ClassInfo> &classesInfo);
    PluginFWType AnalyzeFWType(const std::string &canonicalPath);

    PlatformAdp &platformAdp_;
    PluginFw &pluginFw_;
    GstPluginFw &gstPluginFw_;
};
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // PLUGIN_SERVER_H
