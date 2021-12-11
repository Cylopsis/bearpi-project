/*
* Copyright (C) 2021 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

import { AsyncCallback } from './basic';

declare namespace image {

  /**
   * Enumerates pixel map formats.
   */
  enum PixelMapFormat {
    /**
     * Indicates an unknown pixel map format.
     */
    UNKNOWN = 0,

    /**
     * Indicates that each pixel is stored on 32 bits. Components A, R, G, and B each occupies 8 bits
     * and are stored from the higher-order to the lower-order bits.
     */
    ARGB_8888 = 1,

    /**
     * Indicates that each pixel is stored on 16 bits. Only the R, G, and B components are encoded
     * from the higher-order to the lower-order bits: red is stored with 5 bits of precision,
     * green is stored with 6 bits of precision, and blue is stored with 5 bits of precision.
     */
    RGB_565 = 2
  }

  /**
   * Enumerates color spaces.
   */
  enum ColorSpace {
    /**
     * Indicates an unknown color space.
     */
    UNKNOWN = 0,

    /**
     * Indicates the color space based on SMPTE RP 431-2-2007 and IEC 61966-2.1:1999.
     */
    DISPLAY_P3 = 1,

    /**
     * Indicates the standard red green blue (SRGB) color space based on IEC 61966-2.1:1999.
     */
    SRGB = 2,

    /**
     * Indicates the SRGB using a linear transfer function based on the IEC 61966-2.1:1999 standard.
     */
    LINEAR_SRGB = 3,

    /**
     * Indicates the color space based on IEC 61966-2-2:2003.
     */
    EXTENDED_SRGB = 4,

    /**
     * Indicates the color space based on IEC 61966-2-2:2003.
     */
    LINEAR_EXTENDED_SRGB = 5,

    /**
     * Indicates the color space based on the standard illuminant with D50 as the white point.
     */
    GENERIC_XYZ = 6,

    /**
     * Indicates the color space using CIE XYZ D50 as the profile conversion space.
     */
    GENERIC_LAB = 7,

    /**
     * Indicates the color space based on SMPTE ST 2065-1:2012.
     */
    ACES = 8,

    /**
     * Indicates the color space based on Academy S-2014-004.
     */
    ACES_CG = 9,

    /**
     * Indicates the color space based on Adobe RGB (1998).
     */
    ADOBE_RGB_1998 = 10,

    /**
     * Indicates the color space based on SMPTE RP 431-2-2007.
     */
    DCI_P3 = 11,

    /**
     * Indicates the color space based on Rec.ITU-R BT.709-5.
     */
    ITU_709 = 12,

    /**
     * Indicates the color space based on Rec.ITU-R BT.2020-1.
     */
    ITU_2020 = 13,

    /**
     * Indicates the color space based on ISO 22028-2:2013.
     */
    ROMM_RGB = 14,

    /**
     * Indicates the color space based on the NTSC 1953 standard.
     */
    NTSC_1953 = 15,

    /**
     * Indicates the color space based on SMPTE C.
     */
    SMPTE_C = 16
  }

  /**
   * Enumerates alpha types.
   */
  enum AlphaType {
    /**
     * Indicates an unknown alpha type.
     */
    UNKNOWN = 0,

    /**
     * Indicates that the image has no alpha channel, or all pixels in the image are fully opaque.
     */
    OPAQUE = 1,

    /**
     * Indicates that RGB components of each pixel in the image are premultiplied by alpha.
     */
    PREMUL = 2,

    /**
     * Indicates that RGB components of each pixel in the image are independent of alpha and are not premultiplied by alpha.
     */
    UNPREMUL = 3
  }

  /**
   * Describes the size of an image.
   */
  interface Size {
    height: number;
    width: number;
  }

  interface Region {
    size: Size;
    x: number;
    y: number;
  }

  interface PositionArea {
    pixels: ArrayBuffer;
    offset: number;
    stride: number;
    region: Region;
  }

  /**
   * Describes image information.
   */
  interface ImageInfo {
    size: Size;
    pixelFormat: PixelMapFormat;
    colorSpace: ColorSpace;
    alphaType: AlphaType;
  }

  /**
   * Describes the option for image packing.
   */
  interface PackingOption {
    /**
     * Multipurpose Internet Mail Extensions (MIME) format of the target image, for example, image/jpeg.
     */
    format: string;

    /**
     * Quality of the target image. The value is an integer ranging from 0 to 100. A larger value indicates better
     */
    quality: number;
  }

  interface DecodingOptions {
    // Indicates the sampling size based on which an image is scaled down.
    sampleSize: number;

    // Indicates the rotation angle, ranging from 0 to 360
    rotateDegrees: number;

    // Specifies whether pixel values of the pixel map to be decoded can be edited.
    editable: boolean;

    // Indicates the expected output size
    desiredSize: Size;

    // Indicates an image area to be decoded.
    desiredRegion: Region;

    // Indicates the pixel format of a decoded image, which is defined by PixelFormat.
    desiredPixelFormat: PixelMapFormat;

    // reused current pixelmap buffer address for a new created pixelmap
    reusedPixelMap: PixelMap;
  }

  enum ScaleMode {
    CENTER_CROP = 1, // Indicates the effect that scales an image to fill the target image area and center-crops the part outside the area.
    FIT_TARGET_SIZE = 2, // Indicates the effect that fits the image into the target size.
  }

  interface InitializationOptions {
    // Indicates the expected alpha type of the pixel map to be created
    alphaType: AlphaType;

    // Specifies whether pixel values of the pixel map to be created can be edited
    editable: boolean;

    // Indicates the expected format of the pixel map to be created
    pixelFormat: PixelMapFormat;

    // Indicates the scaling effect used when the aspect ratio of the original image is different from that of the target image
    scaleMode: ScaleMode;

    // Indicates the expected size of the pixel map to be created
    size: Size;
  }

  /**
   * Creates an ImageSource instance.
   */
  function createImageSource(uri: string): ImageSource;

  // create a imagesource based on fd
  function createImageSource(fd: number): ImageSource;

  // Creates an ImageSource based on a ArrayBuffer and source options.
  function createImageSource(data: ArrayBuffer): ImageSource;

  // Creates a pixel map based on initialization options (such as the image size, pixel format, and alpha type)
  // and the data source described by a pixel color array, start offset, and number of pixels in a row.
  function createPixelMap(colors: ArrayBuffer, opts: InitializationOptions): Promise<PixelMap>;
  function createPixelMap(colors: ArrayBuffer, opts: InitializationOptions, callback: AsyncCallback<PixelMap>): void;


  function createIncrementalSource(data: ArrayBuffer): ImageSource;

  /**
   * Creates an ImagePacker instance.
   */
  function createImagePacker(): ImagePacker;

  interface PixelMap {
    // read all pixels to an buffer
    readPixelsToBuffer(dst: ArrayBuffer): Promise<void>;
    readPixelsToBuffer(dst: ArrayBuffer, callback: AsyncCallback<void>): void;

    // read pixels from a specified area into an buffer
    readPixels(area: PositionArea): Promise<Array<number>>;
    readPixels(area: PositionArea, callback: AsyncCallback<Array<number>>): void;

    // write pixels to a specified area
    writePixels(area: PositionArea): Promise<void>;
    writePixels(area: PositionArea, callback: AsyncCallback<void>): void;

    // write array buffer to pixelmap
    writeBufferToPixels(src: ArrayBuffer): Promise<void>;
    writeBufferToPixels(src: ArrayBuffer, callback: AsyncCallback<void>): void;

    // obtains basic image information.
    getImageInfo(): Promise<ImageInfo>;
    getImageInfo(callback: AsyncCallback<ImageInfo>): void;

    // get bytes number per row.
    getBytesNumberPerRow(): Promise<number>;
    getBytesNumberPerRow(callback: AsyncCallback<number>): void;

    // get bytes buffer for a pixelmap.
    getPixelBytesNumber(): Promise<number>;
    getPixelBytesNumber(callback: AsyncCallback<number>): void;

    // release pixelmap
    release(): void;

    readonly isEditable: boolean;
  }

  interface ImageSource {
    /**
     * Obtains information about an image with the specified sequence number and uses a callback to return the result.
     */
    getImageInfo(index: number, callback: AsyncCallback<ImageInfo>): void;

    /**
     * Obtains information about this image and uses a callback to return the result.
     */
    getImageInfo(callback: AsyncCallback<ImageInfo>): void;

    /**
     * Get image information from image source.
     */
    getImageInfo(index?: number): Promise<ImageInfo>;

    // Obtains the integer value of a specified property key for an image at the given index in the ImageSource.
    getImagePropertyInt(index:number, key: string, defaultValue: number): Promise<number>;
    getImagePropertyInt(index:number, key: string, defaultValue: number, callback: AsyncCallback<number>): void;

    // Obtains the string value of a specified image property key.
    getImagePropertyString(key: string): Promise<string>;
    getImagePropertyString(key: string, callback: AsyncCallback<string>): void;

    // Decodes source image data based on a specified index location in the ImageSource and creates a pixel map.
    createPixelMap(index: number, options: DecodingOptions, callback: AsyncCallback<PixelMap>): void;
    createPixelMap(opts: DecodingOptions, callback: AsyncCallback<PixelMap>): void;

    // Updates incremental data to an image data source using a byte array with specified offset and length.
    updateData(data: Array<number>, isFinal: boolean, offset?: number, length?: number): Promise<boolean>; 
    updateData(data: Array<number>, isFinal: boolean, offset: number, length: number, callback: AsyncCallback<boolean>): void;
    updateData(data: Array<number>, isFinal: boolean, callback: AsyncCallback<boolean>): void;

    /**
     * Releases an ImageSource instance and uses a callback to return the result.
     */
    release(callback: AsyncCallback<void>): void;

    /**
     * Releases an ImageSource instance and uses a promise to return the result.
     */
    release(): Promise<void>;

    /**
     * Supported image formats.
     */
    readonly supportedFormats: Array<string>;
  }

  interface ImagePacker {
    /**
     * Compresses or packs an image and uses a callback to return the result.
     */
    packing(source: ImageSource, option: PackingOption, callback: AsyncCallback<Array<number>>): void;

    /**
     * Compresses or packs an image and uses a promise to return the result.
     */
    packing(source: ImageSource, option: PackingOption): Promise<Array<number>>;

    /**
     * Releases an ImagePacker instance and uses a callback to return the result.
     */
    release(callback: AsyncCallback<void>): void;

    /**
     * Releases an ImagePacker instance and uses a promise to return the result.
     */
    release(): Promise<void>;

    /**
     * Supported image formats.
     */
    readonly supportedFormats: Array<string>;
  }
}

export default image;