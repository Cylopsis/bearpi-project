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

#ifndef POINTER_KEY_MAP_H
#define POINTER_KEY_MAP_H

#include <map>

namespace OHOS {
namespace MultimediaPlugin {
template<class K>
struct PointerComparator {
    bool operator()(K *lhs, K *rhs) const
    {
        if (lhs == nullptr || rhs == nullptr) {
            return false;
        }

        return *lhs < *rhs;
    }
};

template<class K, class T> using PointerKeyMap = std::map<K *, T, PointerComparator<K>>;

template<class K, class T> using PointerKeyMultimap = std::multimap<K *, T, PointerComparator<K>>;
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // POINTER_KEY_MAP_H