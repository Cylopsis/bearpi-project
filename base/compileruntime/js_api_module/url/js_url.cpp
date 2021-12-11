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
#include "js_url.h"
#include <cctype>
#include <regex>
#include <sstream>
#include "securec.h"
#include "utils/log.h"
namespace OHOS::Url {
    static std::map<std::string, int> g_head = {
        {"ftp:", 21}, {"file:", -1}, {"gopher:", 70}, {"http:", 80},
        {"https:", 443}, {"ws:", 80}, {"wss:", 443}
    };

    static std::vector<std::string> g_doubleSegment = {
        "..", ".%2e", ".%2E", "%2e.", "%2E.",
        "%2e%2e", "%2E%2E", "%2e%2E", "%2E%2e"
    };

    static std::vector<std::string> g_singlesegment = { ".", "%2e", "%2E" };

    static std::vector<char> g_specialcharacter = {
        '\0', '\t', '\n', '\r', ' ', '#', '%', '/', ':', '?',
        '@', '[', '\\', ']'
    };

    static void ReplaceSpecialSymbols(std::string& input, std::string& oldstr, std::string& newstr)
    {
        size_t oldlen = oldstr.size();
        while (true) {
            size_t pos = 0;
            if ((pos = input.find(oldstr)) != std::string::npos) {
                input.replace(pos, oldlen, newstr);
            } else {
                break;
            }
        }
    }

    template<typename T>
    bool IsASCIITabOrNewline(const T ch)
    {
        return (ch == '\t' || ch == '\n' || ch == '\r');
    }

    template<typename T>
    bool IsHexDigit(const T ch)
    {
        if (isdigit(ch) || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')) {
            return true;
        }
        return false;
    }

    static unsigned AsciiToHex(const unsigned char pram)
    {
        if (pram >= '0' && pram <= '9') {
            return pram - '0';
        }
        if (pram >= 'A' && pram <= 'F') {
            return pram - 'A' + 10; // 10:Convert to hexadecimal
        }
        if (pram >= 'a' && pram <= 'f') {
            return pram - 'a' + 10; // 10:Convert to hexadecimal
        }
        return static_cast<unsigned>(-1);
    }

    static std::string DecodePercent(const char *input, size_t len)
    {
        std::string temp;
        if (len == 0) {
            return temp;
        }
        temp.reserve(len);
        const char *it = input;
        const char *end = input + len;
        while (it < end) {
            const char ch = it[0];
            size_t left = end - it - 1;
            if (ch != '%' || left < 2 || (ch == '%' && (!IsHexDigit(it[1]) || // 2:The length is less than 2
                !IsHexDigit(it[2])))) { // 2:The number of characters is less than 2
                temp += ch;
                it++;
                continue;
            } else {
                unsigned first = AsciiToHex(it[1]);
                unsigned second = AsciiToHex(it[2]); // 2:Subscript 2
                char pram = static_cast<char>(first * 16 + second); // 16:Convert hex
                temp += pram;
                it += 3; // 3:Move the pointer to the right by 3 digits.
            }
        }
        return temp;
    }

    static void DeleteC0OrSpace(std::string& str)
    {
        if (str.empty()) {
            return;
        }
        size_t i = 0;
        size_t strlen = str.size();
        for (; i < strlen;) {
            if (str[i] >= '\0' && str[i] <= ' ') {
                i++;
            } else {
                break;
            }
        }
        str = str.substr(i);
        strlen = str.size();
        for (i = strlen - 1; i != 0; i--) {
            if (str[i] >= '\0' && str[i] <= ' ') {
                str.pop_back();
            } else {
                break;
            }
        }
    }

    static void DeleteTabOrNewline(std::string& str1)
    {
        for (auto item = str1.begin(); item != str1.end();) {
            if (IsASCIITabOrNewline(*item)) {
                item = str1.erase(item);
            } else {
                ++item;
            }
        }
    }

    static bool IsSpecial(std::string scheme)
    {
        auto temp = g_head.count(scheme);
        if (temp > 0) {
            return true;
        }
        return false;
    }

    static bool AnalysisScheme(std::string& input, std::string& scheme,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags)
    {
        if (!isalpha(input[0])) {
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
            return false;
        } else {
            size_t strlen = input.size();
            for (size_t i = 0; i < strlen - 1; ++i) {
                if ((isalnum(input[i]) || input[i] == '+' || input[i] == '-' || input[i] == '.') &&
                    isupper(input[i])) {
                        input[i] = tolower(input[i]);
                }
                if (!isalnum(input[i]) && input[i] != '+' && input[i] != '-' && input[i] != '.') {
                    flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
                    // 0:Bit 0 Set to true,The URL analysis failed
                    return false;
                }
            }
            scheme = input;
            if (IsSpecial(scheme)) {
                flags.set(static_cast<size_t>(BitsetStatusFlag::BIT1));
            }
            return true;
        }
    }

