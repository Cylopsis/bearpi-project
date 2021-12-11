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
#ifndef MULTIMEDIA_TEMPLATES_H
#define MULTIMEDIA_TEMPLATES_H

#include <cstdint>
#include <memory>
#include <utility>

namespace OHOS {
namespace MultiMedia {
/* call a function when this goes out of scope. The template uses two
 * parameters, the object, and a function that is to be called in the destructor
 */
template<typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

template<typename T, T *P>
struct FunctionWrapper {
    template<typename... Args>
    auto operator()(Args &&... args) const -> decltype(P(std::forward<Args>(args)...))
    {
        return P(std::forward<Args>(args)...);
    }
};

template<typename T, void (*P)(T *)>
class TAutoCallProc : public std::unique_ptr<T, FunctionWrapper<remove_pointer_t<decltype(P)>, P>> {
public:
    TAutoCallProc(T *obj) : std::unique_ptr<T, FunctionWrapper<remove_pointer_t<decltype(P)>, P>>(obj)
    {}

    operator T *() const
    {
        return this->get();
    }
    ~TAutoCallProc() = default;
};
}  // namespace MultiMedia
}  // namespace OHOS
#endif  // MULTIMEDIA_TEMPLATES_H
