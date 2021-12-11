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

#ifndef IMPL_CLASS_KEY_H
#define IMPL_CLASS_KEY_H

#include "abs_impl_class_key.h"

namespace OHOS {
namespace MultimediaPlugin {
class ImplClass;

class ImplClassKey final : public AbsImplClassKey {
public:
    // must guarantee that the key object continue to be valid until all corresponding instances are destroyed.
    explicit ImplClassKey(ImplClass &key) : implClass_(key) {};
    ~ImplClassKey() = default;
    void OnObjectDestroy() override;

private:
    ImplClass &implClass_;
};
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // IMPL_CLASS_KEY_H