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

#ifndef MATRIX_H
#define MATRIX_H

#include <cmath>
#include "image_log.h"
#include "image_type.h"

namespace OHOS {
namespace Media {
struct Point {
    float x = 0.0f;
    float y = 0.0f;
};

static constexpr int32_t IMAGE_SCALEX = 0;
static constexpr int32_t IMAGE_SKEWX = 1;
static constexpr int32_t IMAGE_TRANSX = 2;
static constexpr int32_t IMAGE_SKEWY = 3;
static constexpr int32_t IMAGE_SCALEY = 4;
static constexpr int32_t IMAGE_TRANSY = 5;
static constexpr int32_t IMAGE_PERSP0 = 6;
static constexpr int32_t IMAGE_PERSP1 = 7;
static constexpr int32_t IMAGE_PERSP2 = 8;
static constexpr int32_t MATIRX_ITEM_NUM = 9;

static constexpr uint32_t IDENTITY_TYPE = 0;
static constexpr uint32_t TRANSLATE_TYPE = 0x01;
static constexpr uint32_t SCALE_TYPE = 0x02;
static constexpr uint32_t ROTATEORSKEW_TYPE = 0x04;
static constexpr uint32_t PERSPECTIVE_TYPE = 0x08;

static constexpr float FLOAT_PI = 3.14159265f;
static constexpr float FLOAT_NEAR_ZERO = (1.0f / (1 << 12));
static constexpr float RADIAN_FACTOR = 180.0f;
static constexpr uint32_t CALCPROC_FACTOR = 0x07;
static constexpr uint32_t OPERTYPE_MASK = 0xF;
static constexpr float MATRIX_EPSILON = 1e-6;

// Degrees to Radians
static inline float DegreesToRadians(const float degrees)
{
    return degrees * (FLOAT_PI / RADIAN_FACTOR);
}

// Radians to Degrees
static inline float RadiansToDegrees(const float radians)
{
    return radians * (RADIAN_FACTOR / FLOAT_PI);
}

static inline float ValueNearToZero(const float radians, bool isSin)
{
    float value = (isSin ? sinf(radians) : cosf(radians));
    return (fabsf(value) <= FLOAT_NEAR_ZERO) ? 0.0f : value;
}

static inline double MulAddMul(const float a, const float b, const float c, const float d)
{
    return a * b + c * d;
}

static inline double MulSubMul(const float a, const float b, const float c, const float d)
{
    return a * b - c * d;
}

static inline float FDivide(const float number, const float denom)
{
    if (std::fabs(denom - 0) < MATRIX_EPSILON) {
        return 0.0f;
    }
    return number / denom;
}

class Matrix {
public:
    enum OperType {
        IDENTITY = IDENTITY_TYPE,
        TRANSLATE = TRANSLATE_TYPE,
        SCALE = SCALE_TYPE,
        ROTATEORSKEW = ROTATEORSKEW_TYPE,
        PERSPECTIVE = PERSPECTIVE_TYPE,
    };

    using CalcXYProc = void (*)(const Matrix &m, const float x, const float y, Point &result);

    /** The default is the identity matrix
     * | 1  0  0 |
     * | 0  1  0 |
     * | 0  0  1 |
     */
    constexpr Matrix() : Matrix(1, 0, 0, 0, 1, 0, 0, 0, 1, IDENTITY){};

    constexpr Matrix(float sx, float kx, float tx, float ky, float sy, float ty, float p0, float p1, float p2,
                     uint32_t operType)
        : fMat_ { sx, kx, tx, ky, sy, ty, p0, p1, p2 }, operType_(operType){};

    ~Matrix() = default;

    static const CalcXYProc gCalcXYProcs[];

    bool IsIdentity() const
    {
        return GetOperType() == 0;
    }

    Matrix &SetTranslate(const float tx, const float ty);

    Matrix &SetScale(const float sx, const float sy);

    Matrix &SetRotate(const float degrees, const float px = 0.0, const float py = 0.0);

    Matrix &SetSinCos(const float sinValue, const float cosValue, const float px, const float py);

    Matrix &SetConcat(const Matrix &m);

    Matrix &Reset();

    OperType GetOperType() const;

    void SetTranslateAndScale(const float tx, const float ty, const float sx, const float sy);

    static void IdentityXY(const Matrix &m, const float sx, const float sy, Point &pt);

    static void ScaleXY(const Matrix &m, const float sx, const float sy, Point &pt);

    static void TransXY(const Matrix &m, const float tx, const float sy, Point &pt);

    static void RotXY(const Matrix &m, const float rx, const float ry, Point &pt);

    static CalcXYProc GetXYProc(const OperType operType)
    {
        return gCalcXYProcs[static_cast<uint8_t>(operType) & CALCPROC_FACTOR];
    }

    bool Invert(Matrix &invMatrix);

    bool InvertForRotate(Matrix &invMatrix);

    float GetScaleX() const
    {
        return fMat_[IMAGE_SCALEX];
    }

    float GetScaleY() const
    {
        return fMat_[IMAGE_SCALEY];
    }

    float GetTransX() const
    {
        return fMat_[IMAGE_TRANSX];
    }

    float GetTranY() const
    {
        return fMat_[IMAGE_TRANSY];
    }

    float GetSkewX() const
    {
        return fMat_[IMAGE_SKEWX];
    }

    float GetSkewY() const
    {
        return fMat_[IMAGE_SKEWY];
    }

    void Print();

private:
    /** The fMat_ elements
     *  | scalex  skewx  transx |
     *  | skewy   scaley transy |
     *  | persp0  persp1 persp2 |
     */
    float fMat_[MATIRX_ITEM_NUM];
    uint32_t operType_;
};
} // namespace Media
} // namespace OHOS
#endif // MATRIX_H
