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

#include "pixel_map.h"
#include "pixel_map_parcel.h"

using namespace testing::ext;
using namespace OHOS::Media;

namespace OHOS {
namespace Multimedia {
    class ImagePixelMapParcelTest : public testing::Test {
    public:
        ImagePixelMapParcelTest(){};
        ~ImagePixelMapParcelTest(){};
    };

    std::unique_ptr<PixelMap> ConstructPixmap()
    {
        int32_t pixelMapWidth = 4;
        int32_t pixelMapHeight = 3;
        std::unique_ptr<PixelMap> pixelMap = std::make_unique<PixelMap>();
        ImageInfo info;
        info.size.width = pixelMapWidth;
        info.size.height = pixelMapHeight;
        info.pixelFormat = PixelFormat::RGB_888;
        info.colorSpace = ColorSpace::SRGB;
        pixelMap->SetImageInfo(info);

        int32_t rowDataSize = pixelMapWidth;
        uint32_t bufferSize = rowDataSize * pixelMapHeight;
        void *buffer = malloc(bufferSize);
        char *ch = (char *)buffer;
        for (unsigned int i = 0; i < bufferSize; i++) {
            *(ch++) = (char)i;
        }

        pixelMap->SetPixelsAddr(buffer, nullptr, bufferSize, AllocatorType::HEAP_ALLOC, nullptr);

        return pixelMap;
    }
    /**
    * @tc.name: ImagePixelMapParcel001
    * @tc.desc: test WriteToParcel
    * @tc.type: FUNC
    * @tc.require: AR000FTAMO
    */
    HWTEST_F(ImagePixelMapParcelTest, ImagePixelMapParcel001, TestSize.Level3)
    {
        GTEST_LOG_(INFO) << "ImagePixelMapParcelTest: ImagePixelMapParcel001 start";

        MessageParcel data;

        std::unique_ptr<PixelMap> pixelmap = ConstructPixmap();

        bool ret = PixelMapParcel::WriteToParcel(pixelmap.get(), data);

        EXPECT_EQ(true, ret);

        GTEST_LOG_(INFO) << "ImagePixelMapParcelTest: ImagePixelMapParcel001 end";
    }

    /**
    * @tc.name: ImagePixelMapParcel002
    * @tc.desc: test CreateFromParcel
    * @tc.type: FUNC
    * @tc.require: AR000FTAMO
    */
    HWTEST_F(ImagePixelMapParcelTest, ImagePixelMapParcel002, TestSize.Level3)
    {
        GTEST_LOG_(INFO) << "ImagePixelMapParcelTest: ImagePixelMapParcel002 start";

        MessageParcel data;

        std::unique_ptr<PixelMap> pixelmap1 = ConstructPixmap();

        bool ret = PixelMapParcel::WriteToParcel(pixelmap1.get(), data);

        EXPECT_EQ(true, ret);

        std::unique_ptr<PixelMap> pixelmap2 = PixelMapParcel::CreateFromParcel(data);

        EXPECT_EQ(pixelmap1->GetHeight(), pixelmap2->GetHeight());
        EXPECT_EQ(pixelmap1->GetWidth(), pixelmap2->GetWidth());
        EXPECT_EQ(pixelmap1->GetPixelFormat(), pixelmap2->GetPixelFormat());
        EXPECT_EQ(pixelmap1->GetColorSpace(), pixelmap2->GetColorSpace());

        EXPECT_EQ(true, pixelmap1->IsSameImage(*pixelmap2));

        GTEST_LOG_(INFO) << "ImagePixelMapParcelTest: ImagePixelMapParcel002 end";
    }
}  // namespace Multimedia
}  // namespace OHOS