    static void AnalysisFragment(const std::string& input, std::string& fragment,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags)
    {
        fragment = input;
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT8));
    }

    static void AnalysisQuery(const std::string& input, std::string& query,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags)
    {
        query = input;
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT7));
    }
    static void AnalysisUsernameAndPasswd(std::string& input, std::string& username, std::string& password,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags)
    {
        int pos = input.size() - 1;
        for (; pos >= 0; pos--) {
            if (input[pos] == '@') {
                break;
            }
        }
        std::string userAndPasswd = input.substr(0, pos);
        input = input.substr(pos + 1);
        if (userAndPasswd.empty()) {
            return;
        }
        if (userAndPasswd.find('@') != std::string::npos) {
            while (true) {
                size_t i = 0;
                if ((i = userAndPasswd.find('@')) != std::string::npos) {
                    userAndPasswd = userAndPasswd.replace(i, 1, "%40");
                } else {
                    break;
                }
            }
        }

        if (userAndPasswd.find(':') != std::string::npos) {
            size_t i = userAndPasswd.find(':');
            std::string user = userAndPasswd.substr(0, i);
            std::string keyWord = userAndPasswd.substr(i + 1);
            if (!user.empty()) {
                username = user;
                flags.set(static_cast<size_t>(BitsetStatusFlag::BIT2));
            }
            if (!keyWord.empty()) {
                password = keyWord;
                flags.set(static_cast<size_t>(BitsetStatusFlag::BIT3));
            }
        } else {
            username = userAndPasswd;
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT2));
        }
    }

    static void AnalysisPath(std::string& input, std::vector<std::string>& path,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags, bool isSpecial)
    {
        std::vector<std::string> temp;
        size_t pos = 0;
        while (((pos = input.find('/')) != std::string::npos) ||
            ((pos = input.find('\\')) != std::string::npos && isSpecial)) {
            temp.push_back(input.substr(0, pos));
            input = input.substr(pos + 1);
        }
        temp.push_back(input);
        size_t length = temp.size();
        for (size_t it = 0; it < length; ++it) {
            auto result = find(g_doubleSegment.begin(), g_doubleSegment.end(), temp[it]);
            if (result != g_doubleSegment.end()) {
                if (path.empty() && it == length - 1) {
                    path.emplace_back("");
                    flags.set(static_cast<size_t>(BitsetStatusFlag::BIT6));
                }
                if (path.empty()) {
                    continue;
                }
                path.pop_back();
                if (it == length - 1) {
                    path.emplace_back("");
                    flags.set(static_cast<size_t>(BitsetStatusFlag::BIT6));
                }
                continue;
            }
            result = find(g_singlesegment.begin(), g_singlesegment.end(), temp[it]);
            if (result != g_singlesegment.end() && it == length - 1) {
                path.emplace_back("");
                flags.set(static_cast<size_t>(BitsetStatusFlag::BIT6));
                continue;
            }
            if (result == g_singlesegment.end()) {
                path.push_back(temp[it]);
                flags.set(static_cast<size_t>(BitsetStatusFlag::BIT6));
            }
        }
    }

    static void AnalysisPort(std::string input, UrlData& urlinfo,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags)
    {
        for (auto i : input) {
            if (!isdigit(i)) {
                flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
                return;
            }
        }
        int it = stoi(input);
        const int maxPort = 65535; // 65535:Maximum port number
        if (it > maxPort) {
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
            return;
        }
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT5));
        for (auto i : g_head) {
            if (i.first == urlinfo.scheme && i.second == it) {
                urlinfo.port = -1;
                flags.set(static_cast<size_t>(BitsetStatusFlag::BIT5), 0);
                return;
            }
        }
        urlinfo.port = it;
    }

    static void AnalysisOpaqueHost(std::string input, std::string& host,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags)
    {
        size_t strlen = input.size();
        for (size_t i = 0; i < strlen; ++i) {
            char ch = input[i];
            auto result = find(g_specialcharacter.begin(), g_specialcharacter.end(), ch);
            if (ch != '%' && (result != g_specialcharacter.end())) {
                flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
                return;
            }
        }
        host = input;
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT4));
    }

    static std::string IPv6ZeroComperess(std::vector<std::string>& tempIPV6, std::string& input,
        int maxZeroIndex, int max)
    {
        for (int i = 0; i < maxZeroIndex; ++i) {
            input += tempIPV6[i];
            if (i != maxZeroIndex - 1) {
                input += ":";
            }
        }
        input += "::";
        size_t strlen = tempIPV6.size();
        for (size_t i = maxZeroIndex + max; i < strlen; ++i) {
            input += tempIPV6[i];
            if (i != strlen - 1) {
                input += ":";
            }
        }
        return input;
    }

    static std::string IPv6NoComperess(std::vector<std::string>& tempIPV6, std::string& input)
    {
        size_t strlen = tempIPV6.size();
            for (size_t i = 0; i < strlen; ++i) {
                if (tempIPV6[i][0] == '?' && tempIPV6[i].size() == 1) {
                    input += ":";
                } else {
                    input += tempIPV6[i];
                    if (i != tempIPV6.size() - 1) {
                        input += ":";
                    }
                }
            }
        return input;
    }

    static std::string IPv6HostCompress(std::vector<std::string>& tempIPV6, int flag)
    {
        std::string input;
        if (flag == 1) {
            return IPv6NoComperess(tempIPV6, input);
        }
        int max = 0;
        int count = 0;
        size_t maxZeroIndex = 0;
        size_t strlen = tempIPV6.size();
        for (size_t i = 0; i < strlen;) {
            if (tempIPV6[i] == "0" && (i + 1 < strlen && tempIPV6[i + 1] == "0")) {
                int index = i;
                while (i < strlen && tempIPV6[i] == "0") {
                    i++;
                    count++;
                }
                if (max < count) {
                    max = count;
                    maxZeroIndex = index;
                }
            } else {
                count = 0;
                i++;
            }
        }
        if (count == 8) { // 8:If IPv6 is all 0
            return "::";
        } else if (max == 0) {
            strlen = tempIPV6.size();
            for (size_t i = 0; i < strlen; ++i) {
                input += tempIPV6[i];
                if (i != strlen - 1) {
                    input += ":";
                }
            }
            return input;
        } else if (maxZeroIndex == 0) {
            input += "::";
            strlen = tempIPV6.size();
            for (size_t i = max; i < strlen; ++i) {
                input += tempIPV6[i] + ":";
            }
            input.pop_back();
            return input;
        } else {
            return IPv6ZeroComperess(tempIPV6, input, maxZeroIndex, max);
        }
    }

    void DealWithtempIpv6(std::vector<std::string> &tempIpv6, std::stringstream &ss,
        std::string &numberHex, const int tempProt[4], int len)
    {
        tempIpv6.push_back(numberHex);
        ss.clear();
        numberHex.clear();
        if (len == 0) {
            return;
        }
        ss << std::hex << tempProt[2] * 0x100 + tempProt[3]; // 2: 3:subscript position
        ss >> numberHex;
        tempIpv6.push_back(numberHex);
        ss.clear();
        numberHex.clear();
        tempIpv6.erase(tempIpv6.end() - 3); // 3:Remove redundant space
    }

    void IPv6DealWithColon(int& flag, std::string& strInput,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags, size_t &pos)
    {
        flag = 1;
        if (strInput.find("::", pos + 2) != std::string::npos) { // 2:Subscript Move Right2
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
        }
        return;
    }

    void IsFlagExist(size_t &pos, std::vector<std::string> &temp, std::vector<std::string> &tempEnd,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> &flags, unsigned &numberFlag)
    {
        while (((pos = temp[numberFlag].find('.')) != std::string::npos)) {
            tempEnd.push_back(temp[numberFlag].substr(0, pos));
            temp[numberFlag] = temp[numberFlag].substr(pos + 1);
        }
        tempEnd.push_back(temp[numberFlag]);
        if (tempEnd.size() != 4) { // 4:The size is not 4
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
        }
    }
    void DealWithProt(std::vector<std::string> &tempEnd, unsigned &val,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags,
        int &number, int tempProt[4])
    {
        size_t strlen = tempEnd.size();
        for (size_t x = 0; x < strlen; ++x) {
            val = stoi(tempEnd[x]);
            if (val > 255) { // 255:The maximum value is 255
                flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
                return;
            }
            tempProt[number] = val;
            number++;
            val = 0;
        }
    }

    void DealWithElse(std::vector<std::string> &temp, size_t &i, unsigned &numberFlag,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags, unsigned &val)
    {
        size_t strlen = temp[i].size();
        for (size_t j = 0; j < strlen; ++j) {
            if (((temp[i].find('.')) != std::string::npos)) {
                numberFlag = i;
                if (temp.size() == i || temp.size() > 7) { // 7:The size cannot be greater than 7
                    flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
                    return;
                }
                return;
            } else if (IsHexDigit(temp[i][j])) {
                val = val * 0x10 + AsciiToHex(temp[i][j]);
            }
        }
    }

    void DealWithStringStream(std::stringstream &ss, unsigned &val,
        std::string &numberHex, std::vector<std::string> &tempIpv6)
    {
        ss << std::hex << val;
        ss >> numberHex;
        tempIpv6.push_back(numberHex);
        ss.clear();
        numberHex.clear();
        val = 0;
    }

    static void IPv6Host(std::string& input, std::string& host,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags)
    {
        if (input.size() == 0) {
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
            return;
        }
        std::string strInput = input;
        std::stringstream ss;
        std::string numberHex;
        unsigned val = 0;
        unsigned numberFlag = 0;
        std::vector<std::string> temp;
        std::vector<std::string> tempEnd;
        std::vector<std::string> tempIpv6;
        size_t pos = 0;
        int tempProt[4] = { 0 };
        int number = 0;
        int flag = 0;
        if ((pos = strInput.find("::", 0)) != std::string::npos) {
            IPv6DealWithColon(flag, strInput, flags, pos);
        }
        while (((pos = strInput.find(':')) != std::string::npos)) {
            temp.push_back(strInput.substr(0, pos));
            strInput = strInput.substr(pos + 1);
        }
        temp.push_back(strInput);
        if (temp.size() > 8) { // 8:The incoming value does not meet the criteria
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
            return;
        }
        size_t length = temp.size();
        for (size_t i = 0; i < length; ++i) {
            if (temp[i].empty()) {
                tempIpv6.push_back("?");
            } else {
                DealWithElse(temp, i, numberFlag, flags, val);
                DealWithStringStream(ss, val, numberHex, tempIpv6);
            }
        }
        if (numberFlag != 0) {
            IsFlagExist(pos, temp, tempEnd, flags, numberFlag);
            DealWithProt(tempEnd, val, flags, number, tempProt);
            ss << std::hex << tempProt[0] * 0x100 + tempProt[1];
            ss >> numberHex;
            DealWithtempIpv6(tempIpv6, ss, numberHex, tempProt, 4); // 4:tempProtlen
        }
        strInput = IPv6HostCompress(tempIpv6, flag);
        host = '[' + strInput + ']';
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT4));
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT10));
    }

    static bool CheckNunType(const char ch, const unsigned num)
    {
        enum class NUMERATION {
            OCT = 8, // 8:Octal
            DEC = 10, // 10:Decimal
            HEX = 16 // 16:Hexadecimal
        };
        if (NUMERATION(num) == NUMERATION::OCT) {
            if (ch < '0' || ch > '7') { // 0~7:Octal
                return false;
            }
        } else if (NUMERATION(num) == NUMERATION::DEC) {
            if (ch < '0' || ch > '9') { // 0~9:Decimal
                return false;
            }
        } else if (NUMERATION(num) == NUMERATION::HEX) {
            if (!((ch >= '0' && ch <= '9') ||  // 0~9, a~f, A~F:Hexadecimal
                (ch >= 'A' && ch <= 'F') ||
                (ch >= 'a' && ch <= 'f'))) {
                return false;
            }
        }
        return true;
    }

    static int64_t AnalyseNum(std::string parts)
    {
        unsigned num = 10; // 10:Decimal
        std::string partsBeg = parts.substr(0, 2); // 2:Take two digits to determine whether it is hexadecimal
        size_t partsLen = parts.length();
        if (partsLen >= 2 && (partsBeg == "0X" || partsBeg == "0x")) { // 2:parts length
            num = 16; // 16:Convert to hexadecimal
            parts = parts.substr(2); // 2:delete '0x'
        } else if (num == 10 && partsLen > 1 && parts.substr(0, 1) == "0") { // 10:Conversion to Decimal Coefficient
            num = 8; // 8:Convert to octal
            parts = parts.substr(1);
        }
        for (size_t i = 0; i < parts.length(); i++) {
            bool ret = CheckNunType(parts[i], num);
            if (!ret) {
                return -1;
            }
        }
        return strtoll(parts.c_str(), nullptr, num);
    }

    static bool OverHex(std::string input)
    {
        size_t size = input.size();
        for (size_t i = 0; i < size; i++) {
            return !IsHexDigit(input[i]);
        }
        return false;
    }

    static bool NotAllNum(std::string input)
    {
        size_t size = input.size();
        for (size_t i = 0; i < size; i++) {
            if (!isdigit(input[i])) {
                return true;
            }
        }
        return false;
    }

    static bool AnalyseIPv4(const char *instr, std::string &host,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> &flags)
    {
        int count = 0;
        for (const char *ptr = instr; *ptr != '\0'; ptr++) {
            if (*ptr == '.') {
                if (++count > 3) { // 3:The IPV4 address has only four segments
                    return false;
                }
            }
        }
        if (count != 3) { // 3:The IPV4 address has only four segments
            return false;
        }

        size_t pos = 0;
        std::vector<std::string> strVec;
        std::string input = static_cast<std::string>(instr);
        while (((pos = input.find('.')) != std::string::npos)) {
            strVec.push_back(input.substr(0, pos));
            input = input.substr(pos + 1);
        }
        strVec.push_back(input);
        size_t size = strVec.size();
        for (size_t i = 0; i < size; i++) {
            if (strVec[i].empty()) {
                return false;
            }
            std::string begStr = strVec[i].substr(0, 2); // 2:Intercept the first two characters
            if ((begStr == "0x" || begStr == "0X") && OverHex(strVec[i].substr(2))) { // 2:Intercept
                return false;
            } else if ((begStr == "0x" || begStr == "0X") && !(OverHex(strVec[i].substr(2)))) { // 2:Intercept
                continue;
            }
            if (NotAllNum(strVec[i])) {
                return false;
            }
        }
        for (size_t i = 0; i < size; i++) {
            int64_t value = AnalyseNum(strVec[i].c_str());
            if ((value < 0) || (value > 255)) { // 255:Only handle numbers between 0 and 255
                flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
                return false;
            } else {
                host += std::to_string(value);
                if (i != size - 1) {
                    host += ".";
                }
            }
        }
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT4));
        return true;
    }
    static void AnalysisHost(std::string& input, std::string& host,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags, bool special)
    {
        if (input.empty()) {
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
            return;
        }
        if (input[0] == '[') {
            if ((input[input.length() - 1]) == ']') {
                size_t  b = input.length();
                input = input.substr(1, b - 2); // 2:Truncating Strings
                IPv6Host(input, host, flags);
                return;
            } else {
                flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
                return;
            }
        }
        if (!special) {
            AnalysisOpaqueHost(input, host, flags);
            return;
        }
        std::string decodeInput = DecodePercent(input.c_str(), input.length());
        size_t strlen = decodeInput.size();
        for (size_t pos = 0; pos < strlen; ++pos) {
            char ch = decodeInput[pos];
            auto result = find(g_specialcharacter.begin(), g_specialcharacter.end(), ch);
            if (result != g_specialcharacter.end()) {
                flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
                return;
            }
        }
        bool ipv4 = AnalyseIPv4(decodeInput.c_str(), host, flags);
        if (ipv4) {
            return;
        }
        host = decodeInput;
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT4));
    }
    static bool ISFileNohost(const std::string& input)
    {
        if ((isalpha(input[0]) && (input[1] == ':' || input[1] == '|'))) {
            return true;
        }
        return false;
    }
    static void AnalysisFilePath(std::string& input, UrlData& urlinfo,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags)
    {
        std::vector<std::string> temp;
        size_t pos = 0;
        while (((pos = input.find('/')) != std::string::npos) || ((pos = input.find('\\')) != std::string::npos)) {
            temp.push_back(input.substr(0, pos));
            input = input.substr(pos + 1);
        }
        temp.push_back(input);
        size_t length = temp.size();
        for (size_t i = 0; i < length; ++i) {
            auto a = find(g_doubleSegment.begin(), g_doubleSegment.end(), temp[i]);
            if (a != g_doubleSegment.end()) {
                if ((urlinfo.path.size() == 1) && ISFileNohost(urlinfo.path[0]) &&
                    urlinfo.path[0].size() == 2) { // 2:The interception length is 2
                    urlinfo.path[0][1] = ':';
                } else if (!urlinfo.path.empty()) {
                    urlinfo.path.pop_back();
                }
                if (i == temp.size() - 1) {
                    urlinfo.path.push_back("");
                }
                continue;
            }
            a = find(g_singlesegment.begin(), g_singlesegment.end(), temp[i]);
            if (a != g_singlesegment.end()) {
                if (i == temp.size() - 1) {
                    urlinfo.path.push_back("");
                }
                continue;
            }
            urlinfo.path.push_back(temp[i]);
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT6));
        }
        std::string it = urlinfo.path[0];
        if (isalpha(it[0]) && (it[1] == ':' || it[1] == '|')) {
            if (it.size() == 2) { // 2:The length is 2
                it[1] = ':';
                flags.set(static_cast<size_t>(BitsetStatusFlag::BIT4), 0);
                urlinfo.host.clear();
            }
        }
    }

    static void AnalysisFile(std::string& input, UrlData& urlinfo,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags)
    {
        bool special = true;
        if ((input[0] == '/' || input[0] == '\\') && (input[1] == '/' || input[1] == '\\')) {
            std::string temp = input.substr(2); // 2:Intercept from 2 subscripts
            size_t pos = 0;
            if ((((pos = temp.find('/')) != std::string::npos) ||
                ((pos = temp.find('\\')) != std::string::npos)) && pos == 0) {
                temp = temp.substr(1);
                AnalysisFilePath(temp, urlinfo, flags);
            } else if ((((pos = temp.find('/')) != std::string::npos) ||
                ((pos = temp.find('\\')) != std::string::npos)) && pos != 0) {
                std::string strHost = temp.substr(0, pos);
                std::string strPath = temp.substr(pos + 1);
                if (!ISFileNohost(strHost)) {
                    AnalysisHost(strHost, urlinfo.host, flags, special);
                } else if (!ISFileNohost(strHost) && flags.test(static_cast<size_t>(BitsetStatusFlag::BIT0))) {
                    return;
                }
                if (!ISFileNohost(strHost)) {
                    AnalysisFilePath(strPath, urlinfo, flags);
                } else {
                    AnalysisFilePath(temp, urlinfo, flags);
                }
            } else {
                if (!temp.empty() && flags.test(static_cast<size_t>(BitsetStatusFlag::BIT0))) {
                    AnalysisHost(temp, urlinfo.host, flags, special);
                } else if (!temp.empty() && !flags.test(static_cast<size_t>(BitsetStatusFlag::BIT0))) {
                    AnalysisHost(temp, urlinfo.host, flags, special);
                    return;
                }
            }
        } else {
            if (input[0] == '/' || input[0] == '\\') {
                input = input.substr(1);
            }
            AnalysisFilePath(input, urlinfo, flags);
        }
    }

    static void AnalysisFilescheme(std::string& input, UrlData& urlinfo,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags)
    {
        std::string strPath = urlinfo.scheme + input;
        urlinfo.scheme = "file:";
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT1));
        AnalysisFilePath(strPath, urlinfo, flags);
    }

    void AnalyInfoPath(std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> &flags,
        UrlData& urlinfo, std::string& input)
    {
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT9));
        if (urlinfo.path.empty()) {
            urlinfo.path.emplace_back("");
        }
        urlinfo.path[0] = input;
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT6));
        return;
    }

    void AnalyHostPath(std::string &strHost, std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags,
        UrlData& urlinfo)
    {
        size_t pos = 0;
        if (strHost[strHost.size() - 1] != ']' && (pos = strHost.find_last_of(':')) != std::string::npos) {
            std::string port = strHost.substr(pos + 1);
            strHost = strHost.substr(0, pos);
            AnalysisPort(port, urlinfo, flags);
            if (flags.test(static_cast<size_t>(BitsetStatusFlag::BIT0))) {
                return;
            }
        }
    }
    void AnalyStrHost(std::string &strHost, UrlData& urlinfo,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> &flags)
    {
        if (strHost.find('@') != std::string::npos) {
            AnalysisUsernameAndPasswd(strHost, urlinfo.username, urlinfo.password, flags);
        }
        if (strHost.empty()) {
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
            return;
        }
    }

    static void AnalysisNoDefaultProtocol(std::string& input, UrlData& urlinfo,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags)
    {
        if (urlinfo.scheme.size() == 2) { // 2:The length is 2
            AnalysisFilescheme(input, urlinfo, flags);
            return;
        }
        if (input[0] == '/' && input[1] == '/') {
            std::string hostandpath = input.substr(2); // 2:Intercept from 2 subscripts
            if (hostandpath.empty()) {
                return;
            }
            size_t i = 0;
            bool special = false;
            if (hostandpath.find('/') != std::string::npos) {
                i = hostandpath.find('/');
                std::string strHost = hostandpath.substr(0, i);
                std::string strPath = hostandpath.substr(i + 1);
                if (strHost.find('@') != std::string::npos) {
                    AnalysisUsernameAndPasswd(strHost, urlinfo.username, urlinfo.password, flags);
                }
                if (strHost.empty()) {
                    flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
                    return;
                }
                size_t pos = 0;
                if (strHost[strHost.size() - 1] != ']' && (pos = strHost.find_last_of(':')) != std::string::npos) {
                    std::string port = strHost.substr(pos + 1);
                    strHost = strHost.substr(0, pos);
                    AnalysisPort(port, urlinfo, flags);
                }
                if (strHost[strHost.size() - 1] != ']' && (pos = strHost.find_last_of(':')) != std::string::npos &&
                    flags.test(static_cast<size_t>(BitsetStatusFlag::BIT0))) {
                    return;
                }
                AnalysisHost(strHost, urlinfo.host, flags, special);
                AnalysisPath(strPath, urlinfo.path, flags, special);
            } else {
                std::string strHost = hostandpath;
                AnalyStrHost(strHost, urlinfo, flags);
                AnalyHostPath(strHost, flags, urlinfo);
                AnalysisHost(strHost, urlinfo.host, flags, special);
            }
        } else if (input[1] == '/') {
            std::string strPath = input.substr(1);
            AnalysisPath(strPath, urlinfo.path, flags, false);
        } else {
            AnalyInfoPath(flags, urlinfo, input);
        }
    }

    static void AnalysisOnlyHost(std::string& input, UrlData& urlinfo,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags, size_t pos)
    {
        std::string strHost = input;
        if (strHost.find('@') != std::string::npos) {
            AnalysisUsernameAndPasswd(strHost, urlinfo.username, urlinfo.password, flags);
        }
        if (strHost.empty()) {
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
            return;
        }
        if (strHost[strHost.size() - 1] != ']') {
            if ((pos = strHost.find_last_of(':')) != std::string::npos) {
                std::string port = strHost.substr(pos + 1);
                strHost = strHost.substr(0, pos);
                AnalysisPort(port, urlinfo, flags);
            }
            if ((pos = strHost.find_last_of(':')) != std::string::npos &&
                flags.test(static_cast<size_t>(BitsetStatusFlag::BIT0))) {
                return;
            }
        }
        AnalysisHost(strHost, urlinfo.host, flags, true);
    }
    void JudgePos(size_t &pos, size_t &length, std::string& input)
    {
        for (pos = 0; pos < length; pos++) {
            if (input[pos] == '/' || input[pos] == '\\') {
                break;
            }
        }
    }
    static void AnalysisHostAndPath(std::string& input, UrlData& urlinfo,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags)
    {
        if (flags.test(static_cast<size_t>(BitsetStatusFlag::BIT1))) {
            size_t pos = 0;
            bool special = true;
            size_t inputLen = input.size();
            for (; pos < inputLen;) {
                if (input[pos] == '/' || input[pos] == '\\') {
                    pos++;
                } else {
                    break;
                }
            }
            input = input.substr(pos);
            if (input.size() == 0) {
                flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
                return;
            } else if (input.size() != 0 && (input.find('/') != std::string::npos ||
                input.find('\\') != std::string::npos)) {
                size_t length = input.size();
                JudgePos(pos, length, input);
                std::string strHost = input.substr(0, pos);
                std::string strPath = input.substr(pos + 1);
                if (strHost.find('@') != std::string::npos) {
                    AnalysisUsernameAndPasswd(strHost, urlinfo.username, urlinfo.password, flags);
                }
                if (strHost.empty()) {
                    flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
                    return;
                }
                if (strHost[strHost.size() - 1] != ']' && (pos = strHost.find_last_of(':')) != std::string::npos) {
                    std::string port = strHost.substr(pos + 1);
                    strHost = strHost.substr(0, pos);
                    AnalysisPort(port, urlinfo, flags);
                }
                if (strHost[strHost.size() - 1] != ']' && (pos = strHost.find_last_of(':')) != std::string::npos &&
                    flags.test(static_cast<size_t>(BitsetStatusFlag::BIT0))) {
                    return;
                }
                AnalysisHost(strHost, urlinfo.host, flags, special);
                AnalysisPath(strPath, urlinfo.path, flags, special);
            } else if (input.size() != 0 && input.find('/') == std::string::npos &&
                input.find('\\') == std::string::npos) {
                AnalysisOnlyHost(input, urlinfo, flags, pos);
            }
        } else {
            AnalysisNoDefaultProtocol(input, urlinfo, flags);
        }
    }

    static void AnalysisInput(std::string& input, UrlData& urlData,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags)
    {
        if (input.find('#') != std::string::npos) {
            size_t i = input.find('#');
            std::string fragment = input.substr(i);
            AnalysisFragment(fragment, urlData.fragment, flags);
            input = input.substr(0, i);
        }
        if (input.find('?') != std::string::npos) {
            size_t i = input.find('?');
            std::string query = input.substr(i);
            AnalysisQuery(query, urlData.query, flags);
            input = input.substr(0, i);
        }
        bool special = (flags.test(static_cast<size_t>(BitsetStatusFlag::BIT1)) ? true : false);
        AnalysisPath(input, urlData.path, flags, special);
    }

    static void BaseInfoToUrl(const UrlData& baseInfo,
        const std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> baseflags, UrlData& urlData,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags, bool inputIsEmpty)
    {
        urlData.scheme = baseInfo.scheme;
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT1),
            baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT1)));
        urlData.host = baseInfo.host;
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT4));
        urlData.username = baseInfo.username;
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT2),
            baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT2)));
        urlData.password = baseInfo.password;
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT3),
            baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT3)));
        urlData.port = baseInfo.port;
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT5),
            baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT5)));
        if (inputIsEmpty) {
            urlData.path = baseInfo.path;
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT6),
                baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT6)));
            urlData.query = baseInfo.query;
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT7),
                baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT7)));
            urlData.fragment = baseInfo.fragment;
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT8),
                baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT8)));
        }
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT9),
            baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT9)));
        flags.set(static_cast<size_t>(BitsetStatusFlag::BIT10),
            baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT10)));
    }

    static void ShorteningPath(UrlData& baseData, bool isFile)
    {
        if (baseData.path.empty()) {
            return;
        }
        if ((baseData.path.size() == 1) && isFile &&
            isalpha(baseData.path[0][0]) && (baseData.path[0][1] == ':')) {
            return;
        }
        baseData.path.pop_back();
    }

    URL::URL(napi_env env, const std::string& input)
    {
        env_ = env;
        std::string str = input;
        DeleteC0OrSpace(str);
        DeleteTabOrNewline(str);
        InitOnlyInput(str, urlData_, flags_);
    }

    void DelCont(std::string strBase, std::string &strInput, UrlData &baseInfo,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> &baseflags)
    {
        DeleteC0OrSpace(strBase);
        DeleteTabOrNewline(strBase);
        DeleteC0OrSpace(strInput);
        DeleteTabOrNewline(strInput);
        URL::InitOnlyInput(strBase, baseInfo, baseflags);
    }

    URL::URL(napi_env env, const std::string& input, const std::string& base)
    {
        env_ = env;
        UrlData baseInfo;
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> baseflags;
        std::string strBase = base;
        std::string strInput = input;
        if (strBase.empty()) {
            baseflags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
        }
        DelCont(strBase, strInput, baseInfo, baseflags);
        if (baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT0))) {
            flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
            return;
        } else if (!baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT0))) {
            InitOnlyInput(strInput, urlData_, flags_);
            if (!flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT0))) {
                return;
            }
            if ((input[0] == '/') && (input[1] == '/' || (input[1] == '\\' &&
                baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT1))))) {
                std::string newInput = baseInfo.scheme + input;
                flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT0), 0);
                InitOnlyInput(newInput, urlData_, flags_);
                return;
            }
            if (!baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT9))) {
                flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT0), 0);
                BaseInfoToUrl(baseInfo, baseflags, urlData_, flags_, input.empty());
                if (!input.empty() && input[0] == '/') {
                    strInput = input.substr(1);
                    AnalysisInput(strInput, urlData_, flags_);
                } else if (!input.empty() && input[0] != '/') {
                    AnalysisInput(strInput, urlData_, flags_);
                }
                if (!input.empty() && input[0] != '/' && urlData_.path.empty()) {
                    urlData_.path = baseInfo.path;
                    flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT6),
                        baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT6)));
                }
                if (!input.empty() && input[0] != '/' && !urlData_.path.empty()) {
                    bool isFile = ((urlData_.scheme == "file:") ? true : false);
                    ShorteningPath(baseInfo, isFile);
                    baseInfo.path.insert(baseInfo.path.end(), urlData_.path.begin(), urlData_.path.end());
                    urlData_.path = baseInfo.path;
                    flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT6));
                }
            } else if (baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT9))) {
                flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
                return;
            }
        }
    }

    URL::URL(napi_env env, const std::string& input, const URL& base)
    {
        env_ = env;
        std::string strInput = input;
        UrlData baseInfo = base.urlData_;
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> baseflags = base.flags_;
        DeleteC0OrSpace(strInput);
        DeleteTabOrNewline(strInput);
        InitOnlyInput(strInput, urlData_, flags_);
        if (!flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT0))) {
            return;
        }
        if ((input[0] == '/') && (input[1] == '/' || (input[1] == '\\' &&
            baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT1))))) {
            std::string newInput = baseInfo.scheme + input;
            flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT0), 0);
            InitOnlyInput(newInput, urlData_, flags_);
            return;
        }
        if (!baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT9))) {
            flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT0), 0);
            BaseInfoToUrl(baseInfo, baseflags, urlData_, flags_, input.empty());
            if (!input.empty() && input[0] == '/') {
                strInput = input.substr(1);
                AnalysisInput(strInput, urlData_, flags_);
            }
            if (!input.empty() && input[0] != '/') {
                AnalysisInput(strInput, urlData_, flags_);
                if (urlData_.path.empty()) {
                    urlData_.path = baseInfo.path;
                    flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT6),
                        baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT6)));
                } else {
                    bool isFile = ((urlData_.scheme == "file:") ? true : false);
                    ShorteningPath(baseInfo, isFile);
                    baseInfo.path.insert(baseInfo.path.end(), urlData_.path.begin(), urlData_.path.end());
                    urlData_.path = baseInfo.path;
                    flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT6));
                }
            }
        } else if (baseflags.test(static_cast<size_t>(BitsetStatusFlag::BIT9))) {
            flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
            return;
        }
    }

    napi_value URL::GetHostname() const
    {
        napi_value result;
        const char *temp = nullptr;
        if (flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT4))) {
            temp = urlData_.host.c_str();
        } else {
            temp = "";
        }
        size_t templen = strlen(temp);
        NAPI_CALL(env_, napi_create_string_utf8(env_, temp, templen, &result));
        return result;
    }

    napi_value URL::GetSearch() const
    {
        napi_value result;
        const char *temp = nullptr;
        if (flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT7))) {
            if (urlData_.query.size() == 1) {
                temp = "";
            } else {
                temp = urlData_.query.c_str();
            }
        } else {
            temp = "";
        }
        size_t templen = strlen(temp);
        NAPI_CALL(env_, napi_create_string_utf8(env_, temp, templen, &result));
        return result;
    }

    napi_value URL::GetUsername() const
    {
        napi_value result;
        const char *temp = nullptr;
        if (flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT2))) {
            temp = urlData_.username.c_str();
        } else
            temp = "";
        size_t templen = strlen(temp);
        NAPI_CALL(env_, napi_create_string_utf8(env_, temp, templen, &result));
        return result;
    }

    napi_value URL::GetPassword() const
    {
        napi_value result;
        const char *temp = nullptr;
        if (flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT3))) {
            temp = urlData_.password.c_str();
        } else {
            temp = "";
        }
        size_t templen = strlen(temp);
        NAPI_CALL(env_, napi_create_string_utf8(env_, temp, templen, &result));
        return result;
    }

    napi_value URL::GetFragment() const
    {
        napi_value result;
        const char *temp = nullptr;
        if (flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT8))) {
            if (urlData_.fragment.size() == 1) {
                temp = "";
            } else {
                temp = urlData_.fragment.c_str();
            }
        } else {
            temp = "";
        }
        size_t templen = strlen(temp);
        NAPI_CALL(env_, napi_create_string_utf8(env_, temp, templen, &result));
        return result;
    }

    napi_value URL::GetScheme() const
    {
        napi_value result;
        const char *temp = nullptr;
        if (!urlData_.scheme.empty()) {
            temp = urlData_.scheme.c_str();
        } else {
            temp = "";
        }
        size_t templen = strlen(temp);
        NAPI_CALL(env_, napi_create_string_utf8(env_, temp, templen, &result));
        return result;
    }

    napi_value URL::GetPath() const
    {
        napi_value result;
        std::string temp = "/";
        if (flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT6))) {
            size_t length = urlData_.path.size();
            for (size_t i = 0; i < length; i++) {
                if (i < length - 1) {
                    temp += urlData_.path[i] + "/";
                } else {
                    temp += urlData_.path[i];
                }
            }
        } else {
            bool special = IsSpecial(urlData_.scheme);
            if (!special) {
                temp = "";
            }
        }
        NAPI_CALL(env_, napi_create_string_utf8(env_, temp.c_str(), temp.size(), &result));
        return result;
    }


    napi_value URL::GetPort() const
    {
        napi_value result;
        const char *temp = nullptr;
        if (flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT5))) {
            temp = std::to_string(urlData_.port).c_str();
        } else {
            temp = "";
        }
        size_t templen = strlen(temp);
        NAPI_CALL(env_, napi_create_string_utf8(env_, temp, templen, &result));
        return result;
    }

    napi_value URL::GetHost() const
    {
        napi_value result;
        std::string temp1 = urlData_.host;
        if (flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT5))) {
            temp1 += ":";
            temp1 += std::to_string(urlData_.port);
        }
        NAPI_CALL(env_, napi_create_string_utf8(env_, temp1.c_str(), temp1.size(), &result));
        return result;
    }

    napi_value URL::GetOnOrOff() const
    {
        napi_value result;
        if (flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT0))) {
            bool flag = false;
            NAPI_CALL(env_, napi_get_boolean(env_, flag, &result));
        } else {
            bool flag = true;
            NAPI_CALL(env_, napi_get_boolean(env_, flag, &result));
        }
        return result;
    }

    napi_value URL::GetIsIpv6() const
    {
        napi_value result;
        if (flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT10))) {
            bool flag = true;
            NAPI_CALL(env_, napi_get_boolean(env_, flag, &result));
        } else {
            bool flag = false;
            NAPI_CALL(env_, napi_get_boolean(env_, flag, &result));
        }
        return result;
    }

    void URL::SetHostname(const std::string& input)
    {
        if (flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT9))) {
            return;
        }
        std::string strHost = input;
        size_t length = strHost.size();
        for (size_t pos = 0; pos < length; pos++) {
            if ((strHost[pos] == ':') || (strHost[pos] == '?') || (strHost[pos] == '#') ||
                (strHost[pos] == '/') || (strHost[pos] == '\\')) {
                strHost = strHost.substr(0, pos);
                break;
            }
        }
        if (strHost.size() == 0) {
            return;
        }
        bool special = IsSpecial(urlData_.scheme);
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> thisFlags;
        std::string thisHostname = "";
        AnalysisHost(strHost, thisHostname, thisFlags, special);
        if (thisFlags.test(static_cast<size_t>(BitsetStatusFlag::BIT4))) {
            if ((urlData_.scheme == "file:") && (thisHostname == "localhost")) {
                thisHostname = "";
            }
            urlData_.host = thisHostname;
            flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT4));
        }
    }

    void URL::SetHref(const std::string& input)
    {
        std::string str = input;
        DeleteC0OrSpace(str);
        DeleteTabOrNewline(str);
        UrlData thisNewUrl;
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> thisNewFlags;
        InitOnlyInput(str, thisNewUrl, thisNewFlags);
        if (!thisNewFlags.test(static_cast<size_t>(BitsetStatusFlag::BIT0))) {
            urlData_ = thisNewUrl;
            flags_ = thisNewFlags;
        }
    }

    void URL::SetPath(const std::string& input)
    {
        std::string strPath = input;
        if (flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT9))) {
            return;
        }
        if (strPath.empty()) {
            return;
        }
        std::string oldstr = "%3A";
        std::string newstr = ":";
        ReplaceSpecialSymbols(strPath, oldstr, newstr);
        bool special = IsSpecial(urlData_.scheme);
        if (urlData_.scheme == "file:") {
            UrlData thisFileDate;
            std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> thisFileFlag;
            if ((strPath[0] == '/') || (strPath[0] == '\\' &&
                flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT1)))) {
                strPath = strPath.substr(1);
            }
            AnalysisFilePath(strPath, thisFileDate, thisFileFlag);
            if (thisFileFlag.test(static_cast<size_t>(BitsetStatusFlag::BIT6))) {
                urlData_.path = thisFileDate.path;
                flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT6));
            }
        } else {
            std::vector<std::string> thisPath;
            std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> thisFlags;
            if ((strPath[0] == '/') || (strPath[0] == '\\' &&
                flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT1)))) {
                strPath = strPath.substr(1);
            }
            AnalysisPath(strPath, thisPath, thisFlags, special);
            if (thisFlags.test(static_cast<size_t>(BitsetStatusFlag::BIT6))) {
                urlData_.path = thisPath;
                flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT6));
            }
        }
    }

    void SplitString(const std::string& input, std::string& strHost, std::string& port)
    {
        size_t strlen = input.size();
        for (size_t pos = 0; pos < strlen; pos++) {
            if ((input[pos] == ':') || (input[pos] == '?') || (input[pos] == '#') ||
                (input[pos] == '/') || (input[pos] == '\\')) {
                strHost = input.substr(0, pos);
                if (input[pos] == ':') {
                    pos++;
                    port = input.substr(pos);
                }
                break;
            }
        }
    }

    void URL::SetHost(const std::string& input)
    {
        if (flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT9))) {
            return;
        }
        if (input.empty()) {
            return;
        }
        std::string strHost = input;
        std::string port = "";
        SplitString(input, strHost, port);
        if (strHost.size() == 0) {
            return;
        }
        bool special = IsSpecial(urlData_.scheme);
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> hostnameflags;
        std::string thisHostname = "";
        AnalysisHost(strHost, thisHostname, hostnameflags, special);
        if (hostnameflags.test(static_cast<size_t>(BitsetStatusFlag::BIT4))) {
            if ((urlData_.scheme == "file:") && (thisHostname == "localhost")) {
                thisHostname = "";
            }
            urlData_.host = thisHostname;
            flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT4));
        } else {
            return;
        }
        if (port.size() > 0) {
            size_t strlen = port.size();
            for (size_t pos = 0; pos < strlen; pos++) {
                if ((port[pos] == '?') || (port[pos] == '#') || (port[pos] == '/') || (port[pos] == '\\')) {
                    port = port.substr(0, pos);
                    break;
                }
            }
            if (port.size() > 0) {
                std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> thisFlags;
                UrlData thisport;
                AnalysisPort(port, thisport, thisFlags);
                if (thisFlags.test(static_cast<size_t>(BitsetStatusFlag::BIT5))) {
                    flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT5));
                    urlData_.port = thisport.port;
                }
            }
        }
    }

    void URL::SetPort(const std::string& input)
    {
        std::string port = input;
        size_t portlen = port.size();
        for (size_t pos = 0; pos < portlen; pos++) {
            if ((port[pos] == '?') || (port[pos] == '#') || (port[pos] == '/') || (port[pos] == '\\')) {
                port = port.substr(0, pos);
                break;
            }
        }
        if (port.size() > 0) {
            std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> thisFlags;
            UrlData thisport;
            AnalysisPort(port, thisport, thisFlags);
            if (thisFlags.test(static_cast<size_t>(BitsetStatusFlag::BIT5))) {
                flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT5));
                urlData_.port = thisport.port;
            }
        }
    }

    void URL::SetSearch(const std::string& input)
    {
        std::string temp;
        if (input.size() == 0) {
            urlData_.query = "";
            flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT7), 0);
        } else {
            if (input[0] != '?') {
                temp = "?";
                temp += input;
            } else {
                temp = input;
            }
            std::string oldstr = "#";
            std::string newstr = "%23";
            ReplaceSpecialSymbols(temp, oldstr, newstr);
            AnalysisQuery(temp, urlData_.query, flags_);
        }
    }

    void URL::SetFragment(const std::string& input)
    {
        std::string temp;
        if (input.size() == 0) {
            urlData_.fragment = "";
            flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT8), 0);
        } else {
            if (input[0] != '#') {
                temp = "#";
                temp += input;
            } else {
                temp = input;
            }
            AnalysisFragment(temp, urlData_.fragment, flags_);
        }
    }

    void URL::SetScheme(const std::string& input)
    {
        std::string strInput = input;
        bool special = IsSpecial(urlData_.scheme);
        bool inputIsSpecial = IsSpecial(input);
        if ((special != inputIsSpecial) || ((input == "file") &&
            (flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT2)) ||
            flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT3)) ||
            flags_.test(static_cast<size_t>(BitsetStatusFlag::BIT5))))) {
            return;
        }
        std::string thisScheme = "";
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)> thisFlags;
        if (AnalysisScheme(strInput, thisScheme, thisFlags)) {
            if (thisFlags.test(static_cast<size_t>(BitsetStatusFlag::BIT1))) {
                flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT1));
            }
            urlData_.scheme = thisScheme;
        }
    }

    void URL::SetUsername(const std::string& input)
    {
        if (input.size() == 0) {
            urlData_.username = "";
            flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT2), 0);
        } else {
            if (!input.empty()) {
                std::string usname = input;
                std::string oldstr = "@";
                std::string newstr = "%40";
                ReplaceSpecialSymbols(usname, oldstr, newstr);
                oldstr = "/";
                newstr = "%2F";
                ReplaceSpecialSymbols(usname, oldstr, newstr);
                urlData_.username = usname;
                flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT2));
            }
        }
    }

    void URL::SetPassword(const std::string& input)
    {
        if (input.size() == 0) {
            urlData_.password = "";
            flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT3), 0);
        } else {
            if (!input.empty()) {
                std::string keyWord = input;
                std::string oldstr = "@";
                std::string newstr = "%40";
                ReplaceSpecialSymbols(keyWord, oldstr, newstr);
                oldstr = "/";
                newstr = "%2F";
                ReplaceSpecialSymbols(keyWord, oldstr, newstr);
                urlData_.password = keyWord;
                flags_.set(static_cast<size_t>(BitsetStatusFlag::BIT3));
            }
        }
    }

    void URL::InitOnlyInput(std::string& input, UrlData& urlData,
        std::bitset<static_cast<size_t>(BitsetStatusFlag::BIT_STATUS_11)>& flags)
    {
        if (input.empty()) {
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
            return;
        }
        if (input.find(':') != std::string::npos) {
            size_t pos = input.find(':');
            pos++;
            std::string scheme = input.substr(0, pos);
            if (!AnalysisScheme(scheme, urlData.scheme, flags)) {
                return;
            }
            if (input.find('#') != std::string::npos) {
                size_t i = input.find('#');
                std::string fragment = input.substr(i);
                AnalysisFragment(fragment, urlData.fragment, flags);
                input = input.substr(0, i);
            }
            if (input.find('?') != std::string::npos) {
                size_t i = input.find('?');
                std::string query = input.substr(i);
                AnalysisQuery(query, urlData.query, flags);
                input = input.substr(0, i);
            }
            std::string str = input.substr(pos);
            if (urlData.scheme == "file:") {
                AnalysisFile(str, urlData, flags);
            } else {
                AnalysisHostAndPath(str, urlData, flags);
            }
        } else {
            flags.set(static_cast<size_t>(BitsetStatusFlag::BIT0));
            return;
        }
    }

    URLSearchParams::URLSearchParams(napi_env env) : env(env)
    {}
    std::wstring StrToWstr(const std::string& str)
    {
        char *p = setlocale(LC_ALL, "");
        if (p == nullptr) {
            return L"";
        }
        std::wstring wstr = L"";
        size_t len = str.size() + 1;
        if (len > 0) {
            auto wch = new wchar_t[len];
            mbstowcs(wch, str.c_str(), len);
            wstr = wch;
            delete[] wch;
            p = setlocale(LC_ALL, "");
            if (p == nullptr) {
            return L"";
            }
            return wstr;
        }
        return wstr;
    }

    bool IsEscapeRange(const char charaEncode)
    {
        if ((charaEncode > 0 && charaEncode < '*') ||
            (charaEncode > '*' && charaEncode < '-') ||
            (charaEncode == '/') ||
            (charaEncode > '9' && charaEncode < 'A') ||
            (charaEncode > 'Z' && charaEncode < '_') ||
            (charaEncode == '`') ||
            (charaEncode > 'z')) {
            return true;
        }
        return false;
    }

    size_t CharToUnicode(std::string str, size_t &i)
    {
        size_t bytOfSpeChar = 3; // 3:Bytes of special characters in Linux
        std::string subStr = str.substr(i, bytOfSpeChar);
        i += 2; // 2:Searching for the number and number of keys and values
        std::wstring wstr = StrToWstr(subStr.c_str());
        wchar_t wch = wstr[0];
        auto charaEncode = static_cast<size_t>(wch);
        return charaEncode;
    }
    std::string ReviseStr(std::string str, std::string *reviseChar)
    {
        const size_t lenStr = str.length();
        if (lenStr == 0) {
            return "";
        }
        std::string output = "";
        size_t numOfAscii = 128; // 128:Number of ASCII characters
        size_t i = 0;
        for (; i < lenStr; i++) {
            auto charaEncode = static_cast<size_t>(str[i]);
            if (charaEncode < 0 || charaEncode >= numOfAscii) {
                charaEncode = CharToUnicode(str, i);
            }
            if (charaEncode >= 0 && charaEncode < numOfAscii) {
                // 2:Defines the escape range of ASCII characters
                if (IsEscapeRange(charaEncode)) {
                    output += reviseChar[charaEncode];
                } else {
                    output += str.substr(i, 1);
                }
            } else if (charaEncode <= 0x000007FF) { // Convert the Unicode code into two bytes
                std::string output1 = reviseChar[0x000000C0 |
                    (charaEncode / 64)]; // 64:Acquisition method of the first byte
                std::string output2 = reviseChar[numOfAscii |
                    (charaEncode & 0x0000003F)]; // Acquisition method of the second byte
                output += output1 + output2;
            } else if ((charaEncode >= 0x0000E000) ||
                       (charaEncode <= 0x0000D7FF)) { // Convert the Unicode code into three bytes
                std::string output1 = reviseChar[0x000000E0 |
                    (charaEncode / 4096)]; // 4096:Acquisition method of the first byte
                std::string output2 = reviseChar[numOfAscii |
                    ((charaEncode / 64) & 0x0000003F)]; // 64:method of the second byte
                std::string output3 = reviseChar[numOfAscii |
                    (charaEncode & 0x0000003F)]; // Acquisition method of the third  byte
                output += output1 + output2 + output3;
            } else {
                const size_t charaEncode1 = static_cast<size_t>(str[++i]) & 1023; // 1023:Convert codes
                charaEncode = 65536 + (((charaEncode & 1023) << 10) |
                    charaEncode1); // 65536:Specific transcoding method
                std::string output1 = reviseChar[0x000000F0 |
                    (charaEncode / 262144)]; // 262144:Acquisition method of the first byte
                std::string output2 = reviseChar[numOfAscii |
                    ((charaEncode / 4096) & 0x0000003F)]; // 4096:Acquisition method of the second byte
                std::string output3 = reviseChar[numOfAscii |
                    ((charaEncode / 64) & 0x0000003F)]; // 64:Acquisition method of the third  byte
                std::string output4 = reviseChar[numOfAscii |
                    (charaEncode & 0x0000003F)]; // Acquisition method of the fourth   byte
                output += output1 + output2 + output3 + output4;
            }
        }
        return output;
    }

    napi_value URLSearchParams::ToString()
    {
        std::string output = "";
        std::string reviseChar[256] = {""}; // 256:Array length
        for (size_t i = 0; i < 256; ++i) { // 256:Array length
            size_t j = i;
            std::stringstream ioss;
            std::string str1 = "";
            ioss << std::hex << j;
            ioss >> str1;
            transform(str1.begin(), str1.end(), str1.begin(), ::toupper);
            if (i < 16) { // 16:Total number of 0-F
                reviseChar[i] = '%' + ("0" + str1);
            } else {
                reviseChar[i] = '%' + str1;
            }
        }
        reviseChar[0x20] = "+"; // 0x20:ASCII value of spaces
        const size_t lenStr = searchParams.size();
        if (lenStr == 0) {
            napi_value result = nullptr;
            napi_create_string_utf8(env, output.c_str(), output.size(), &result);
            return result;
        }
        std::string firstStrKey = ReviseStr(searchParams[0], reviseChar);
        std::string firstStrValue = ReviseStr(searchParams[1], reviseChar);
        output = firstStrKey + "=" + firstStrValue;
        if (lenStr % 2 == 0) { // 2:Divisible by 2
            size_t i = 2; // 2:Initial Position
            for (; i < lenStr; i += 2) { // 2:Searching for the number and number of keys and values
                std::string strKey = ReviseStr(searchParams[i], reviseChar);
                std::string strValue = ReviseStr(searchParams[i + 1], reviseChar);
                output += +"&" + strKey + "=" + strValue;
            }
        }
        napi_value result = nullptr;
        napi_create_string_utf8(env, output.c_str(), output.size(), &result);
        return result;
    }
    void URLSearchParams::HandleIllegalChar(std::wstring& inputStr, std::wstring::const_iterator it)
    {
        std::wstring::iterator iter = inputStr.begin();
        advance(iter, std::distance<std::wstring::const_iterator>(iter, it));
        while (iter != inputStr.end()) {
            char16_t ch = *iter;
            if (!((ch & 0xF800) == 0xD800)) {
                ++iter;
                continue;
            } else if ((ch & 0x400) != 0 || iter == inputStr.end() - 1) {
                *iter = 0xFFFD;
            } else {
                char16_t dh = *(iter + 1);
                if ((dh & 0xFC00) == 0xDC00) {
                    ++iter;
                } else {
                    *iter = 0xFFFD;
                }
            }
            ++iter;
        }
    }
    std::string URLSearchParams::ToUSVString(std::string inputStr)
    {
        size_t strLen = strlen(inputStr.c_str());
        wchar_t *strPtr = nullptr;
        std::wstring winput = L"";
        int strSize = mbstowcs(strPtr, inputStr.c_str(), 0) + 1;
        if (strSize > 0) {
            strPtr = new wchar_t[strSize];
            mbstowcs(strPtr, inputStr.c_str(), strLen);
            winput = strPtr;
        }
        const char *expr = "(?:[^\\uD800-\\uDBFF]|^)[\\uDC00-\\uDFFF]|[\\uD800-\\uDBFF](?![\\uDC00-\\uDFFF])";
        size_t exprLen = strlen(expr);
        wchar_t *exprPtr = nullptr;
        int exprSize = mbstowcs(exprPtr, expr, 0) + 1;
        if (exprSize > 0) {
            exprPtr = new wchar_t[exprSize];
            mbstowcs(exprPtr, expr, exprLen);
        }
        std::wregex wexpr(exprPtr);
        delete[] exprPtr;
        std::wsmatch result;
        delete[] strPtr;
        std::wstring::const_iterator iterStart = winput.begin();
        std::wstring::const_iterator iterEnd = winput.end();
        if (!regex_search(iterStart, iterEnd, result, wexpr)) {
            return inputStr;
        }
        HandleIllegalChar(winput, result[0].first);
        size_t inputLen = wcslen(winput.c_str());
        char *rePtr = nullptr;
        std::string reStr = "";
        int reSize = wcstombs(rePtr, winput.c_str(), 0) + 1;
        if (reSize > 0) {
            rePtr = new char[reSize];
            if (memset_s(rePtr, reSize, 0, reSize) != 0) {
                HILOG_ERROR("ToUSVString memset_s failed");
                delete[] rePtr;
                return reStr;
            } else {
                wcstombs(rePtr, winput.c_str(), inputLen);
                reStr = rePtr;
            }
        }
        delete[] rePtr;
        return reStr;
    }
    napi_value URLSearchParams::Get(napi_value buffer)
    {
        char *name = nullptr;
        size_t nameSize = 0;
        std::string temp = "";
        napi_get_value_string_utf8(env, buffer, nullptr, 0, &nameSize);
        if (nameSize > 0) {
            name = new char[nameSize + 1];
            napi_get_value_string_utf8(env, buffer, name, nameSize + 1, &nameSize);
            temp = name;
        }
        std::string sname = ToUSVString(temp);
        delete[] name;
        napi_value result = nullptr;
        if (searchParams.size() == 0) {
            return result;
        }
        size_t size = searchParams.size() - 1;
        for (size_t i = 0; i < size; i += 2) { // 2:Searching for the number and number of keys and values
            if (searchParams[i] == sname) {
                std::string str = searchParams[i + 1];
                napi_create_string_utf8(env, searchParams[i + 1].c_str(), searchParams[i + 1].length(), &result);
                return result;
            }
        }
        return result;
    }
    napi_value URLSearchParams::GetAll(napi_value buffer)
    {
        char *name = nullptr;
        size_t nameSize = 0;
        std::string sname = "";
        napi_get_value_string_utf8(env, buffer, nullptr, 0, &nameSize);
        if (nameSize > 0) {
            name = new char[nameSize + 1];
            napi_get_value_string_utf8(env, buffer, name, nameSize + 1, &nameSize);
            sname = ToUSVString(name);
        }
        delete[] name;
        napi_value result = nullptr;
        napi_value napiStr = nullptr;
        NAPI_CALL(env, napi_create_array(env, &result));
        size_t flag = 0;
        if (searchParams.size() == 0) {
            return result;
        }
        size_t size = searchParams.size() - 1;
        for (size_t i = 0; i < size; i += 2) { // 2:Searching for the number and number of keys and values
            if (searchParams[i] == sname) {
                napi_create_string_utf8(env, searchParams[i + 1].c_str(), searchParams[i + 1].length(), &napiStr);
                napi_status status = napi_set_element(env, result, flag, napiStr);
                if (status != napi_ok) {
                    HILOG_INFO("set element error");
                }
                flag++;
            }
        }
        return result;
    }
    void URLSearchParams::Append(napi_value buffer, napi_value temp)
    {
        char *name = nullptr;
        size_t nameSize = 0;
        std::string tempName = "";
        std::string tempValue = "";
        napi_get_value_string_utf8(env, buffer, nullptr, 0, &nameSize);
        if (nameSize > 0) {
            name = new char[nameSize + 1];
            napi_get_value_string_utf8(env, buffer, name, nameSize + 1, &nameSize);
            tempName = name;
        }
        char *value = nullptr;
        size_t valueSize = 0;
        napi_get_value_string_utf8(env, temp, nullptr, 0, &valueSize);
        if (valueSize > 0) {
            value = new char[valueSize + 1];
            napi_get_value_string_utf8(env, temp, value, valueSize + 1, &valueSize);
            tempValue = value;
        }
        searchParams.push_back(tempName);
        searchParams.push_back(tempValue);
        delete[] name;
        delete[] value;
    }
    void URLSearchParams::Delete(napi_value buffer)
    {
        char *name = nullptr;
        size_t nameSize = 0;
        std::string sname = "";
        napi_get_value_string_utf8(env, buffer, nullptr, 0, &nameSize);
        if (nameSize > 0) {
            name = new char[nameSize + 1];
            napi_get_value_string_utf8(env, buffer, name, nameSize + 1, &nameSize);
            sname = ToUSVString(name);
        }
        delete[] name;
        for (auto iter = searchParams.begin(); iter != searchParams.end();) {
            if (*iter == sname) {
                iter = searchParams.erase(iter, iter + 2); // 2:Searching for the number and number of keys and values
            } else {
                iter += 2; // 2:Searching for the number and number of keys and values
            }
        }
    }
    napi_value URLSearchParams::Entries() const
    {
        napi_value resend = nullptr;
        napi_value firNapiStr = nullptr;
        napi_value secNapiStr = nullptr;
        napi_create_array(env, &resend);
        if (searchParams.size() == 0) {
            return resend;
        }
        size_t size = searchParams.size() - 1;
        for (size_t i = 0; i < size; i += 2) { // 2:Searching for the number and number of keys and values
            napi_value result = nullptr;
            napi_create_array(env, &result);

            napi_create_string_utf8(env, searchParams[i].c_str(), searchParams[i].length(), &firNapiStr);
            napi_create_string_utf8(env, searchParams[i + 1].c_str(), searchParams[i + 1].length(), &secNapiStr);
            napi_set_element(env, result, 0, firNapiStr);
            napi_set_element(env, result, 1, secNapiStr);
            napi_set_element(env, resend, i / 2, result); // 2:Find the number of keys
        }
        return resend;
    }
    void URLSearchParams::ForEach(napi_value function, napi_value thisVar)
    {
        if (searchParams.size() == 0) {
            return;
        }
        size_t size = searchParams.size() - 1;
        for (size_t i = 0; i < size; i += 2) { // 2:Searching for the number and number of keys and values
            napi_value returnVal = nullptr;
            size_t argc = 3;
            napi_value global = nullptr;
            napi_get_global(env, &global);
            napi_value key = nullptr;
            napi_create_string_utf8(env, searchParams[i].c_str(), strlen(searchParams[i].c_str()), &key);
            napi_value value = nullptr;
            napi_create_string_utf8(env, searchParams[i + 1].c_str(), strlen(searchParams[i + 1].c_str()), &value);
            napi_value argv[3] = {key, value, thisVar};
            napi_call_function(env, global, function, argc, argv, &returnVal);
        }
    }
    napi_value URLSearchParams::IsHas(napi_value name) const
    {
        char *buffer = nullptr;
        size_t bufferSize = 0;
        std::string buf = "";
        napi_get_value_string_utf8(env, name, nullptr, 0, &bufferSize);
        if (bufferSize > 0) {
            buffer = new char[bufferSize + 1];
            napi_get_value_string_utf8(env, name, buffer, bufferSize + 1, &bufferSize);
            buf = buffer;
        }
        bool flag = false;
        napi_value result = nullptr;
        size_t lenStr = searchParams.size();
        for (size_t i = 0; i != lenStr; i += 2) { // 2:Searching for the number and number of keys and values
            if (searchParams[i] == buf) {
                flag = true;
                napi_get_boolean(env, flag, &result);
                return result;
            }
        }
        delete []buffer;
        napi_get_boolean(env, flag, &result);
        return result;
    }
    void URLSearchParams::Set(napi_value name, napi_value value)
    {
        char *buffer0 = nullptr;
        size_t bufferSize0 = 0;
        std::string cppName = "";
        std::string cppValue = "";
        napi_get_value_string_utf8(env, name, nullptr, 0, &bufferSize0);
        if (bufferSize0 > 0) {
            buffer0 = new char[bufferSize0 + 1];
            napi_get_value_string_utf8(env, name, buffer0, bufferSize0 + 1, &bufferSize0);
            cppName = buffer0;
            delete[] buffer0;
        }
        char *buffer1 = nullptr;
        size_t bufferSize1 = 0;
        napi_get_value_string_utf8(env, value, nullptr, 0, &bufferSize1);
        if (bufferSize1 > 0) {
            buffer1 = new char[bufferSize1 + 1];
            napi_get_value_string_utf8(env, value, buffer1, bufferSize1 + 1, &bufferSize1);
            cppValue = buffer1;
            delete[] buffer1;
        }
        bool flag = false;
        for (auto it = searchParams.begin(); it < (searchParams.end() - 1);) {
            if (*it == cppName) {
                if (!flag) {
                    *(it + 1) = cppValue;
                    flag = true;
                    it += 2; // 2:Searching for the number and number of keys and values
                } else {
                    it = searchParams.erase(it, it + 2); // 2:Searching for the number and number of keys and values
                }
            } else {
                it += 2; // 2:Searching for the number and number of keys and values
            }
        }
        if (!flag) {
            searchParams.push_back(cppName);
            searchParams.push_back(cppValue);
        }
    }
    void URLSearchParams::Sort()
    {
        unsigned int len = searchParams.size();
        if (len <= 2 && (len % 2 != 0)) { // 2: Iterate over key-value pairs
            return;
        }
        unsigned int i = 0;
        for (; i < len - 2; i += 2) { // 2:Iterate over key-value pairs
            unsigned int  j = i + 2; // 2:Iterate over key-value pairs
            for (; j < len; j += 2) { // 2:Iterate over key-value pairs
                bool tmp = (searchParams[i] > searchParams[j]);
                if (tmp) {
                    const std::string curKey = searchParams[i];
                    const std::string curVal = searchParams[i + 1];
                    searchParams[i] = searchParams[j];
                    searchParams[i + 1] = searchParams[j + 1];
                    searchParams[j] = curKey;
                    searchParams[j + 1] = curVal;
                }
            }
        }
    }
    napi_value URLSearchParams::IterByKeys()
    {
        std::vector<std::string> toKeys;
        napi_value result = nullptr;
        napi_value napiStr = nullptr;
        napi_create_array(env, &result);
        size_t stepSize = 2; // 2:Searching for the number and number of keys and values
        size_t lenStr = searchParams.size();
        if (lenStr % 2 == 0) { // 2:Get the number of values
            for (auto it = searchParams.begin(); it != searchParams.end(); it += stepSize) {
                toKeys.push_back(*it);
            }
            size_t lenToKeys = toKeys.size();
            for (size_t i = 0; i < lenToKeys; i++) {
                napi_create_string_utf8(env, toKeys[i].c_str(), toKeys[i].length(), &napiStr);
                napi_set_element(env, result, i, napiStr);
            }
        }
        return result;
    }
    napi_value URLSearchParams::IterByValues()
    {
        std::vector<std::string> toKeys;
        napi_value result = nullptr;
        napi_value napiStr = nullptr;
        napi_create_array(env, &result);
        size_t stepSize = 2; // 2:Searching for the number and number of keys and values
        size_t lenStr = searchParams.size();
        if (lenStr % 2 == 0) { // 2:Get the number of values
            for (auto it = searchParams.begin();
                it != searchParams.end();
                it += stepSize) {
                toKeys.push_back(*(it + 1));
            }
            size_t lenToKeys = toKeys.size();
            for (size_t i = 0; i < lenToKeys; i++) {
                napi_create_string_utf8(env, toKeys[i].c_str(), toKeys[i].length(), &napiStr);
                napi_set_element(env, result, i, napiStr);
            }
        }
        return result;
    }
    void URLSearchParams::SetArray(const std::vector<std::string> vec)
    {
        searchParams = vec;
    }
    napi_value URLSearchParams::GetArray() const
    {
        napi_value arr = nullptr;
        napi_create_array(env, &arr);
        size_t length = searchParams.size();
        for (size_t i = 0; i < length; i++) {
            napi_value result = nullptr;
            napi_create_string_utf8(env, searchParams[i].c_str(), searchParams[i].size(), &result);
            napi_set_element(env, arr, i, result);
        }
        return arr;
    }
} // namespace