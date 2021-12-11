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

#include "matrix.h"

namespace OHOS {
namespace Media {
Matrix &Matrix::Reset()
{
    *this = Matrix();
    return *this;
}

Matrix &Matrix::SetTranslate(const float tx, const float ty)
{
    *this = Matrix(1, 0, tx, 0, 1, ty, 0, 1, 1, (tx == 0 || ty == 0) ? IDENTITY : TRANSLATE);
    return *this;
}

Matrix &Matrix::SetScale(const float sx, const float sy)
{
    *this = Matrix(sx, 0, 0, 0, sy, 0, 0, 0, 1, (sx == 1 && sy == 1) ? IDENTITY : SCALE);
    return *this;
}

Matrix &Matrix::SetRotate(const float degrees, const float px, const float py)
{
    float radians = DegreesToRadians(degrees);
    return SetSinCos(ValueNearToZero(radians, true), ValueNearToZero(radians, false), px, py);
}

Matrix &Matrix::SetSinCos(const float sinValue, const float cosValue, const float px, const float py)
{
    const float reverseCosValue = 1.0f - cosValue;

    fMat_[IMAGE_SCALEX] = cosValue;
    fMat_[IMAGE_SKEWX] = -sinValue;
    fMat_[IMAGE_TRANSX] = sinValue * py + reverseCosValue * px;

    fMat_[IMAGE_SKEWY] = sinValue;
    fMat_[IMAGE_SCALEY] = cosValue;
    fMat_[IMAGE_TRANSY] = -sinValue * px + reverseCosValue * py;

    fMat_[IMAGE_PERSP0] = fMat_[IMAGE_PERSP1] = 0;
    fMat_[IMAGE_PERSP2] = 1;

    operType_ = ROTATEORSKEW;

    return *this;
}

Matrix &Matrix::SetConcat(const Matrix &m)
{
    OperType aOperType = this->GetOperType();
    OperType bOperType = m.GetOperType();

    if ((static_cast<uint8_t>(aOperType) & OPERTYPE_MASK) == 0) {
        *this = m;
    } else if (((static_cast<uint8_t>(aOperType) | static_cast<uint8_t>(bOperType)) & ROTATEORSKEW) == 0) {
        SetTranslateAndScale(fMat_[IMAGE_TRANSX] * m.fMat_[IMAGE_TRANSX] + fMat_[IMAGE_TRANSX],
                             fMat_[IMAGE_TRANSY] * m.fMat_[IMAGE_TRANSY] + fMat_[IMAGE_TRANSY],
                             fMat_[IMAGE_SCALEX] * m.fMat_[IMAGE_SCALEX], fMat_[IMAGE_SCALEY] * m.fMat_[IMAGE_SCALEY]);
    } else {
        Matrix src = *this;
        fMat_[IMAGE_SCALEX] = static_cast<float>(
            MulAddMul(src.fMat_[IMAGE_SCALEX], m.fMat_[IMAGE_SCALEX], src.fMat_[IMAGE_SKEWX], m.fMat_[IMAGE_SKEWY]));

        fMat_[IMAGE_SKEWX] = static_cast<float>(
            MulAddMul(src.fMat_[IMAGE_SCALEX], m.fMat_[IMAGE_SKEWX], src.fMat_[IMAGE_SKEWX], m.fMat_[IMAGE_SCALEY]));

        fMat_[IMAGE_TRANSX] = static_cast<float>(
            MulAddMul(src.fMat_[IMAGE_SCALEX], m.fMat_[IMAGE_TRANSX], src.fMat_[IMAGE_SKEWX], m.fMat_[IMAGE_TRANSY]) +
            src.fMat_[IMAGE_TRANSX]);

        fMat_[IMAGE_SKEWY] = static_cast<float>(
            MulAddMul(src.fMat_[IMAGE_SKEWY], m.fMat_[IMAGE_SCALEX], src.fMat_[IMAGE_SCALEY], m.fMat_[IMAGE_SKEWY]));

        fMat_[IMAGE_SCALEY] = static_cast<float>(
            MulAddMul(src.fMat_[IMAGE_SKEWY], m.fMat_[IMAGE_SKEWX], src.fMat_[IMAGE_SCALEY], m.fMat_[IMAGE_SCALEY]));

        fMat_[IMAGE_TRANSY] = static_cast<float>(
            MulAddMul(src.fMat_[IMAGE_SKEWY], m.fMat_[IMAGE_TRANSX], src.fMat_[IMAGE_SCALEY], m.fMat_[IMAGE_TRANSY]) +
            src.fMat_[IMAGE_TRANSY]);

        fMat_[IMAGE_PERSP0] = fMat_[IMAGE_PERSP1] = 0;
        fMat_[IMAGE_PERSP2] = 1;

        operType_ = ROTATEORSKEW;
    }
    return *this;
}

Matrix::OperType Matrix::GetOperType() const
{
    return (OperType)(operType_ & OPERTYPE_MASK);
}

void Matrix::SetTranslateAndScale(const float tx, const float ty, const float sx, const float sy)
{
    fMat_[IMAGE_SCALEX] = sx;
    fMat_[IMAGE_SKEWX] = 0;
    fMat_[IMAGE_TRANSX] = tx;

    fMat_[IMAGE_SKEWY] = 0;
    fMat_[IMAGE_SCALEY] = sy;
    fMat_[IMAGE_TRANSY] = ty;

    fMat_[IMAGE_PERSP0] = fMat_[IMAGE_PERSP1] = 0;
    fMat_[IMAGE_PERSP2] = 1;

    if (sx != 1 || sy != 1) {
        operType_ |= SCALE;
    }
    if (tx != 0 || ty != 0) {
        operType_ |= TRANSLATE;
    }
}

bool Matrix::Invert(Matrix &invMatrix)
{
    invMatrix.operType_ = operType_;
    if (IsIdentity()) {
        invMatrix.Reset();
        return true;
    }

    if ((operType_ & (~(TRANSLATE | SCALE))) == 0) {
        if (operType_ & SCALE) {
            float invScaleX = fMat_[IMAGE_SCALEX];
            float invScaleY = fMat_[IMAGE_SCALEY];
            if (std::fabs(invScaleX) < MATRIX_EPSILON || std::fabs(invScaleY) < MATRIX_EPSILON) {
                return false;
            }

            // 1.0f used when calculating the inverse matrix
            invScaleX = FDivide(1.0f, invScaleX);
            invScaleY = FDivide(1.0f, invScaleY);

            invMatrix.fMat_[IMAGE_SCALEX] = invScaleX;
            invMatrix.fMat_[IMAGE_SCALEY] = invScaleY;

            invMatrix.fMat_[IMAGE_TRANSX] = -fMat_[IMAGE_TRANSX] * invScaleX;
            invMatrix.fMat_[IMAGE_TRANSY] = -fMat_[IMAGE_TRANSY] * invScaleY;

            invMatrix.fMat_[IMAGE_SKEWX] = invMatrix.fMat_[IMAGE_SKEWY] = invMatrix.fMat_[IMAGE_PERSP0] =
                invMatrix.fMat_[IMAGE_PERSP1] = 0;
            invMatrix.fMat_[IMAGE_PERSP2] = 1;
        } else {
            invMatrix.SetTranslate(-fMat_[IMAGE_TRANSX], -fMat_[IMAGE_TRANSY]);
        }
        return true;
    }

    return InvertForRotate(invMatrix);
}

bool Matrix::InvertForRotate(Matrix &invMatrix)
{
    double invDet = MulSubMul(fMat_[IMAGE_SCALEX], fMat_[IMAGE_SCALEY], fMat_[IMAGE_SKEWX], fMat_[IMAGE_SKEWY]);
    if (fabsf(static_cast<float>(invDet)) < (FLOAT_NEAR_ZERO * FLOAT_NEAR_ZERO * FLOAT_NEAR_ZERO)) {
        return false;
    } else {
        invDet = 1.0 / invDet;  // 1.0 used when calculating the inverse matrix
    }

    invMatrix.fMat_[IMAGE_SCALEX] = static_cast<float>(fMat_[IMAGE_SCALEY] * invDet);
    invMatrix.fMat_[IMAGE_SKEWX] = static_cast<float>(-fMat_[IMAGE_SKEWX] * invDet);
    invMatrix.fMat_[IMAGE_TRANSX] = static_cast<float>(
        MulSubMul(fMat_[IMAGE_SKEWX], fMat_[IMAGE_TRANSY], fMat_[IMAGE_SCALEY], fMat_[IMAGE_TRANSX]) * invDet);

    invMatrix.fMat_[IMAGE_SKEWY] = static_cast<float>(-fMat_[IMAGE_SKEWY] * invDet);
    invMatrix.fMat_[IMAGE_SCALEY] = static_cast<float>(fMat_[IMAGE_SCALEX] * invDet);
    invMatrix.fMat_[IMAGE_TRANSY] = static_cast<float>(
        MulSubMul(fMat_[IMAGE_SKEWY], fMat_[IMAGE_TRANSX], fMat_[IMAGE_SCALEX], fMat_[IMAGE_TRANSY]) * invDet);

    invMatrix.fMat_[IMAGE_PERSP0] = invMatrix.fMat_[IMAGE_PERSP1] = 0;
    invMatrix.fMat_[IMAGE_PERSP2] = 1;

    return true;
}

void Matrix::IdentityXY(const Matrix &m, const float sx, const float sy, Point &pt)
{
    if (m.GetOperType() == 0) {
        pt.x = sx;
        pt.y = sy;
    }
}

void Matrix::ScaleXY(const Matrix &m, const float sx, const float sy, Point &pt)
{
    if ((static_cast<uint8_t>(m.GetOperType()) & SCALE) == SCALE) {
        pt.x = sx * m.fMat_[IMAGE_SCALEX] + m.fMat_[IMAGE_TRANSX];
        pt.y = sy * m.fMat_[IMAGE_SCALEY] + m.fMat_[IMAGE_TRANSY];
    }
}

void Matrix::TransXY(const Matrix &m, const float tx, const float ty, Point &pt)
{
    if (m.GetOperType() == TRANSLATE) {
        pt.x = tx + m.fMat_[IMAGE_TRANSX];
        pt.y = ty + m.fMat_[IMAGE_TRANSY];
    }
}

void Matrix::RotXY(const Matrix &m, const float rx, const float ry, Point &pt)
{
    if ((static_cast<uint8_t>(m.GetOperType()) & ROTATEORSKEW) == ROTATEORSKEW) {
        pt.x = rx * m.fMat_[IMAGE_SCALEX] + ry * m.fMat_[IMAGE_SKEWX] + m.fMat_[IMAGE_TRANSX];
        pt.y = rx * m.fMat_[IMAGE_SKEWY] + ry * m.fMat_[IMAGE_SCALEY] + m.fMat_[IMAGE_TRANSY];
    }
}

const Matrix::CalcXYProc Matrix::gCalcXYProcs[] = { Matrix::IdentityXY, Matrix::TransXY, Matrix::ScaleXY,
                                                    Matrix::ScaleXY,    Matrix::RotXY,   Matrix::RotXY,
                                                    Matrix::RotXY,      Matrix::RotXY };

// Matrix print function, including 9 elements
void Matrix::Print()
{
    IMAGE_LOGD("[Matrix][%{public}8.4f %{public}8.4f %{public}8.4f]\
                [%{public}8.4f %{public}8.4f %{public}8.4f]\
                [%{public}8.4f %{public}8.4f %{public}8.4f].",
               fMat_[0], fMat_[1], fMat_[2], fMat_[3], fMat_[4], fMat_[5], fMat_[6], fMat_[7], fMat_[8]);
}
} // namespace Media
} // namespace OHOS