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

import { AsyncCallback } from './basic';

declare namespace mediaLibrary {

    /**
     * Returns an instance of MediaLibraryHelper
     *
     * @return Instance of MediaLibraryHelper
     * @version1
     */
    function getMediaLibraryHelper(): MediaLibraryHelper;

    /**
     * Enumeration types for different kind of Media Files
     *
     * @version1
     */
    enum MediaType {
        DEFAULT = 0,
        FILE,
        MEDIA,
        IMAGE,
        VIDEO,
        AUDIO,
        ALBUM_LIST,
        ALBUM_LIST_INFO
    }

    /**
     * Defines the media asset.
     *
     * @SysCap SystemCapability.Multimedia.MediaLibrary
     * @devices common
     * @version 1
     */
    export interface MediaAsset {
        id: number;
        URI: string;
        mediaType: MediaType;
        name: string;
        size: number;
        dateAdded: number;
        dateModified: number;
        albumId: number;
        albumName: string;

        /**
         * Start setting the properties for a newly created media asset
         *
         * @return Returns whether property setting can be started for a created media asset
         * @version 1
         */
        startCreate(callback: AsyncCallback<boolean>): void;
        startCreate(): Promise<boolean>;

        /**
         * Start modifying the properties for an existing media asset
         *
         * @return Returns whether property modification can be started for a media asset
         * @version 1
         */
        startModify(callback: AsyncCallback<boolean>): void;
        startModify(): Promise<boolean>;

        /**
         * Cancel the creation of a media asset
         *
         * @return Returns whether creation of a media asset was cancelled or not
         * @version 1
         */
        cancelCreate(callback: AsyncCallback<boolean>): void;
        cancelCreate(): Promise<boolean>;

        /**
         * Cancel the modification of a media asset
         *
         * @return Returns whether modification of a media asset was cancelled or not
         * @version 1
         */
        cancelModify(callback: AsyncCallback<boolean>): void;
        cancelModify(): Promise<boolean>;

        /**
         * Commit the creation of a media asset
         *
         * @return Returns whether creation of a media asset was committed or not
         * @version 1
         */
        commitCreate(callback: AsyncCallback<boolean>): void;
        commitCreate(): Promise<boolean>;

        /**
         * Delete a media asset
         *
         * @return Returns whether deletion of a media asset was successful or not
         * @version 1
         */
        commitDelete(callback: AsyncCallback<boolean>): void;
        commitDelete(): Promise<boolean>;

        /**
         * Commit the modification of a media asset
         *
         * @return Returns whether modification of a media asset was committed or not
         * @version 1
         */
        commitModify(callback: AsyncCallback<boolean>): void;
        commitModify(): Promise<boolean>;

        /**
         * Copy a media asset properties to the target media asset
         *
         * @param target The target media asset on which the source properties are to be copied
         * @return Returns whether copying of a media asset to the target was successful or not
         * @version 1
         */
        commitCopy(target: MediaAsset, callback: AsyncCallback<boolean>): void;
        commitCopy(target: MediaAsset): Promise<boolean>;
    }

    /**
     * Defines the audio asset.
     *
     * @SysCap SystemCapability.Multimedia.MediaLibrary
     * @devices common
     * @version 1
     */
    export interface AudioAsset extends MediaAsset {
        mimeType: string;
        title: string;
        artist: string;
        duration: number;
    }

    /**
     * Defines the video asset.
     *
     * @SysCap SystemCapability.Multimedia.MediaLibrary
     * @devices common
     * @version 1
     */
    export interface VideoAsset extends MediaAsset {
        mimeType: string;
        width: number;
        height: number;
        duration: number;
    }

    /**
     * Defines the image asset.
     *
     * @SysCap SystemCapability.Multimedia.MediaLibrary
     * @devices common
     * @version 1
     */
    export interface ImageAsset extends MediaAsset {
        mimeType: string;
        width: number;
        height: number;
    }

    /**
     * Defines the album.
     *
     * @SysCap SystemCapability.Multimedia.MediaLibrary
     * @devices common
     * @version 1
     */
    export interface Album {
        albumName : string;
        albumId : number;

        /**
         * Gets all video assets in album.
         *
         * @return Returns video assets list as asynchronous response
         * @version 1
         */
        getVideoAssets(): Promise<VideoAssets>;
        getVideoAssets(callback: AsyncCallback<VideoAssets>): void;

        /**
         * Gets all image assets in album.
         *
         * @return Returns image assets list as asynchronous response
         * @version 1
         */
        getImageAssets(): Promise<ImageAssets>;
        getImageAssets(callback: AsyncCallback<ImageAssets>): void;

        /**
         * Commit the creation of an album
         *
         * @return Returns whether creation of an album was committed or not
         * @version 1
         */
        commitCreate(callback: AsyncCallback<boolean>): void;
        commitCreate(): Promise<boolean>;

