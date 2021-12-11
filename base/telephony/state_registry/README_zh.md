# 状态注册<a name="ZH-CN_TOPIC_0000001152064139"></a>

-   [简介](#section117mcpsimp)
-   [目录](#section124mcpsimp)
-   [约束](#section128mcpsimp)
-   [说明](#section134mcpsimp)
    -   [接口说明](#section136mcpsimp)

-   [使用说明](#section163mcpsimp)
    -   [订阅接口参数说明](#section1099113151207)
    -   [接口调用代码示例](#section1558565082915)

-   [相关仓](#section206mcpsimp)

## 简介<a name="section117mcpsimp"></a>

状态注册主要负责提供电话服务子系统各种消息事件的订阅以及取消订阅的API。事件类型包括网络状态变化、信号强度变化、小区信息变化、蜂窝数据连接状态变化、通话状态变化等等。

**图 1**  状态注册架构图<a name="fig13267152558"></a>
![](figures/zh-cn-architecture-of-the-state-registry-module.png)

## 目录<a name="section124mcpsimp"></a>

```
/base/telephony/state_registry      # 状态注册转发服务
├─ BUILD.gn                         # 编译gn脚本
├─ README.md                        # Readme文档
├─ interfaces                       # API,js文件
├─ service
│  ├─ include                       # 头文件
│  └─ src                           # 源文件
├─ sa_profile                       # sa文件
├─ ohos.build                       # 编译build
└─ test                             # 测试相关
```

## 约束<a name="section128mcpsimp"></a>

-   开发语言：JavaScript。
-   软件约束：需要与以下服务配合使用：Telephony核心服务（core\_service）。
-   硬件约束：需要搭载的设备支持以下硬件：可以进行独立蜂窝通信的Modem以及SIM卡。
-   使用场景：注册获取SIM卡状态接口仅针对有SIM卡在位场景生效，若用户拔出SIM卡，则接收不到回调事件。应用可通过调用getSimState接口来确定当前卡槽是否有卡在位。

## 说明<a name="section134mcpsimp"></a>

### 接口说明<a name="section136mcpsimp"></a>

**表 1**  注册接口

<a name="table165976561598"></a>
<table><thead align="left"><tr id="row1059785615915"><th class="cellrowborder" valign="top" width="50.019999999999996%" id="mcps1.2.3.1.1"><p id="p81665114103"><a name="p81665114103"></a><a name="p81665114103"></a>接口定义</p>
</th>
<th class="cellrowborder" valign="top" width="49.980000000000004%" id="mcps1.2.3.1.2"><p id="p916145121017"><a name="p916145121017"></a><a name="p916145121017"></a><strong id="b15161551111018"><a name="b15161551111018"></a><a name="b15161551111018"></a>接口描述</strong></p>
</th>
</tr>
</thead>
<tbody><tr id="row137081297171"><td class="cellrowborder" valign="top" width="50.019999999999996%" headers="mcps1.2.3.1.1 "><p id="p570813931718"><a name="p570813931718"></a><a name="p570813931718"></a>function on(type: String, options: { slotId?: number }, callback: AsyncCallback&lt;T&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="49.980000000000004%" headers="mcps1.2.3.1.2 "><p id="p770811916175"><a name="p770811916175"></a><a name="p770811916175"></a>开启订阅</p>
</td>
</tr>
<tr id="row176541675174"><td class="cellrowborder" valign="top" width="50.019999999999996%" headers="mcps1.2.3.1.1 "><p id="p06544714174"><a name="p06544714174"></a><a name="p06544714174"></a>function off(type: String, callback?: AsyncCallback&lt;T&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="49.980000000000004%" headers="mcps1.2.3.1.2 "><p id="p26546716175"><a name="p26546716175"></a><a name="p26546716175"></a>关闭订阅</p>
</td>
</tr>
<tr id="row1526612541718"><td class="cellrowborder" valign="top" width="50.019999999999996%" headers="mcps1.2.3.1.1 "><p id="p62673520171"><a name="p62673520171"></a><a name="p62673520171"></a>function once(type: String, options: { slotId?: number }, callback: AsyncCallback&lt;T&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="49.980000000000004%" headers="mcps1.2.3.1.2 "><p id="p152671855177"><a name="p152671855177"></a><a name="p152671855177"></a>一次性订阅</p>
</td>
</tr>
</tbody>
</table>

## 使用说明<a name="section163mcpsimp"></a>

### 订阅接口参数说明<a name="section1099113151207"></a>

不同订阅事件通过type进行区分，type列表如下：

**表 2**  type参数说明

<a name="table1234838197"></a>
<table><thead align="left"><tr id="row82351335191"><th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.1"><p id="p2023519312196"><a name="p2023519312196"></a><a name="p2023519312196"></a>type参数</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.2"><p id="p1823516361916"><a name="p1823516361916"></a><a name="p1823516361916"></a>说明</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.3"><p id="p17904634202019"><a name="p17904634202019"></a><a name="p17904634202019"></a>所需权限</p>
</th>
</tr>
</thead>
<tbody><tr id="row122350371913"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p22351321915"><a name="p22351321915"></a><a name="p22351321915"></a>networkStateChange</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p142353317193"><a name="p142353317193"></a><a name="p142353317193"></a>网络状态变化事件</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p15933202217"><a name="p15933202217"></a><a name="p15933202217"></a>ohos.permission.GET_NETWORK_INFO</p>
</td>
</tr>
<tr id="row9235183101918"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p1523593201916"><a name="p1523593201916"></a><a name="p1523593201916"></a>signalInfoChange</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p1123553161910"><a name="p1123553161910"></a><a name="p1123553161910"></a>信号变化事件</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p2904134182011"><a name="p2904134182011"></a><a name="p2904134182011"></a>无</p>
</td>
</tr>
<tr id="row823512391918"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p1823516319196"><a name="p1823516319196"></a><a name="p1823516319196"></a>cellularDataConnectionStateChange</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p8235103161914"><a name="p8235103161914"></a><a name="p8235103161914"></a>蜂窝数据连接状态事件</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p1790403492014"><a name="p1790403492014"></a><a name="p1790403492014"></a>无</p>
</td>
</tr>
<tr id="row823510321915"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p1423523191911"><a name="p1423523191911"></a><a name="p1423523191911"></a>cellularDataFlowChange</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p423515361917"><a name="p423515361917"></a><a name="p423515361917"></a>蜂窝数据流变化事件</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p1190463416203"><a name="p1190463416203"></a><a name="p1190463416203"></a>无</p>
</td>
</tr>
<tr id="row223563151918"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p2235153191910"><a name="p2235153191910"></a><a name="p2235153191910"></a>callStateChange</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p123513331917"><a name="p123513331917"></a><a name="p123513331917"></a>通话状态变化事件，其中phoneNumber没有权限返回空字符串</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p1828812257217"><a name="p1828812257217"></a><a name="p1828812257217"></a>ohos.permission.READ_CALL_LOG</p>
</td>
</tr>
</tbody>
</table>

### 接口调用代码示例<a name="section1558565082915"></a>

以订阅通话状态变化事件为例，主要步骤和代码如下：

1.  不同的事件指定不同的type，传入参数，调用on方法（开启订阅）或once方法（一次性订阅）。
2.  收到回调后，若err为空，则订阅成功，否则订阅失败。订阅成功则可以从value中获取数据。
3.  可以通过off方法取消订阅。取消订阅之后，不会再收到任何回调。

    ```
    // 引入包名
    import observer from '@ohos.telephony.observer';

    // 开启订阅
    observer.on('callStateChange', {slotId: 1}, (err, value) => {
      if (err) {
        // 接口调用失败，err非空
        console.error(`failed, because ${err.message}`);
        return;
      }
      // 接口调用成功，err为空
      console.log(`success on. number is ` + value.number + ", state is " + value.state);
    });

    // 只订阅一次
    observer.once('callStateChange', {slotId: 1}, (err, value) => {
      if (err) {
        // 接口调用失败，err非空
        console.error(`failed, because ${err.message}`);
        return;
      }
      // 接口调用成功，err为空
      console.log(`success once. number is ` + value.number + ", state is " + value.state);
    });

    // 关闭订阅
    observer.off('callStateChange', (err, value) => {
      if (err) {
        // 接口调用失败，err非空
        console.error(`failed, because ${err.message}`);
        return;
      }
      // 接口调用成功，err为空
      console.log(`success off`);
    });
    ```


## 相关仓<a name="section206mcpsimp"></a>

[电话服务子系统](https://gitee.com/openharmony/docs/blob/master/zh-cn/readme/%E7%94%B5%E8%AF%9D%E6%9C%8D%E5%8A%A1%E5%AD%90%E7%B3%BB%E7%BB%9F.md)

**telephony_state_registry**

[telephony_core_service](https://gitee.com/openharmony/telephony_core_service/blob/master/README_zh.md)

[telephony_cellular_call](https://gitee.com/openharmony/telephony_cellular_call/blob/master/README_zh.md)

[telephony_call_manager](https://gitee.com/openharmony/telephony_call_manager/blob/master/README_zh.md)