# 显示管理组件<a name="ZH-CN_TOPIC_0000001152026155"></a>

-   [简介](#section11660541593)
-   [目录](#section19472752217)
-   [相关仓](#section63151229062)

## 简介<a name="section11660541593"></a>

显示管理组件主要负责显示屏的亮/灭、亮度调节等功能，如下：

1.  显示屏的亮/灭。
2.  显示屏亮度调节。

**图 1**  电源管理子系统架构图<a name="fig106301571239"></a>  
![](figures/power-management-subsystem-architecture.png "电源管理子系统架构图")

## 目录<a name="section19472752217"></a>

```
base/powermgr/display_manager$
├── interfaces      # 接口层
│   ├── innerkits   # 内部接口
│   └── kits        # 外部接口
├── sa_profile      # SA配置文件
├── services        # 服务层
│   ├── native      # Native层
│   └── zidl        # zidl层
└── utils           # 工具和通用层
```

## 相关仓<a name="section63151229062"></a>

电源管理子系统

powermgr_battery_manager

powermgr_power_manager

**powermgr_display_manager**
