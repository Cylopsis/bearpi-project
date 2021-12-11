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

#include "data_translator.h"

#include "field_const.h"

namespace OHOS {
namespace Security {
namespace Permission {
int DataTranslator::TranslationIntoPermissionRecord(
    const GenericValues &inGenericValues, PermissionRecord &outPermissionRecord)
{
    outPermissionRecord.timestamp = inGenericValues.GetInt(FIELD_TIMESTAMP);
    outPermissionRecord.visitorId = inGenericValues.GetInt(FIELD_VISITOR_ID);
    outPermissionRecord.opCode = inGenericValues.GetInt(FIELD_OP_CODE);
    outPermissionRecord.isForeground = inGenericValues.GetInt(FIELD_IS_FOREGROUND);
    outPermissionRecord.accessCount = inGenericValues.GetInt(FIELD_ACCESS_COUNT);
    outPermissionRecord.rejectCount = inGenericValues.GetInt(FIELD_REJECT_COUNT);
    return Constant::SUCCESS;
}

int DataTranslator::TranslationIntoGenericValues(
    const PermissionRecord &inPermissionRecord, GenericValues &outGenericValues)
{
    outGenericValues.Put(FIELD_TIMESTAMP, inPermissionRecord.timestamp);
    outGenericValues.Put(FIELD_VISITOR_ID, inPermissionRecord.visitorId);
    outGenericValues.Put(FIELD_OP_CODE, inPermissionRecord.opCode);
    outGenericValues.Put(FIELD_IS_FOREGROUND, inPermissionRecord.isForeground);
    outGenericValues.Put(FIELD_ACCESS_COUNT, inPermissionRecord.accessCount);
    outGenericValues.Put(FIELD_REJECT_COUNT, inPermissionRecord.rejectCount);
    return Constant::SUCCESS;
}

int DataTranslator::TranslationIntoPermissionVisitor(
    const GenericValues &inGenericValues, PermissionVisitor &outPermissionVisitor)
{
    outPermissionVisitor.id = inGenericValues.GetInt(FIELD_ID);
    outPermissionVisitor.deviceId = inGenericValues.GetString(FIELD_DEVICE_ID);
    outPermissionVisitor.deviceName = inGenericValues.GetString(FIELD_DEVICE_NAME);
    outPermissionVisitor.bundleUserId = inGenericValues.GetInt(FIELD_BUNDLE_USER_ID);
    outPermissionVisitor.bundleName = inGenericValues.GetString(FIELD_BUNDLE_NAME);
    outPermissionVisitor.bundleLabel = inGenericValues.GetString(FIELD_BUNDLE_LABEL);
    return Constant::SUCCESS;
}

int DataTranslator::TranslationIntoGenericValues(
    const PermissionVisitor &inPermissionVisitor, GenericValues &outGenericValues)
{
    outGenericValues.Put(FIELD_DEVICE_ID, inPermissionVisitor.deviceId);
    outGenericValues.Put(FIELD_DEVICE_NAME, inPermissionVisitor.deviceName);
    outGenericValues.Put(FIELD_BUNDLE_USER_ID, inPermissionVisitor.bundleUserId);
    outGenericValues.Put(FIELD_BUNDLE_NAME, inPermissionVisitor.bundleName);
    outGenericValues.Put(FIELD_BUNDLE_LABEL, inPermissionVisitor.bundleLabel);
    return Constant::SUCCESS;
}

int DataTranslator::TranslationIntoGenericValues(const QueryPermissionUsedRequest &inQueryRequest,
    GenericValues &outVisitorValues, GenericValues &outRecordAndValues, GenericValues &outRecordOrValues)
{
    if (!inQueryRequest.deviceLabel.empty()) {
        outVisitorValues.Put(FIELD_DEVICE_NAME, inQueryRequest.deviceLabel);
    }
    if (!inQueryRequest.bundleName.empty()) {
        outVisitorValues.Put(FIELD_BUNDLE_NAME, inQueryRequest.bundleName);
    }
    std::string permissionNames;
    for (unsigned int i = 0; i < inQueryRequest.permissionNames.size(); i++) {
        int32_t code = 0;
        std::string perName = inQueryRequest.permissionNames[i];
        if (Constant::PermissionNameToOrFromOpCode(perName, code)) {
            outRecordOrValues.Put(FIELD_OP_CODE + ToString(i), code);
        }
    }
    int64_t begin = inQueryRequest.beginTimeMillis;
    int64_t end = inQueryRequest.endTimeMillis;
    if (begin < 0 || end < 0 || (begin > end && end > 0)) {
        return Constant::FAILURE;
    }
    if (begin != 0) {
        outRecordAndValues.Put(FIELD_TIMESTAMP_BEGIN, begin);
    }
    if (end != 0) {
        outRecordAndValues.Put(FIELD_TIMESTAMP_END, end);
    }
    return Constant::SUCCESS;
}

int DataTranslator::TranslationIntoBundlePermissionUsedRecord(
    const GenericValues &inGenericValues, BundlePermissionUsedRecord &outBundleRecord)
{
    outBundleRecord.deviceId = inGenericValues.GetString(FIELD_DEVICE_ID);
    outBundleRecord.deviceLabel = inGenericValues.GetString(FIELD_DEVICE_NAME);
    outBundleRecord.bundleName = inGenericValues.GetString(FIELD_BUNDLE_NAME);
    outBundleRecord.bundleLabel = inGenericValues.GetString(FIELD_BUNDLE_LABEL);
    return Constant::SUCCESS;
}

int DataTranslator::TranslationIntoPermissionUsedRecord(
    const GenericValues &inGenericValues, PermissionUsedRecord &outPermRecord)
{
    std::string perName;
    int32_t code = inGenericValues.GetInt(FIELD_OP_CODE);
    if (!Constant::PermissionNameToOrFromOpCode(perName, code)) {
        return Constant::FAILURE;
    }
    outPermRecord.permissionName = perName;
    if (inGenericValues.GetInt(FIELD_IS_FOREGROUND) == 1) {
        outPermRecord.accessCountFg = inGenericValues.GetInt(FIELD_ACCESS_COUNT);
        outPermRecord.rejectCountFg = inGenericValues.GetInt(FIELD_REJECT_COUNT);
    } else {
        outPermRecord.accessCountBg = inGenericValues.GetInt(FIELD_ACCESS_COUNT);
        outPermRecord.rejectCountBg = inGenericValues.GetInt(FIELD_REJECT_COUNT);
    }
    if (inGenericValues.GetInt(FIELD_ACCESS_COUNT) > inGenericValues.GetInt(FIELD_REJECT_COUNT)) {
        if (inGenericValues.GetInt(FIELD_REJECT_COUNT) != 0) {
            outPermRecord.lastRejectTime = inGenericValues.GetInt64(FIELD_TIMESTAMP);
        }
        outPermRecord.lastAccessTime = inGenericValues.GetInt64(FIELD_TIMESTAMP);
    } else {
        if (inGenericValues.GetInt(FIELD_ACCESS_COUNT) != 0) {
            outPermRecord.lastAccessTime = inGenericValues.GetInt64(FIELD_TIMESTAMP);
        }
        outPermRecord.lastRejectTime = inGenericValues.GetInt64(FIELD_TIMESTAMP);
    }
    if (inGenericValues.GetInt(FIELD_FLAG) == 1) {
        if (outPermRecord.accessCountFg > 0 || outPermRecord.rejectCountFg > 0) {
            (outPermRecord.lastAccessTime > 0)
                ? outPermRecord.accessRecordFg.emplace_back(outPermRecord.lastAccessTime)
                : outPermRecord.rejectRecordFg.emplace_back(outPermRecord.lastRejectTime);
        } else {
            (outPermRecord.lastAccessTime > 0)
                ? outPermRecord.accessRecordBg.emplace_back(outPermRecord.lastAccessTime)
                : outPermRecord.rejectRecordBg.emplace_back(outPermRecord.lastRejectTime);
        }
    }

    return Constant::SUCCESS;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS