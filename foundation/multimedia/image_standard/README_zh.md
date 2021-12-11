# Image组件<a name="ZH-CN_TOPIC_0000001139841951"></a>

-   [简介](#section11660541593)
-   [目录](#section161941989596)
-   [相关仓](#section1533973044317)

## 简介<a name="section11660541593"></a>

Image组件提供了简单易用的接口用于开发图像的编解码功能，目前支持主流的如下图片格式：JPEG、PNG、BMP

**图 1**  Image组件架构图<a name="fig99659301300"></a>  
![](figures/Image组件架构图.png "Image组件架构图")

## 目录<a name="section161941989596"></a>

仓目录结构如下：

```
/foundation/multimedia/image      # Image组件业务代码
├── frameworks                    # 框架代码
│   ├── innerkitsimpl             # 框架native层业务实现
│   └── jni                       # jni层实现
├── ohos.build                    # 编译配置
├── interfaces                    # 外部接口层
│   ├── innerkits                 # 内部其他子系统接口
│   └── kits                      # java接口层
├── plugins                       # 图像插件实现
│   ├── common                    # 图像通用插件
│   └── manager                   # 图像插件管理模块
├── test                          # 测试资源目录
```

## 相关仓<a name="section1533973044317"></a>

hmf/multimedia/image

