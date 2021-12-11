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

#include <gtest/gtest.h>
#include <iostream>
#include "abs_image_detector.h"
#include "hilog/log.h"
#include "log_tags.h"
#include "plugin_server.h"

using OHOS::DelayedRefSingleton;
using std::string;
using std::vector;
using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::MultimediaPlugin;
using namespace OHOS::PluginExample;

static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "PluginManagerTest" };

class PluginManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static uint32_t DoTestRegister003(OHOS::MultimediaPlugin::PluginServer &pluginServer);
    static uint32_t DoTestRegister004(OHOS::MultimediaPlugin::PluginServer &pluginServer);
    static uint32_t DoTestInstanceLimit001(OHOS::MultimediaPlugin::PluginServer &pluginServer);
    static uint32_t DoTestInstanceLimit003(OHOS::MultimediaPlugin::PluginServer &pluginServer);
};

void PluginManagerTest::SetUpTestCase(void)
{}

void PluginManagerTest::TearDownTestCase(void)
{}

void PluginManagerTest::SetUp(void)
{}

void PluginManagerTest::TearDown(void)
{}

/**
 * @tc.name: TestRegister001
 * @tc.desc: Verify that the plugin management module supports the basic scenario of
 *           registering and managing one plugin package in one directory.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestRegister001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register one directory with one plugin package.
     * @tc.expected: step1. The directory was registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin/testplugins" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. Create a plugin object by class name from the plugin package.
     * @tc.expected: step2. The plugin object was created successfully.
     */
    uint32_t errorCode;
    string implClassName = "OHOS::PluginExample::CloudLabelDetector";
    AbsImageDetector *cLabelDetector = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    ASSERT_NE(cLabelDetector, nullptr);

    /**
     * @tc.steps: step3. Call the member function of the plugin object.
     * @tc.expected: step3. The member function of the plugin object can be called normally,
     *                      and the execution result is correct.
     */
    cLabelDetector->Prepare();
    string result = cLabelDetector->Process();
    delete cLabelDetector;
    EXPECT_EQ(result, "CloudLabelDetector");
}

/**
 * @tc.name: TestRegister002
 * @tc.desc: Verify that the plugin management module supports the basic scenario of
 *           registering and managing multiple plugins in one directory.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestRegister002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register one directory with two plugin packages.
     * @tc.expected: step1. The directory was registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. Create a plugin object by class name from the first plugin package.
     * @tc.expected: step2. The plugin object was created successfully.
     */
    string implClassName = "OHOS::PluginExample::CloudLabelDetector2";
    uint32_t errorCode;
    AbsImageDetector *cLabelDetector2 = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    ASSERT_NE(cLabelDetector2, nullptr);

    /**
     * @tc.steps: step3. Call the member function of the plugin object from the first plugin package.
     * @tc.expected: step3. The member function of the plugin object can be called normally,
     *                      and the execution result is correct.
     */
    cLabelDetector2->Prepare();
    string result = cLabelDetector2->Process();
    delete cLabelDetector2;
    EXPECT_EQ(result, "CloudLabelDetector2");

    /**
     * @tc.steps: step4. Create a plugin object by class name from the second plugin package.
     * @tc.expected: step4. The plugin object was created successfully.
     */
    implClassName = "OHOS::PluginExample::LabelDetector3";
    AbsImageDetector *labelDetector3 = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    ASSERT_NE(labelDetector3, nullptr);

    /**
     * @tc.steps: step5. Call the member function of the plugin object from the second plugin package.
     * @tc.expected: step5. The member function of the plugin object can be called normally,
     *                      and the execution result is correct.
     */
    labelDetector3->Prepare();
    result = labelDetector3->Process();
    delete labelDetector3;
    EXPECT_EQ(result, "LabelDetector3");
}