        /**
         * Delete an album
         *
         * @return Returns whether deletion of an album was successful or not
         * @version 1
         */
        commitDelete(callback: AsyncCallback<boolean>): void;
        commitDelete(): Promise<boolean>;

        /**
         * Commit the modification of an album
         *
         * @return Returns whether modification of an album was committed or not
         * @version 1
         */
        commitModify(callback: AsyncCallback<boolean>): void;
        commitModify(): Promise<boolean>;
    }

    /**
     * Fetch parameters applicable on images, videos, audios, albums and other media
     *
     * @version 1
     */
    export interface MediaFetchOptions {
        selections: string;
        selectionArgs: Array<string>;
    }

    type MediaAssets = Array<Readonly<MediaAsset>>;

    type AudioAssets = Array<Readonly<AudioAsset>>;

    type VideoAssets = Array<Readonly<VideoAsset>>;

    type ImageAssets = Array<Readonly<ImageAsset>>;

    type Albums = Array<Readonly<Album>>;

    /**
     * Defines the MediaLibraryHelper class and provides functions to access the data in media storage.
     *
     * @SysCap SystemCapability.Multimedia.MediaLibrary
     * @devices common
     * @version 1
     */
    export class MediaLibraryHelper {

        /**
         * Gets all media assets from system.
         *
         * @param options Fetch options with selection strings based on which to select the media assets
         * @return Returns media assets list as asynchronous response
         * @version 1
         */
        getMediaAssets(callback: AsyncCallback<MediaAssets>): void;
        getMediaAssets(options: MediaFetchOptions, callback: AsyncCallback<MediaAssets>): void;
        getMediaAssets(options?: MediaFetchOptions): Promise<MediaAssets>;

        /**
         * Gets all audio assets from system.
         *
         * @param options Fetch options with selection strings based on which to select the audio assets
         * @return Returns audio assets list as asynchronous response
         * @version 1
         */
        getAudioAssets(callback: AsyncCallback<AudioAssets>): void;
        getAudioAssets(options: MediaFetchOptions, callback: AsyncCallback<AudioAssets>): void;
        getAudioAssets(options?: MediaFetchOptions): Promise<AudioAssets>;

        /**
         * Gets all video assets from system.
         *
         * @param options Fetch options with selection strings based on which to select the video assets
         * @return Returns video assets list as asynchronous response
         * @version 1
         */
        getVideoAssets(callback: AsyncCallback<VideoAssets>): void;
        getVideoAssets(options: MediaFetchOptions, callback: AsyncCallback<VideoAssets>): void;
        getVideoAssets(options?: MediaFetchOptions): Promise<VideoAssets>;

        /**
         * Gets all image assets from system.
         *
         * @param options Fetch options with selection strings based on which to select the image assets
         * @return Returns image assets list as asynchronous response
         * @version 1
         */
        getImageAssets(callback: AsyncCallback<ImageAssets>): void;
        getImageAssets(options: MediaFetchOptions, callback: AsyncCallback<ImageAssets>): void;
        getImageAssets(options?: MediaFetchOptions): Promise<ImageAssets>;

        /**
         * Gets video album from system
         *
         * @param options Fetch options with selection strings based on which to select the albums
         * @return Returns video album as asynchronous response
         * @version 1
         */
        getVideoAlbums(options: MediaFetchOptions, callback: AsyncCallback<Albums>): void;
        getVideoAlbums(options: MediaFetchOptions): Promise<Albums>;

        /**
         * Gets image album from system
         *
         * @param options Fetch options with selection strings based on which to select the albums
         * @return Returns image album as asynchronous response
         * @version 1
         */
        getImageAlbums(options: MediaFetchOptions, callback: AsyncCallback<Albums>): void;
        getImageAlbums(options: MediaFetchOptions): Promise<Albums>;

        /**
         * Create a video asset with empty properties
         *
         * @return Returns a video asset as asynchronous response
         * @version 1
         */
        createVideoAsset(callback: AsyncCallback<VideoAsset>): void;
        createVideoAsset(): Promise<VideoAsset>;

        /**
         * Create an image asset with empty properties
         *
         * @return Returns an image asset as asynchronous response
         * @version 1
         */
        createImageAsset(callback: AsyncCallback<ImageAsset>): void;
        createImageAsset(): Promise<ImageAsset>;

        /**
         * Create an audio asset with empty properties
         *
         * @return Returns an audio asset as asynchronous response
         * @version 1
         */
        createAudioAsset(callback: AsyncCallback<AudioAsset>): void;
        createAudioAsset(): Promise<AudioAsset>;

        /**
         * Create an album with empty properties
         *
         * @return Returns an album as asynchronous response
         * @version 1
         */
        createAlbum(callback: AsyncCallback<Album>): void;
        createAlbum(): Promise<Album>;
    }
}
