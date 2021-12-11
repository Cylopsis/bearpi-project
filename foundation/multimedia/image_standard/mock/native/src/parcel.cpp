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

#include "parcel.h"

namespace OHOS {
Parcelable::Parcelable() : Parcelable(false)
{}

Parcelable::Parcelable(bool asRemote)
{}

Parcel::Parcel()
{}

Parcel::~Parcel() = default;

size_t Parcel::GetDataCapacity() const
{
    return 0;
}

bool Parcel::SetDataCapacity(size_t newCapacity)
{
    (void) newCapacity;
    return false;
}

bool Parcel::WriteBuffer(const void *data, size_t size)
{
    (void) data;
    (void) size;
    return false;
}

bool Parcel::WriteInt32(int32_t value)
{
    (void) value;
    return false;
}

const uint8_t *Parcel::ReadBuffer(size_t length)
{
    (void) length;
    return nullptr;
}

int32_t Parcel::ReadInt32()
{
    return 0;
}
} // namespace OHOS