/**
 * @tc.name: TestRegister003
 * @tc.desc: Verify that the plugin management module supports registration of
 *           multiple directories not contain each other.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestRegister003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register two non-inclusive directories that contain a total of three plugin packages.
     * @tc.expected: step1. The directories were registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin/testplugins",
                                   "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. Test registered plugin packages can be used normally.
     * @tc.expected: step2. Plugin objects can be created correctly from the three registered plugin packages.
     */
    ASSERT_EQ(DoTestRegister003(pluginServer), SUCCESS);
}

/**
 * @tc.name: TestRegister004
 * @tc.desc: Verify that the plugin management module supports the registration of
 *           multiple directories with duplicate relationships.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestRegister004, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register three directories with duplicate relationships.
     * @tc.expected: step1. The directories were registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin/testplugins2", "/system/etc/multimediaplugin",
                                   "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. Test registered plugin packages can be used normally.
     * @tc.expected: step2. Plugin objects can be created correctly from registered plugin packages.
     */
    ASSERT_EQ(DoTestRegister004(pluginServer), SUCCESS);
}

/**
 * @tc.name: TestRegister005
 * @tc.desc: Verify that the plugin management module supports managing
 *           multiple plugin classes in a plugin package.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestRegister005, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register one plugin packages with two plugin classes.
     * @tc.expected: step1. The directory was registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin/testplugins" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. Create a plugin object by class name from the first plugin class.
     * @tc.expected: step2. The plugin object was created successfully.
     */
    uint32_t errorCode;
    string implClassName = "OHOS::PluginExample::LabelDetector";
    AbsImageDetector *labelDetector = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    ASSERT_NE(labelDetector, nullptr);

    /**
     * @tc.steps: step3. Call the member function of the plugin object from the first plugin class.
     * @tc.expected: step3. The member function of the plugin object can be called normally,
     *                      and the execution result is correct.
     */
    labelDetector->Prepare();
    string result = labelDetector->Process();
    delete labelDetector;
    EXPECT_EQ(result, "LabelDetector");

    /**
     * @tc.steps: step4. Create a plugin object by class name from the second plugin class.
     * @tc.expected: step4. The plugin object was created successfully.
     */
    implClassName = "OHOS::PluginExample::CloudLabelDetector";
    AbsImageDetector *cLabelDetector = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    ASSERT_NE(cLabelDetector, nullptr);

    /**
     * @tc.steps: step5. Call the member function of the plugin object from the second plugin class.
     * @tc.expected: step5. The member function of the plugin object can be called normally,
     *                      and the execution result is correct.
     */
    cLabelDetector->Prepare();
    result = cLabelDetector->Process();
    delete cLabelDetector;
    EXPECT_EQ(result, "CloudLabelDetector");
}

/**
 * @tc.name: TestCreateByName001
 * @tc.desc: Verify that the object is not able to be created and
 *           returns the correct error code when the class is not found by class name.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestCreateByName001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register a directory with some valid plugin packages.
     * @tc.expected: step1. The directory was registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin/testplugins" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. Create a plugin object with a non-existing class name parameter.
     * @tc.expected: step2. Creation failed and correctly returned error code indicating the reason.
     */
    uint32_t errorCode;
    // "UnknownDetector" means non-existing detector object.
    AbsImageDetector *unknownDetector = pluginServer.CreateObject<AbsImageDetector>("UnknownDetector", errorCode);
    EXPECT_EQ(unknownDetector, nullptr);

    delete unknownDetector;
    EXPECT_EQ(errorCode, ERR_MATCHING_PLUGIN);
}

