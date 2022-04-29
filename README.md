
---
# 小熊派-鸿蒙·叔(BearPi-HM Micro)
### 资料导航
| No | 类别 | 路径  |
| :----: | ---- |  ------------------------------------------------------------ |
| 1    | 开发板硬件资料 | [applications/BearPi/BearPi-HM_Micro/docs/board](/applications/BearPi/BearPi-HM_Micro/docs/board) |
| 2    | 快速上手教程 |  [applications/BearPi/BearPi-HM_Micro/docs/device-dev/](/applications/BearPi/BearPi-HM_Micro/docs/device-dev/BearPi-HM_Micro十分钟上手.md) |
| 3    | 案例代码列表 |  [applications/BearPi/BearPi-HM_Micro/sample/](/applications/BearPi/BearPi-HM_Micro/samples/README.md) |
| 4    | 视频学习地址 |  [B站](https://www.bilibili.com/video/BV12Y411H7ry) |
| 5    | 视频课程PPT |   [applications/BearPi/BearPi-HM_Micro/docs/board/PPT](/applications/BearPi/BearPi-HM_Micro/docs/board/PPT)|
| 6    | 常见问题 | [ISSUES](https://gitee.com/bearpi/bearpi-hm_micro_small/issues) |

### 注：如果您是北向应用开发者请移步[北向仓库](https://gitee.com/bearpi/bearpi-hm_micro_app/blob/master/README.md)学习

## 一、BearPi-HM Micro 开发板介绍

BearPi-HM Micro开发板是一块高度集成并可运行Openharmony系统的开发板，板载高性能的工业级处理器STM32MP157芯片，搭配4.3寸LCD电容式触摸屏，并板载wifi电路及标准的E53接口，标准的E53接口可扩展智能加湿器、智能台灯、智能安防、智能烟感等案例。可折叠式屏幕设计大大提高用户开发体验，便于携带和存放，更好地满足不同用户的需求，拓展无限可能。


## 二、快速上手（十分钟上手）

如果您想快速的体验一下BearPi-HM Micro开发板的源码`[获取]`、`[编译]`、`[烧录]`过程，强烈建议您学习以下教程，这能让您在环境安装环节省下不少时间。

1、视频教程（B站）


* [开发环境搭建(简单)—十分钟快速上手](https://www.bilibili.com/video/BV12Y411H7ry?p=2)

2、文档（Gitee）
* [VMware 镜像版本：十分钟上手体验](applications/BearPi/BearPi-HM_Micro/docs/device-dev/BearPi-HM_Micro十分钟上手.md)


## 三、设备开发手把手教程系列

1、视频系列（B站） 对应PPT地址
* [P1. 初识BearPi-HM Micro开发板](https://www.bilibili.com/video/BV12Y411H7ry?p=1)
* [P2. 开发环境搭建(复杂)——编译环境搭建](https://www.bilibili.com/video/BV12Y411H7ry?p=3)
* [P3. 开发环境搭建(复杂)——源码获取及编译](https://www.bilibili.com/video/BV12Y411H7ry?p=4)
* [P4. BearPi-HM Micro开发板源码烧录](https://www.bilibili.com/video/BV12Y411H7ry?p=5)
* [P5. OpenHarmony 快速入门：HelloWorld](https://www.bilibili.com/video/BV12Y411H7ry?p=6)
* [P6. OpenHarmony 快速入门：点亮LED](https://www.bilibili.com/video/BV12Y411H7ry?p=7)
* [P7. OpenHarmony 快速入门：安装HAP](https://www.bilibili.com/video/BV12Y411H7ry?p=8)
* [P8. OpenHarmony 快速入门：JS点亮LED](https://www.bilibili.com/video/BV12Y411H7ry?p=9)

2、文档系列（Gitee）
- [1. 初识BearPi-HM Micro开发板](applications/BearPi/BearPi-HM_Micro/docs/device-dev/初识BearPi-HM_Micro开发板.md)
- [2. 开发环境搭建](applications/BearPi/BearPi-HM_Micro/docs/device-dev/开发环境搭建.md)
- [3. 源码获取](applications/BearPi/BearPi-HM_Micro/docs/device-dev/源码获取.md)
- [4. 如何编译系统](applications/BearPi/BearPi-HM_Micro/docs/device-dev/如何编译系统.md)
- [5. 如何烧录固件并启动](applications/BearPi/BearPi-HM_Micro/docs/device-dev/如何烧录固件并启动.md)
- [6. 编写一个hello_world程序](applications/BearPi/BearPi-HM_Micro/docs/device-dev/编写一个hello_world程序.md)
- [7. 如何运行一个可执行程序](applications/BearPi/BearPi-HM_Micro/docs/device-dev/如何运行一个可执行程序.md)
- [8. 编写一个点亮LED灯程序](applications/BearPi/BearPi-HM_Micro/docs/device-dev/编写一个点亮LED灯程序.md)
- [9. 通过JS应用控制LED灯](applications/BearPi/BearPi-HM_Micro/docs/device-dev/通过JS应用控制LED灯.md)
- [10. 如何在开发板上安装HAP应用](applications/BearPi/BearPi-HM_Micro/docs/device-dev/如何在开发板上安装HAP应用.md)
- [11. 开发板Wi-Fi功能使用指导](applications/BearPi/BearPi-HM_Micro/docs/device-dev/开发板Wi-Fi功能使用指导.md)
- [12. E53案例开发](applications/BearPi/BearPi-HM_Micro/docs/device-dev/E53案例开发.md)
    - [1. 智慧农业E53_IA1扩展板开发](applications/BearPi/BearPi-HM_Micro/docs/device-dev/智慧农业E53_IA1扩展板功能使用指导.md)
    - [2. 智慧井盖E53_SC2扩展板开发](applications/BearPi/BearPi-HM_Micro/docs/device-dev/智慧井盖E53_SC2扩展板功能使用指导.md)
    - [3. 智慧路灯E53_SC1扩展板开发](applications/BearPi/BearPi-HM_Micro/docs/device-dev/智慧路灯E53_SC1扩展板功能使用指导.md)
    - [4. 智慧烟感E53_SF1扩展板开发](applications/BearPi/BearPi-HM_Micro/docs/device-dev/智慧烟感E53_SF1扩展板功能使用指导.md)
    - [5. 人体感应E53_IS1扩展板开发](applications/BearPi/BearPi-HM_Micro/docs/device-dev/人体感应E53_IS1扩展板功能使用指导.md)


3、开发者贡献（以下教程为开发者贡献）
- [1. 纯Linux搭建小熊派-鸿蒙·叔(BearPi-HM Micro)设备开发的开发环境](applications/BearPi/BearPi-HM_Micro/docs/device-dev/Linux下配置小熊派设备开发的开发环境.md)@nebula-yong

## 四、开发板图片预览[（淘宝链接）](https://item.taobao.com/item.htm?id=662078665554)

[![](/applications/BearPi/BearPi-HM_Micro/docs/figures/BearPi-HM_Micro_Info.png)](https://item.taobao.com/item.htm?id=633296694816)


## 五、联系我们



![](/applications/BearPi/BearPi-HM_Micro/docs/figures/bearpi_club_wechat.jpg)

