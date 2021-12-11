# MediaLibrary<a name="EN-US_TOPIC_0000001147574647"></a>

- [Introduction<a name="section1158716411637"></a>](#introduction)
- [Directory Structure<a name="section161941989596"></a>](#directory-structure)
- [Usage Guidelines<a name="usage-guidelines"></a>](#usage-guidelines)
    - [Query AudioAsset<a name="get-audioasset"></a>](#query-audioasset)
    - [Create Album<a name="create-album"></a>](#create-album)
    - [Copy ImageAsset<a name="copy-imageasset"></a>](#copy-imageasset)
- [Repositories Involved<a name="section1533973044317"></a>](#repositories-involved)


## Introduction<a name="section1158716411637"></a>

The **medialibrary\_standard**  repository provides a set of easy-to-use APIs for getting media file metadata information. 
MediaLibrary APIs can only be used internally, not exposed to public application currently.

The various capabilities can be categorized as below:
- Query for audio, video and image files metadata information
- Query for image and video albums
- File operations like create, rename, copy and delete media file
- Album operations like create, rename and delete album


## Directory Structure<a name="section161941989596"></a>

The structure of the repository directory is as follows:
```
/foundation/multimedia/medialibrary_standard    # Medialibrary code
├── frameworks                                  # Framework code
│   ├── innerkitsimpl                           # Internal Native API implementation
│   │   └── media_library                       # Native MediaLibrary implementation
│   └── kitsimpl                                # External JS API implementation
│       └── medialibrary                        # External MediaLibrary NAPI implementation
├── interfaces                                  # Interfaces
│   ├── innerkits                               # Internal Native APIs
│   └── kits                                    # External JS APIs
├── LICENSE                                     # License file
├── ohos.build                                  # Build file
├── sa_profile                                  # Service configuration profile
└── services                                    # Service implementation
```

## Usage Guidelines<a name="usage-guidelines"></a>
### Query AudioAsset<a name="get-audioasset"></a>
We can use APIs like **GetMediaAssets**, **GetAudioAssets**, **GetVideoAssets** and **GetImageAssets** to query for different file metadata information. The following steps describe how to use the GetAudioAssets API to obtain audio related metadata.
1. Use **GetMediaLibraryClientInstance** API to obtain the **Medialibrary** instance
    ```
    IMediaLibraryClient* mediaLibClientInstance = IMediaLibraryClient::GetMediaLibraryClientInstance();
    ```
2. Set the scanning directory for audio files in **selection** string. The selection value will be a relative path to a media root directory i.e. "/data/media". It will check for audio files recursively in the given directory.
    ```
    string selection = "audios/audio1";
    ```
3. Use the **GetAudioAssets** API to query for audio files. The input argument *selectionArgs* does not have any use case currently. The API returns a list of *AudioAsset*.
    ```
    vector<string> selectionArgs;
    vector<unique_ptr<AudioAsset>> audioAssets = mediaLibClientInstance->GetAudioAssets(selection, selectionArgs);
    ```
4. We can obtain audio metadata for each file from the list.
    ```
    for (size_t i = 0; i < audioAssets.size(); i++) {
        cout << audioAssets[i]->uri_ << endl;
        cout << audioAssets[i]->size_ << endl;
        cout << audioAssets[i]->dateAdded_ << endl;
        cout << audioAssets[i]->dateModified_ << endl;
        cout << audioAssets[i]->albumName_ << endl;
        cout << audioAssets[i]->duration_ << endl;
        cout << audioAssets[i]->artist_ << endl;
    }
    ```

### Create Album<a name="create-album"></a>
MediaLibrary offers APIs for application to perform album operations like create, modify and delete. Below are the steps on how to create a new album.
1. Use **GetMediaLibraryInstance** API to obtain the **Medialibrary** instance
    ```
    IMediaLibraryClient* mediaLibClientInstance = IMediaLibraryClient::GetMediaLibraryClientInstance();
    ```
2. Specify the album type. It can be either *ASSET_GENERIC_ALBUM*, *ASSET_IMAGEALBUM* or *ASSET_VIDEOALBUM*.
    ```
    AssetType assetType = ASSET_VIDEOALBUM;
    ```
3. Create a new **AlbumAsset** object and provide a new album name. Below album "new_video" will get created in the path "/data/media/videos".
    ```
    AlbumAsset albumAsset;
    albumAsset.albumName_ = "videos/new_video";
    ```
4. Use **CreateMediaAlbumAsset** API to create the new album in the device. The return value will be boolean, which states whether the album creation succeeded or failed.
    ```
    bool errCode = mediaLibClientInstance->CreateMediaAlbumAsset(assetType, albumAsset);
    ```

### Copy ImageAsset<a name="copy-imageasset"></a>
File operations are supported via APIs like **CreateMediaAsset**, **ModifyMediaAsset**, **CopyMediaAsset**, **DeleteMediaAsset**. The example below explains the usage of CopyMediaAsset API.
1. Use **GetMediaLibraryInstance** API to obtain the **Medialibrary** instance
    ```
    IMediaLibraryClient* mediaLibClientInstance = IMediaLibraryClient::GetMediaLibraryClientInstance();
    ```
2. Specify the asset type. It can be either *ASSET_MEDIA*, *ASSET_IMAGE*, *ASSET_AUDIO* or *ASSET_VIDEO*.
    ```
    AssetType assetType = ASSET_IMAGE;
    ```
3. Define the source and target **ImageAsset**. The target asset must specify the new album name where the source asset will be copied.
    ```
    MediaAsset srcMediaAsset;
    MediaAsset dstMediaAsset;

    srcMediaAsset.name_ = "image1.jpg";
    srcMediaAsset.uri_ = "/data/media/images/001/image1.jpg";

    dstMediaAsset.albumName_ = "images/new_image";
    ```
4. Use **CopyMediaAsset** API to copy the source asset to target asset's album name location. The boolean return value denotes the status of the file operation. The source file "image1.jpg" will get copied to "/data/media/images/new_image".
    ```
    bool errCode = mediaLibClientInstance->CopyMediaAsset(assetType, srcMediaAsset, dstMediaAsset);
    ```

Please refer [**IMediaLibraryClient.h**](https://gitee.com/openharmony/multimedia_medialibrary_standard/blob/master/interfaces/innerkits/native/include/IMediaLibraryClient.h) for more APIs.


## Repositories Involved<a name="section1533973044317"></a>
[multimedia/medialibrary_standard](https://gitee.com/openharmony/multimedia_medialibrary_standard)