/**
 * @tc.name: TestCreateByService001
 * @tc.desc: Verify that the plugin object can be found and created correctly by service
 *           type id.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestCreateByService001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register a directory with some valid plugin packages.
     * @tc.expected: step1. The directory was registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. Create a plugin object by servicer type id of face detector.
     * @tc.expected: step2. The plugin object was created successfully.
     */
    uint32_t errorCode;
    AbsImageDetector *labelDetector =
        pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_FACE, errorCode);
    ASSERT_NE(labelDetector, nullptr);

    /**
     * @tc.steps: step3. Call the member function of the plugin object of face detector.
     * @tc.expected: step3. The member function of the plugin object can be called normally,
     *                      and the execution result is correct.
     */
    labelDetector->Prepare();
    string result = labelDetector->Process();
    delete labelDetector;
    ASSERT_EQ(result, "LabelDetector3");

    /**
     * @tc.steps: step4. Create a plugin object by servicer type id of text detector.
     * @tc.expected: step4. The plugin object was created successfully.
     */
    AbsImageDetector *cLabelDetector =
        pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_TEXT, errorCode);
    ASSERT_NE(cLabelDetector, nullptr);

    /**
     * @tc.steps: step5. Call the member function of the plugin object of text detector.
     * @tc.expected: step5. The member function of the plugin object can be called normally,
     *                      and the execution result is correct.
     */
    cLabelDetector->Prepare();
    result = cLabelDetector->Process();
    delete cLabelDetector;
    ASSERT_EQ(result, "CloudLabelDetector3");
}

/**
 * @tc.name: TestCreateByService002
 * @tc.desc: Verify that the object is not able to be created and return the correct error code
 *           when the matching class is not found by the service type id parameter.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestCreateByService002, TestSize.Level3)
{
    AbsImageDetector *unknownDetector = nullptr;
    HiLog::Debug(LABEL, "[PluginManager_TestCreateByService_002] Start.");
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();

    /**
     * @tc.steps: step1. Register a directory with some valid plugin packages.
     * @tc.expected: step1. The directory was registered successfully.
     */
    vector<string> pluginPaths = { "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. Create a plugin object with a non-existing service type id parameter.
     * @tc.expected: step2. Creation failed and correctly returned error code indicating the reason.
     */
    uint32_t errorCode;
    unknownDetector = pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_FLOWER, errorCode);
    EXPECT_EQ(unknownDetector, nullptr);

    delete unknownDetector;
    EXPECT_EQ(errorCode, ERR_MATCHING_PLUGIN);
}

/**
 * @tc.name: TestCreateByCapabilities001
 * @tc.desc: Verify that the plugin object can be found and created correctly by capabilities.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestCreateByCapabilities001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register multiple plugin directories with multiple valid plugin packages.
     * @tc.expected: step1. The directories were registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin", "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. Create a plugin object by servicer type id and capabilities.
     * @tc.expected: step2. The plugin object was created successfully.
     */
    uint32_t errorCode;
    // "labelNum" means capability name, 10000 means capability value, exist in metadata.
    map<string, AttrData> capabilities = { { "labelNum", AttrData(static_cast<uint32_t>(10000)) } };
    AbsImageDetector *labelDetector =
        pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_LABEL, capabilities, errorCode);
    ASSERT_NE(labelDetector, nullptr);

    /**
     * @tc.steps: step3. Call the member function of the plugin object.
     * @tc.expected: step4. The member function of the plugin object can be called normally,
     *                      and the execution result is correct.
     */
    labelDetector->Prepare();
    string result = labelDetector->Process();
    delete labelDetector;
    ASSERT_EQ(result, "CloudLabelDetector");
}

/**
 * @tc.name: TestCreateByCapabilities002
 * @tc.desc: Verify that the object is not able to be created and return the correct error code
 *           when the matching class is not found by the capabilities.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestCreateByCapabilities002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register multiple plugin directories with multiple valid plugin packages.
     * @tc.expected: step1. The directories were registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin/testplugins2", "/system/etc/multimediaplugin",
                                   "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. Create a plugin object with a non-existing service type id and capabilities parameter.
     * @tc.expected: step2. Creation failed and correctly returned error code indicating the reason.
     */
    uint32_t errorCode;
    // "labelNum" means capability name, 128 means capability value, not exist in metadata.
    map<string, AttrData> capabilities = { { "labelNum", AttrData(static_cast<uint32_t>(128)) } };
    AbsImageDetector *unknownDetector =
        pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_LABEL, capabilities, errorCode);
    EXPECT_EQ(unknownDetector, nullptr);
    delete unknownDetector;
    EXPECT_EQ(errorCode, ERR_MATCHING_PLUGIN);
}

