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


#ifndef ZIPC_PARCEL_H
#define ZIPC_PARCEL_H

#include <string>
#include <vector>
#include "nocopyable.h"
#include "refbase.h"

namespace OHOS {
class Parcel;

class Parcelable : public virtual RefBase {
public:
    virtual ~Parcelable() = default;

    Parcelable();
    explicit Parcelable(bool asRemote);
};

class Parcel {
public:
    Parcel();

    ~Parcel();

    size_t GetDataCapacity() const;

    bool SetDataCapacity(size_t newCapacity);

    bool WriteInt32(int32_t value);

    bool WriteBuffer(const void *data, size_t size);

    int32_t ReadInt32();

    bool ReadInt32(int32_t &value);

    const uint8_t *ReadBuffer(size_t length);
};
} // namespace OHOS
#endif // ZIPC_PARCEL_H
