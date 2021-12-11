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

#include "test.h"

#include <codecvt>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "securec.h"
#include "utils/log.h"
#include "js_textdecoder.h"
#include "js_textencoder.h"

#define ASSERT_CHECK_CALL(call)   \
    {                             \
        ASSERT_EQ(call, napi_ok); \
    }

#define ASSERT_CHECK_VALUE_TYPE(env, value, type)               \
    {                                                           \
        napi_valuetype valueType = napi_undefined;              \
        ASSERT_TRUE(value != nullptr);                          \
        ASSERT_CHECK_CALL(napi_typeof(env, value, &valueType)); \
        ASSERT_EQ(valueType, type);                             \
    }


/* @tc.name: getEncodingTest001
 * @tc.desc: Test acquire encoding mode.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, getEncodingTest001, testing::ext::TestSize.Level0)
{
    HILOG_INFO("getEncodingTest001 start");
    napi_env env = (napi_env)engine_;

    OHOS::Util::TextEncoder textEncoder(env);
    napi_value result = textEncoder.GetEncoding();

    char *buffer = nullptr;
    size_t bufferSize = 0;
    napi_get_value_string_utf8(env, result, buffer, -1, &bufferSize);
    if (bufferSize > 0) {
        buffer = new char[bufferSize + 1];
        napi_get_value_string_utf8(env, result, buffer, bufferSize + 1, &bufferSize);
    }

    ASSERT_STREQ(buffer, "utf-8");
    if (buffer != nullptr) {
        delete []buffer;
        buffer = nullptr;
    }
}

/**
 * @tc.name: textEncodeTest001
 * @tc.desc: Test encode src.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, textEncodeTest001, testing::ext::TestSize.Level0)
{
    HILOG_INFO("getEncodingTest001 start");
    napi_env env = (napi_env)engine_;
    OHOS::Util::TextEncoder textEncoder(env);

    std::string input = "abc123";
    napi_value src = nullptr;
    napi_create_string_utf8(env, input.c_str(), input.size(), &src);
    napi_value result = textEncoder.Encode(src);

    char excepted[7] = {0x61, 0x62, 0x63, 0x31, 0x32, 0x33, 0};

    napi_typedarray_type type;
    size_t srcLength = 0;
    void* srcData = nullptr;
    napi_value srcBuffer = nullptr;
    size_t byteOffset = 0;

    napi_get_typedarray_info(
        env, result, &type, &srcLength, &srcData, &srcBuffer, &byteOffset);

    ASSERT_EQ(srcLength, 6);
    char* res = reinterpret_cast<char*>(srcData);

    res[srcLength] = 0;
    ASSERT_STREQ(res, excepted);
}

/**
 * @tc.name: textEncodeTest001
 * @tc.desc: Test encode src.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, textEncodeTest002, testing::ext::TestSize.Level0)
{
    HILOG_INFO("getEncodingTest001 start");
    napi_env env = (napi_env)engine_;
    OHOS::Util::TextEncoder textEncoder(env);

    std::string input = "";
    napi_value src = nullptr;
    napi_create_string_utf8(env, input.c_str(), input.size(), &src);
    napi_value result = textEncoder.Encode(src);

    napi_typedarray_type type;
    size_t srcLength = 0;
    void* srcData = nullptr;
    napi_value srcBuffer = nullptr;
    size_t byteOffset = 0;

    napi_get_typedarray_info(
        env, result, &type, &srcLength, &srcData, &srcBuffer, &byteOffset);

    ASSERT_STREQ((char*)srcData, nullptr);
}

/**
 * @tc.name: textEncodeIntoTest001
 * @tc.desc: Test returns a dictionary object indicating the progress of the encoding
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, textEncodeIntoTest001, testing::ext::TestSize.Level0)
{
    HILOG_INFO("textEncodeIntoTest001 start");
    napi_env env = (napi_env)engine_;
    OHOS::Util::TextEncoder textEncoder(env);

    std::string input = "abc123";
    napi_value src = nullptr;
    napi_create_string_utf8(env, input.c_str(), input.size(), &src);

    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 20;
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    napi_value dest = nullptr;
        napi_create_typedarray(env, napi_int8_array, arrayBufferSize, arrayBuffer, 0, &dest);

    napi_value result = textEncoder.EncodeInto(src, dest);

    napi_value read = nullptr;
    napi_get_named_property(env, result, "read", &read);

    uint32_t resRead = 0;

    napi_get_value_uint32(env, read, &resRead);

    napi_value written = nullptr;
    napi_get_named_property(env, result, "written", &written);

    uint32_t resWritten = 0;
    napi_get_value_uint32(env, read, &resWritten);

    ASSERT_EQ(resRead, (uint32_t)6);
    ASSERT_EQ(resWritten, (uint32_t)6);
}


/**
 * @tc.name: GetEncoding001
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, GetEncoding001, testing::ext::TestSize.Level0)
{
    HILOG_INFO("TextDecoder::getEncodingTest001 start");
    napi_env env = (napi_env)engine_;
    std::vector<int> inputVec;
    int fatal = -1;
    int ignoreBOM = -1;
    inputVec.push_back(fatal);
    inputVec.push_back(ignoreBOM);
    std::string str = "utf-8";
    OHOS::Util::TextDecoder textDecoder(env, str, inputVec);
    napi_value testString = textDecoder.GetEncoding();
    size_t bufferSize = 0;
    napi_get_value_string_utf8(env, testString, nullptr, 0, &bufferSize);
    std::string tmpTestStr = "utf-8";
    size_t strLength = 0;
    char* buffer = nullptr;
    if (bufferSize > 0) {
        buffer = new char[bufferSize + 1]();
        napi_get_value_string_utf8(env, testString, buffer, bufferSize + 1, &strLength);
    }
    const char *result = tmpTestStr.c_str();
    size_t resultLength = tmpTestStr.length();
    ASSERT_STREQ(result, buffer);
    ASSERT_EQ(resultLength, strLength);
    if (buffer != nullptr) {
        delete []buffer;
        buffer = nullptr;
    }
}

/**
 * @tc.name: GetFatal001
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, GetFatal001, testing::ext::TestSize.Level0)
{
    HILOG_INFO("TextDecoder::GetFatal001 start");
    napi_env env = (napi_env)engine_;
    std::vector<int> inputVec;
    int fatal = 1;
    int ignoreBOM = 0;
    inputVec.push_back(fatal);
    inputVec.push_back(ignoreBOM);
    std::string str = "utf-8";
    OHOS::Util::TextDecoder textDecoder(env, str, inputVec);
    napi_value naVal = textDecoder.GetFatal();
    bool result = false;
    napi_get_value_bool(env, naVal, &result);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: GetFatal002
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, GetFatal002, testing::ext::TestSize.Level0)
{
    HILOG_INFO("TextDecoder::GetFatal002 start");
    napi_env env = (napi_env)engine_;
    std::vector<int> inputVec;
    int fatal = -1;
    int ignoreBOM = 1;
    inputVec.push_back(fatal);
    inputVec.push_back(ignoreBOM);
    std::string str = "utf-8";
    OHOS::Util::TextDecoder textDecoder(env, str, inputVec);
    napi_value naVal = textDecoder.GetFatal();
    bool result = false;
    napi_get_value_bool(env, naVal, &result);
    ASSERT_FALSE(result);
}

/**
 * @tc.name: GetIgnoreBOM001
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, GetIgnoreBOM001, testing::ext::TestSize.Level0)
{
    HILOG_INFO("TextDecoder::GetIgnoreBOM001 start");
    napi_env env = (napi_env)engine_;
    std::vector<int> inputVec;
    int fatal = -1;
    int ignoreBOM = 1;
    inputVec.push_back(fatal);
    inputVec.push_back(ignoreBOM);
    std::string str = "utf-8";
    OHOS::Util::TextDecoder textDecoder(env, str, inputVec);
    napi_value naVal = textDecoder.GetIgnoreBOM();
    bool result = false;
    napi_get_value_bool(env, naVal, &result);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: decoderUtf8001 utf-8
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, decoderUtf8001, testing::ext::TestSize.Level0)
{
    HILOG_INFO("decoderUtf8001 start");
    napi_env env = (napi_env)engine_;
    std::vector<int> inputVec;
    int fatal = -1;
    int ignoreBOM = -1;
    inputVec.push_back(fatal);
    inputVec.push_back(ignoreBOM);
    std::string str = "utf-8";
    OHOS::Util::TextDecoder textDecoder(env, str, inputVec);
    bool iflag = false;
    size_t byteLength = 3;
    void* data = nullptr;
    napi_value resultBuff = nullptr;
    napi_create_arraybuffer(env, byteLength, &data, &resultBuff);
    unsigned char arr[3] = {0x61, 0x62, 0x63};
    int ret = memcpy_s(data, sizeof(arr), reinterpret_cast<void*>(arr), sizeof(arr));
    ASSERT_EQ(0, ret);
    napi_value result2 = nullptr;
    napi_create_typedarray(env, napi_int8_array, byteLength, resultBuff, 0, &result2);
    napi_value testString = textDecoder.Decode(result2, iflag);
    size_t bufferSize = 0;
    napi_get_value_string_utf8(env, testString, nullptr, 0, &bufferSize);
    size_t length = 0;
    char* ch = nullptr;
    if (bufferSize > 0) {
        ch = new char[bufferSize + 1]();
        napi_get_value_string_utf8(env, testString, ch, bufferSize + 1, &length);
    }
    ASSERT_STREQ("abc", ch);
    if (ch != nullptr) {
        delete []ch;
        ch = nullptr;
    }
}

/**
 * @tc.name: decoderUtf8002 utf-8
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, decoderUtf8002, testing::ext::TestSize.Level0)
{
    HILOG_INFO("decoderUtf8002 start");
    napi_env env = (napi_env)engine_;
    std::vector<int> inputVec;
    int fatal = -1;
    int ignoreBOM = 0;
    inputVec.push_back(fatal);
    inputVec.push_back(ignoreBOM);
    std::string str = "utf-8";
    OHOS::Util::TextDecoder textDecoder(env, str, inputVec);
    bool iflag = true;
    size_t byteLength = 3;
    void* data = nullptr;
    napi_value resultBuff = nullptr;
    napi_create_arraybuffer(env, byteLength, &data, &resultBuff);
    unsigned char arr[3] = {0x61, 0x62, 0x63};
    int ret = memcpy_s(data, sizeof(arr), reinterpret_cast<void*>(arr), sizeof(arr));
    ASSERT_EQ(0, ret);
    napi_value result2 = nullptr;
    napi_create_typedarray(env, napi_int8_array, byteLength, resultBuff, 0, &result2);
    napi_value testString = textDecoder.Decode(result2, iflag);
    size_t bufferSize = 0;
    size_t length = 0;
    napi_get_value_string_utf8(env, testString, nullptr, 0, &bufferSize);
    char* ch = nullptr;
    if (bufferSize > 0) {
        ch = new char[bufferSize + 1]();
        napi_get_value_string_utf8(env, testString, ch, bufferSize + 1, &length);
    }
    ASSERT_STREQ("abc", ch);
    if (ch != nullptr) {
        delete []ch;
        ch = nullptr;
    }
}

/**
 * @tc.name: decoderUtf16le001 utf-16le
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, decoderUtf16le001, testing::ext::TestSize.Level0)
{
    HILOG_INFO("decoderUtf16le001 start");
    napi_env env = (napi_env)engine_;
    std::vector<int> inputVec;
    int fatal = 0;
    int ignoreBOM = 0;
    inputVec.push_back(fatal);
    inputVec.push_back(ignoreBOM);
    std::string str = "utf-16le";
    OHOS::Util::TextDecoder textDecoder(env, str, inputVec);
    bool iflag = false;
    size_t byteLength = 6;
    void* data = nullptr;
    napi_value resultBuff = nullptr;
    napi_create_arraybuffer(env, byteLength, &data, &resultBuff);
    unsigned char arr[6] = {0x61, 0x00, 0x62, 0x00, 0x63, 0x00};
    int ret = memcpy_s(data, sizeof(arr), reinterpret_cast<void*>(arr), sizeof(arr));
    ASSERT_EQ(0, ret);
    napi_value result2 = nullptr;
    napi_create_typedarray(env, napi_int8_array, byteLength, resultBuff, 0, &result2);
    napi_value testString = textDecoder.Decode(result2, iflag);
    size_t bufferSize = 0;
    size_t length = 0;
    napi_get_value_string_utf8(env, testString, nullptr, 0, &bufferSize);
    char* ch = nullptr;
    if (bufferSize > 0) {
        ch = new char[bufferSize + 1]();
        napi_get_value_string_utf8(env, testString, ch, bufferSize + 1, &length);
    }
    ASSERT_STREQ("abc", ch);
    if (ch != nullptr) {
        delete []ch;
        ch = nullptr;
    }
}

/**
 * @tc.name: decoderUtf16le002 utf-16le
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, decoderUtf16le002, testing::ext::TestSize.Level0)
{
    HILOG_INFO("decoderUtf16le002 start");
    napi_env env = (napi_env)engine_;
    std::vector<int>  inputVec;
    int fatal = 0;
    int ignoreBOM = 1;
    inputVec.push_back(fatal);
    inputVec.push_back(ignoreBOM);
    std::string str = "utf-16le";
    OHOS::Util::TextDecoder textDecoder(env, str, inputVec);
    bool iflag = true;
    size_t byteLength = 6;
    void* data = nullptr;
    napi_value resultBuff = nullptr;
    napi_create_arraybuffer(env, byteLength, &data, &resultBuff);
    unsigned char arr[6] = {0x61, 0x00, 0x62, 0x00, 0x63, 0x00};
    int ret = memcpy_s(data, sizeof(arr), reinterpret_cast<void*>(arr), sizeof(arr));
    ASSERT_EQ(0, ret);
    napi_value result2 = nullptr;
    napi_create_typedarray(env, napi_int8_array, byteLength, resultBuff, 0, &result2);
    napi_value testString = textDecoder.Decode(result2, iflag);
    size_t bufferSize = 0;
    napi_get_value_string_utf8(env, testString, nullptr, 0, &bufferSize);
    char* ch = nullptr;
    size_t length = 0;
    if (bufferSize > 0) {
        ch = new char[bufferSize + 1]();
        napi_get_value_string_utf8(env, testString, ch, bufferSize + 1, &length);
    }
    ASSERT_STREQ("abc", ch);
    if (ch != nullptr) {
        delete []ch;
        ch = nullptr;
    }
}

/**
 * @tc.name: decoderUtf16le003 utf-16le
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, decoderUtf16le003, testing::ext::TestSize.Level0)
{
    HILOG_INFO("decoderUtf16le003 start");
    napi_env env = (napi_env)engine_;
    std::vector<int>  inputVec;
    int fatal = 0;
    int ignoreBOM = 0;
    inputVec.push_back(fatal);
    inputVec.push_back(ignoreBOM);
    std::string str = "utf-16le";
    OHOS::Util::TextDecoder textDecoder(env, str, inputVec);
    bool iflag = true;
    size_t byteLength = 8;
    void* data = nullptr;
    napi_value resultBuff = nullptr;
    napi_create_arraybuffer(env, byteLength, &data, &resultBuff);
    unsigned char arr[8] = {0xFF, 0xFE, 0x61, 0x00, 0x62, 0x00, 0x63, 0x00};
    int ret = memcpy_s(data, sizeof(arr), reinterpret_cast<void*>(arr), sizeof(arr));
    ASSERT_EQ(0, ret);
    napi_value result2 = nullptr;
    napi_create_typedarray(env, napi_int8_array, byteLength, resultBuff, 0, &result2);
    napi_value testString = textDecoder.Decode(result2, iflag);
    size_t bufferSize = 0;
    napi_get_value_string_utf8(env, testString, nullptr, 0, &bufferSize);
    char* ch = nullptr;
    size_t length = 0;
    std::string tempStr01 = "";
    if (bufferSize > 0) {
        ch = new char[bufferSize + 1]();
        napi_get_value_string_utf8(env, testString, ch, bufferSize + 1, &length);
        tempStr01 = ch;
    }
    std::u16string tempU16str02 =
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(tempStr01);
    ASSERT_EQ(0xFEFF, (int)tempU16str02[0]);
    ASSERT_EQ(0x61, (int)tempU16str02[1]);
    ASSERT_EQ(0x62, (int)tempU16str02[2]);
    ASSERT_EQ(0x63, (int)tempU16str02[3]);
    if (ch != nullptr) {
        delete []ch;
        ch = nullptr;
    }
}

/**
 * @tc.name: decoderUtf16le004 utf-16le
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, decoderUtf16le004, testing::ext::TestSize.Level0)
{
    HILOG_INFO("decoderUtf16le004 start");
    napi_env env = (napi_env)engine_;
    std::vector<int>  inputVec;
    int fatal = -1;
    int ignoreBOM = -1;
    inputVec.push_back(fatal);
    inputVec.push_back(ignoreBOM);
    std::string str = "utf-16le";
    OHOS::Util::TextDecoder textDecoder(env, str, inputVec);
    bool iflag = false;
    size_t byteLength = 8;
    void* data = nullptr;
    napi_value resultBuff = nullptr;
    napi_create_arraybuffer(env, byteLength, &data, &resultBuff);
    unsigned char arr[8] = {0xFF, 0xFE, 0x61, 0x00, 0x62, 0x00, 0x63, 0x00};
    int ret = memcpy_s(data, sizeof(arr), reinterpret_cast<void*>(arr), sizeof(arr));
    ASSERT_EQ(0, ret);
    napi_value result2 = nullptr;
    napi_create_typedarray(env, napi_int8_array, byteLength, resultBuff, 0, &result2);
    napi_value testString = textDecoder.Decode(result2, iflag);
    size_t bufferSize = 0;
    napi_get_value_string_utf8(env, testString, nullptr, 0, &bufferSize);
    char* ch = nullptr;
    size_t length = 0;
    std::string tempStr01 = "";
    if (bufferSize > 0) {
        ch = new char[bufferSize + 1]();
        napi_get_value_string_utf8(env, testString, ch, bufferSize + 1, &length);
        tempStr01 = ch;
    }
    std::u16string tempU16str02 =
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(tempStr01);
    ASSERT_EQ(0xFEFF, (int)tempU16str02[0]);
    ASSERT_EQ(0x61, (int)tempU16str02[1]);
    ASSERT_EQ(0x62, (int)tempU16str02[2]);
    ASSERT_EQ(0x63, (int)tempU16str02[3]);
    if (ch != nullptr) {
        delete []ch;
        ch = nullptr;
    }
}

/**
 * @tc.name: decoderUtf16be001 utf-16be
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, decoderUtf16be001, testing::ext::TestSize.Level0)
{
    HILOG_INFO("decoderUtf16be001 start");
    napi_env env = (napi_env)engine_;
    std::vector<int>  inputVec;
    int fatal = 0;
    int ignoreBOM = 0;
    inputVec.push_back(fatal);
    inputVec.push_back(ignoreBOM);
    std::string str = "utf-16be";
    OHOS::Util::TextDecoder textDecoder(env, str, inputVec);
    bool iflag = false;
    size_t byteLength = 6;
    void* data = nullptr;
    napi_value resultBuff = nullptr;
    napi_create_arraybuffer(env, byteLength, &data, &resultBuff);
    unsigned char arr[6] = {0x00, 0x61, 0x00, 0x62, 0x00, 0x63};
    int ret = memcpy_s(data, sizeof(arr), reinterpret_cast<void*>(arr), sizeof(arr));
    ASSERT_EQ(0, ret);
    napi_value result2 = nullptr;
    napi_create_typedarray(env, napi_int8_array, byteLength, resultBuff, 0, &result2);
    napi_value testString = textDecoder.Decode(result2, iflag);
    size_t bufferSize = 0;
    napi_get_value_string_utf8(env, testString, nullptr, 0, &bufferSize);
    size_t length = 0;
    char* ch = nullptr;
    if (bufferSize > 0) {
        ch = new char[bufferSize + 1]();
        napi_get_value_string_utf8(env, testString, ch, bufferSize + 1, &length);
    }
    ASSERT_STREQ("abc", ch);
    if (ch != nullptr) {
        delete []ch;
        ch = nullptr;
    }
}

/**
 * @tc.name: decoderUtf16be002 utf-16be
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, decoderUtf16be002, testing::ext::TestSize.Level0)
{
    HILOG_INFO("decoderUtf16be002 start");
    napi_env env = (napi_env)engine_;
    std::vector<int>  inputVec;
    int fatal = 0;
    int ignoreBOM = 0;
    inputVec.push_back(fatal);
    inputVec.push_back(ignoreBOM);
    std::string str = "utf-16be";
    OHOS::Util::TextDecoder textDecoder(env, str, inputVec);
    bool iflag = false;
    size_t byteLength = 8;
    void* data = nullptr;
    napi_value resultBuff = nullptr;
    napi_create_arraybuffer(env, byteLength, &data, &resultBuff);
    unsigned char arr[8] = {0xFE, 0xFF, 0x00, 0x61, 0x00, 0x62, 0x00, 0x63};
    int ret = memcpy_s(data, sizeof(arr), reinterpret_cast<void*>(arr), sizeof(arr));
    ASSERT_EQ(0, ret);
    napi_value result2 = nullptr;
    napi_create_typedarray(env, napi_int8_array, byteLength, resultBuff, 0, &result2);
    napi_value testString = textDecoder.Decode(result2, iflag);
    size_t bufferSize = 0;
    napi_get_value_string_utf8(env, testString, nullptr, 0, &bufferSize);
    size_t length = 0;
    char* ch = nullptr;
    std::string tempStr01 = "";
    if (bufferSize > 0) {
        ch = new char[bufferSize + 1]();
        napi_get_value_string_utf8(env, testString, ch, bufferSize + 1, &length);
        tempStr01 = ch;
    }
    std::u16string tempU16str02 =
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(tempStr01);
    ASSERT_EQ(0xFEFF, (int)tempU16str02[0]);
    ASSERT_EQ(0x61, (int)tempU16str02[1]);
    ASSERT_EQ(0x62, (int)tempU16str02[2]);
    ASSERT_EQ(0x63, (int)tempU16str02[3]);
    if (ch != nullptr) {
        delete []ch;
        ch = nullptr;
    }
}

/**
 * @tc.name: decoderUtf16be003 utf-16be
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, decoderUtf16be003, testing::ext::TestSize.Level0)
{
    HILOG_INFO("decoderUtf16be003 start");
    napi_env env = (napi_env)engine_;
    std::vector<int>  inputVec;
    int fatal = 0;
    int ignoreBOM = 1;
    inputVec.push_back(fatal);
    inputVec.push_back(ignoreBOM);
    std::string str = "utf-16be";
    OHOS::Util::TextDecoder textDecoder(env, str, inputVec);
    bool iflag = true;
    size_t byteLength = 8;
    void* data = nullptr;
    napi_value resultBuff = nullptr;
    napi_create_arraybuffer(env, byteLength, &data, &resultBuff);
    unsigned char arr[8] = {0xFE, 0xFF, 0x00, 0x61, 0x00, 0x62, 0x00, 0x63};
    int ret = memcpy_s(data, sizeof(arr), reinterpret_cast<void*>(arr), sizeof(arr));
    ASSERT_EQ(0, ret);
    napi_value result2 = nullptr;
    napi_create_typedarray(env, napi_int8_array, byteLength, resultBuff, 0, &result2);
    napi_value testString = textDecoder.Decode(result2, iflag);
    size_t bufferSize = 0;
    napi_get_value_string_utf8(env, testString, nullptr, 0, &bufferSize);
    size_t length = 0;
    char* ch = nullptr;
    std::string tempStr01 = "";
    if (bufferSize > 0) {
        ch = new char[bufferSize + 1]();
        napi_get_value_string_utf8(env, testString, ch, bufferSize + 1, &length);
        tempStr01 = ch;
    }
    std::u16string tempU16str02 =
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(tempStr01);
    ASSERT_EQ(0xFEFF, (int)tempU16str02[0]);
    ASSERT_EQ(0x61, (int)tempU16str02[1]);
    ASSERT_EQ(0x62, (int)tempU16str02[2]);
    ASSERT_EQ(0x63, (int)tempU16str02[3]);
    if (ch != nullptr) {
        delete []ch;
        ch = nullptr;
    }
}