/**
 * @tc.name: TestPluginPriority001
 * @tc.desc: Verify that the plugin class static priority function is correct.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestPluginPriority001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register multiple plugin directories with multiple valid plugin packages.
     * @tc.expected: step1. The directories were registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin", "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. Create a plugin object by servicer type id and there are multiple classes
     *                   that can match the id.
     * @tc.expected: step2. The highest priority matching plugin object was created successfully.
     */
    uint32_t errorCode;
    AbsImageDetector *labelDetector =
        pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_LABEL, errorCode);
    ASSERT_NE(labelDetector, nullptr);

    /**
     * @tc.steps: step3. Call the member function of the plugin object.
     * @tc.expected: step4. The member function of the plugin object can be called normally,
     *                      and the execution result is correct.
     */
    labelDetector->Prepare();
    string result = labelDetector->Process();
    delete labelDetector;
    // here, the higher target is LabelDetector and is not CloudLabelDetector.
    ASSERT_EQ(result, "LabelDetector");
}

/**
 * @tc.name: TestPluginPriority002
 * @tc.desc: Verify that the plugin class dynamic priority is correct and
 *           takes precedence over static priority.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestPluginPriority002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register multiple plugin directories with multiple valid plugin packages.
     * @tc.expected: step1. The directories were registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin", "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. Create a plugin object by servicer type id and priorityScheme, and there are multiple classes
     *                   that can match the id.
     * @tc.expected: step2. The highest priority matching plugin object was created successfully.
     */
    uint32_t errorCode;
    // "labelNum" means attrdata key, exist in metedata.
    PriorityScheme priorityScheme = { PriorityType::PRIORITY_ORDER_BY_ATTR_DESCENDING, "labelNum" };
    AbsImageDetector *labelDetector =
        pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_LABEL, priorityScheme, errorCode);
    ASSERT_NE(labelDetector, nullptr);

    /**
     * @tc.steps: step3. Call the member function of the plugin object.
     * @tc.expected: step4. The member function of the plugin object can be called normally,
     *                      and the execution result is correct.
     */
    labelDetector->Prepare();
    string result = labelDetector->Process();
    delete labelDetector;
    // here, the higher target is CloudLabelDetector and is not LabelDetector.
    ASSERT_EQ(result, "CloudLabelDetector");
}

/**
 * @tc.name: TestGetClassByService001
 * @tc.desc: Verify that the plugin object can be found and get classes info correctly by service
 *           type id.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestGetClassByService001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register a directory with some valid plugin packages.
     * @tc.expected: step1. The directory was registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin", "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. get classes information list by servicer type id of face detector.
     * @tc.expected: step2. The getclass info result successfully.
     */
    vector<ClassInfo> classInfo;
    uint32_t errorCode =
        pluginServer.PluginServerGetClassInfo<AbsImageDetector>(AbsImageDetector::SERVICE_FACE, classInfo);
    EXPECT_NE(classInfo.size(), 0UL);  // existing service type id, get class success, size should not be zero.
    EXPECT_EQ(errorCode, SUCCESS);

    /**
     * @tc.steps: step3. get classes information list by servicer type id of text detector.
     * @tc.expected: step3. The getclass info result successfully.
     */
    errorCode = pluginServer.PluginServerGetClassInfo<AbsImageDetector>(AbsImageDetector::SERVICE_TEXT, classInfo);
    EXPECT_NE(classInfo.size(), 0UL);  // existing service type id, get class success, size should not be zero.
    EXPECT_EQ(errorCode, SUCCESS);
}

/**
 * @tc.name: TestGetClassByService002
 * @tc.desc: Verify that the plugin classes can not be found by non-existing service type id.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestGetClassByService002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register a directory with some valid plugin packages.
     * @tc.expected: step1. The directory was registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin", "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. get classes information with non-existing service type id parameter.
     * @tc.expected: step2. The getclass info result fail.
     */
    vector<ClassInfo> classInfo;
    uint32_t errorCode =
        pluginServer.PluginServerGetClassInfo<AbsImageDetector>(AbsImageDetector::SERVICE_FLOWER, classInfo);
    ASSERT_EQ(classInfo.size(), 0UL);  // non-existing service type id, get class success, size should be zero.
    ASSERT_NE(errorCode, SUCCESS);
}

/**
 * @tc.name: TestGetClassByCapbility001
 * @tc.desc: Verify that the plugin classes can be found and get classes info correctly by service
 *           type id.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestGetClassByCapbility001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register a directory with some valid plugin packages.
     * @tc.expected: step1. The directory was registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin", "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. get classes information list by servicer type id of face detector and capbilities.
     * @tc.expected: step2. The getclass info result successfully.
     */
    vector<ClassInfo> classInfo;
    // "labelNum" means capability name, 256 means capability value, exist in metedata.
    map<string, AttrData> capabilities = { { "labelNum", AttrData(static_cast<uint32_t>(256)) } };
    uint32_t errorCode =
        pluginServer.PluginServerGetClassInfo<AbsImageDetector>(AbsImageDetector::SERVICE_FACE, capabilities, classInfo);
    ASSERT_NE(classInfo.size(), 0UL);  // existing service type id, get class success, size should not be zero.
    ASSERT_EQ(errorCode, SUCCESS);
}

/**
 * @tc.name: TestGetClassByCapbility002
 * @tc.desc: Verify that the plugin classes can not be found by the correct service type id
 *           but the non-existing capbility.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestGetClassByCapbility002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register a directory with some valid plugin packages.
     * @tc.expected: step1. The directory was registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin", "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. get classes information list by servicer type id of face detector and capbilities.
     * @tc.expected: step2. The getclass info result successfully.
     */
    vector<ClassInfo> classInfo;
    // "labelNum1" means capability name, 1000 means capability value, not exist in metedata.
    map<string, AttrData> capabilities = { { "labelNum1", AttrData(static_cast<uint32_t>(1000)) } };
    uint32_t errorCode =
        pluginServer.PluginServerGetClassInfo<AbsImageDetector>(AbsImageDetector::SERVICE_FACE, capabilities, classInfo);
    ASSERT_EQ(classInfo.size(), 0UL);  // non-existing service type id, get class success, size should be zero.
    ASSERT_NE(errorCode, SUCCESS);
}

/**
 * @tc.name: TestInstanceLimit001
 * @tc.desc: Verify cross-create multiple plugin objects within the limit of the number of instances.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestInstanceLimit001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register a directory with some valid plugin packages.
     * @tc.expected: step1. The directory was registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin/testplugins" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. Cross-create multiple plugin objects within the limit of the number of instances.
     * @tc.expected: step2. The plugin objects were created successfully.
     */
    ASSERT_EQ(DoTestInstanceLimit001(pluginServer), SUCCESS);
}

/**
 * @tc.name: TestInstanceLimit002
 * @tc.desc: Verify create multiple plugin objects belonging to the same class, up to the
 *           maximum number of instances.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestInstanceLimit002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register a directory with some valid plugin packages.
     * @tc.expected: step1. The directory was registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    /**
     * @tc.steps: step2. Create multiple plugin objects belonging to the same class,
     *                   up to the maximum number of instances.
     * @tc.expected: step2. The plugin objects were created successfully.
     */
    uint32_t errorCode;
    AbsImageDetector *labelDetectorIns1 =
        pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_FACE, errorCode);
    EXPECT_NE(labelDetectorIns1, nullptr);
    delete labelDetectorIns1;

    AbsImageDetector *labelDetectorIns2 =
        pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_FACE, errorCode);
    EXPECT_NE(labelDetectorIns2, nullptr);
    delete labelDetectorIns2;

    AbsImageDetector *labelDetectorIns3 =
        pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_FACE, errorCode);
    EXPECT_NE(labelDetectorIns3, nullptr);
    delete labelDetectorIns3;
}

/**
 * @tc.name: TestInstanceLimit003
 * @tc.desc: Verify that the number of instances limit mechanism is correct.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, TestInstanceLimit003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Register a directory with some valid plugin packages.
     * @tc.expected: step1. The directory was registered successfully.
     */
    PluginServer &pluginServer = DelayedRefSingleton<PluginServer>::GetInstance();
    vector<string> pluginPaths = { "/system/etc/multimediaplugin/testplugins2" };
    uint32_t ret = pluginServer.Register(std::move(pluginPaths));
    ASSERT_EQ(ret, SUCCESS);

    ASSERT_EQ(DoTestInstanceLimit003(pluginServer), SUCCESS);
}

// ------------------------------- private method -------------------------------
/*
 * Feature: MultiMedia
 * Function: Plugins
 * SubFunction: Plugins Manager
 * FunctionPoints: Registering and managing multiple Plugins
 * EnvConditions: NA
 * CaseDescription: Verify that the plugin management module supports registration of
 *                  multiple directories not contain each other.
 */
uint32_t PluginManagerTest::DoTestRegister003(PluginServer &pluginServer)
{
    uint32_t testRet = ERR_GENERAL;
    uint32_t errorCode;
    AbsImageDetector *labelDetector3 = nullptr;
    AbsImageDetector *cLabelDetector = nullptr;
    string result;
    string implClassName = "OHOS::PluginExample::CloudLabelDetector2";
    AbsImageDetector *cLabelDetector2 = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    if (cLabelDetector2 == nullptr) {
        HiLog::Error(LABEL, "[DoTestRegister003] cLabelDetector2 null. ERRNO: %{public}u.", errorCode);
        goto TEST_END;
    }

    implClassName = "OHOS::PluginExample::LabelDetector3";
    labelDetector3 = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    if (labelDetector3 == nullptr) {
        HiLog::Error(LABEL, "[DoTestRegister003] labelDetector3 null. ERRNO: %{public}u.", errorCode);
        goto TEST_END;
    }

    implClassName = "OHOS::PluginExample::CloudLabelDetector";
    cLabelDetector = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    if (cLabelDetector == nullptr) {
        HiLog::Error(LABEL, "[DoTestRegister003] cLabelDetector null. ERRNO: %{public}u.", errorCode);
        goto TEST_END;
    }

    result = cLabelDetector2->Process();
    if (result != "CloudLabelDetector2") {
        HiLog::Error(LABEL, "[DoTestRegister003] result1 check fail, result: %{public}s.", result.c_str());
        goto TEST_END;
    }

    result = labelDetector3->Process();
    if (result != "LabelDetector3") {
        HiLog::Error(LABEL, "[DoTestRegister003] result2 check fail, result: %{public}s.", result.c_str());
        goto TEST_END;
    }

    result = cLabelDetector->Process();
    if (result != "CloudLabelDetector") {
        HiLog::Error(LABEL, "[DoTestRegister003] result3 check fail, result: %{public}s.", result.c_str());
        goto TEST_END;
    }
    testRet = SUCCESS;

TEST_END:
    delete cLabelDetector;
    delete cLabelDetector2;
    delete labelDetector3;

    return testRet;
}

/*
 * Feature: MultiMedia
 * Function: Plugins
 * SubFunction: Plugins Manager
 * FunctionPoints: Registering and managing multiple Plugins
 * EnvConditions: NA
 * CaseDescription: Verify that the plugin management module supports the registration of
 *                  multiple directories with duplicate relationships.
 */
uint32_t PluginManagerTest::DoTestRegister004(PluginServer &pluginServer)
{
    uint32_t testRet = ERR_GENERAL;
    uint32_t errorCode;
    AbsImageDetector *cLabelDetector2 = nullptr;
    AbsImageDetector *labelDetector3 = nullptr;
    string result;
    string implClassName = "OHOS::PluginExample::CloudLabelDetector";
    AbsImageDetector *cLabelDetector = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    if (cLabelDetector == nullptr) {
        HiLog::Error(LABEL, "[DoTestRegister004] cLabelDetector null. ERRNO: %{public}u.", errorCode);
        goto TEST_END;
    }

    implClassName = "OHOS::PluginExample::CloudLabelDetector2";
    cLabelDetector2 = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    if (cLabelDetector2 == nullptr) {
        HiLog::Error(LABEL, "[DoTestRegister004] cLabelDetector2 null. ERRNO: %{public}u.", errorCode);
        goto TEST_END;
    }

    implClassName = "OHOS::PluginExample::LabelDetector3";
    labelDetector3 = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    if (labelDetector3 == nullptr) {
        HiLog::Error(LABEL, "[DoTestRegister004] labelDetector3 null. ERRNO: %{public}u.", errorCode);
        goto TEST_END;
    }

    result = cLabelDetector->Process();
    if (result != "CloudLabelDetector") {
        HiLog::Error(LABEL, "[DoTestRegister004] result1 check fail, result: %{public}s.", result.c_str());
        goto TEST_END;
    }

    result = cLabelDetector2->Process();
    if (result != "CloudLabelDetector2") {
        HiLog::Error(LABEL, "[DoTestRegister004] result2 check fail, result: %{public}s.", result.c_str());
        goto TEST_END;
    }

    result = labelDetector3->Process();
    if (result != "LabelDetector3") {
        HiLog::Error(LABEL, "[DoTestRegister004] result3 check fail, result: %{public}s.", result.c_str());
        return ERR_GENERAL;
    }
    testRet = SUCCESS;

TEST_END:
    delete cLabelDetector;
    delete cLabelDetector2;
    delete labelDetector3;

    return testRet;
}

/*
 * Feature: MultiMedia
 * Function: Plugins
 * SubFunction:  Reference count
 * FunctionPoints: Registering and managing multiple Plugins
 * EnvConditions: NA
 * CaseDescription: Verify cross-create multiple plugin objects within the limit of the number of instances.
 */
uint32_t PluginManagerTest::DoTestInstanceLimit001(PluginServer &pluginServer)
{
    uint32_t testRet = ERR_GENERAL;
    uint32_t errorCode;
    AbsImageDetector *labelDetectorIns1 = nullptr;
    AbsImageDetector *cLabelDetectorIns1 = nullptr;
    AbsImageDetector *labelDetectorIns2 = nullptr;
    AbsImageDetector *cLabelDetectorIns2 = nullptr;
    string implClassName = "OHOS::PluginExample::LabelDetector";
    labelDetectorIns1 = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    if (labelDetectorIns1 == nullptr) {
        HiLog::Error(LABEL, "[PluginManager_TestInstanceLimit_001] labelDetectorIns1 null. ERRNO: %{public}u.",
                     errorCode);
        goto TEST_END;
    }

    implClassName = "OHOS::PluginExample::CloudLabelDetector";
    cLabelDetectorIns1 = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    if (cLabelDetectorIns1 == nullptr) {
        HiLog::Error(LABEL, "[PluginManager_TestInstanceLimit_001] cLabelDetectorIns1 null. ERRNO: %{public}u.",
                     errorCode);
        goto TEST_END;
    }

    implClassName = "OHOS::PluginExample::LabelDetector";
    labelDetectorIns2 = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    if (labelDetectorIns2 == nullptr) {
        HiLog::Error(LABEL, "[PluginManager_TestInstanceLimit_001] labelDetectorIns2 null. ERRNO: %{public}u.",
                     errorCode);
        goto TEST_END;
    }

    implClassName = "OHOS::PluginExample::CloudLabelDetector";
    cLabelDetectorIns2 = pluginServer.CreateObject<AbsImageDetector>(implClassName, errorCode);
    if (cLabelDetectorIns2 == nullptr) {
        HiLog::Error(LABEL, "[PluginManager_TestInstanceLimit_001] cLabelDetectorIns2 null. ERRNO: %{public}u.",
                     errorCode);
        goto TEST_END;
    }
    testRet = SUCCESS;

TEST_END:
    delete labelDetectorIns1;
    delete cLabelDetectorIns1;
    delete labelDetectorIns2;
    delete cLabelDetectorIns2;

    return testRet;
}

/*
 * Feature: MultiMedia
 * Function: Plugins
 * SubFunction: Plugins Manager
 * FunctionPoints: Instance number
 * EnvConditions: NA
 * CaseDescription: Verify that the number of instances limit mechanism is correct.
 */
uint32_t PluginManagerTest::DoTestInstanceLimit003(PluginServer &pluginServer)
{
    uint32_t testRet = ERR_GENERAL;
    uint32_t errorCode;
    AbsImageDetector *labelDetectorIns2 = nullptr;
    AbsImageDetector *labelDetectorIns3 = nullptr;
    AbsImageDetector *labelDetectorIns4 = nullptr;
    AbsImageDetector *labelDetectorIns5 = nullptr;

    /**
     * @tc.steps: step2. Create multiple plugin objects belonging to the same class,
     *                   the number of which exceeds the maximum number of instances.
     * @tc.expected: step2. The part that did not exceed the number of instances was created successfully,
     *                      the excess part was created failed and an error code indicating the reason for
     *                      the name was returned.
     */
    AbsImageDetector *labelDetectorIns1 =
        pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_FACE, errorCode);
    if (labelDetectorIns1 == nullptr) {
        HiLog::Error(LABEL, "[DoTestInstanceLimit003] labelDetectorIns1 null. ERRNO: %{public}u.", errorCode);
        goto TEST_END;
    }

    labelDetectorIns2 = pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_FACE, errorCode);
    if (labelDetectorIns1 == nullptr) {
        HiLog::Error(LABEL, "[DoTestInstanceLimit003] labelDetectorIns2 null. ERRNO: %{public}u.", errorCode);
        goto TEST_END;
    }

    labelDetectorIns3 = pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_FACE, errorCode);
    if (labelDetectorIns1 == nullptr) {
        HiLog::Error(LABEL, "[DoTestInstanceLimit003] labelDetectorIns3 null. ERRNO: %{public}u.", errorCode);
        goto TEST_END;
    }

    labelDetectorIns4 = pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_FACE, errorCode);
    if (labelDetectorIns4 != nullptr) {
        HiLog::Error(LABEL, "[DoTestInstanceLimit003] labelDetectorIns4 not null. ERRNO: %{public}u.", errorCode);
        goto TEST_END;
    }

    if (errorCode != ERR_INSTANCE_LIMIT) {
        HiLog::Error(LABEL, "[DoTestInstanceLimit003] unexpected errorCode: %{public}u.", errorCode);
        goto TEST_END;
    }

    /**
     * @tc.steps: step3. Release a plugin object, making the number of instances below the limit,
     *                   then request to create a new plugin object.
     * @tc.expected: step3. The new plugin object was created successfully.
     */
    delete labelDetectorIns2;
    labelDetectorIns2 = nullptr;

    labelDetectorIns5 = pluginServer.CreateObject<AbsImageDetector>(AbsImageDetector::SERVICE_FACE, errorCode);
    if (labelDetectorIns1 == nullptr) {
        HiLog::Error(LABEL, "[DoTestInstanceLimit003] labelDetectorIns5 null. ERRNO: %{public}u.", errorCode);
        goto TEST_END;
    }
    testRet = SUCCESS;

TEST_END:
    delete labelDetectorIns1;
    delete labelDetectorIns2;
    delete labelDetectorIns3;
    delete labelDetectorIns4;
    delete labelDetectorIns5;

    return testRet;
}
