# State Registry<a name="EN-US_TOPIC_0000001152064139"></a>

-   [Introduction](#section117mcpsimp)
-   [Directory Structure](#section124mcpsimp)
-   [Constraints](#section128mcpsimp)
-   [Usage](#section134mcpsimp)
    -   [Available APIs](#section136mcpsimp)

-   [Usage Guidelines](#section163mcpsimp)
    -   [Parameters of C APIs](#section1099113151207)
    -   [Sample Code](#section1558565082915)

-   [Repositories Involved](#section206mcpsimp)

## Introduction<a name="section117mcpsimp"></a>

The state registry module provides APIs to register and deregister an observer that listens for various callback events of the telephony subsystem. Such events include but are not limited to the following: network status change, signal strength change, cell information change, cellular data connection status change, and call status change.

**Figure  1**  Architecture of the state registry module<a name="fig13267152558"></a>
![](figures/en-us-architecture-of-the-state-registry-module.png)

## Directory Structure<a name="section124mcpsimp"></a>

```
/base/telephony/state_registry      # State registry service
├─ BUILD.gn                         # Build script (gn)
├─ README.md                        # Readme
├─ interfaces                       # JS APIs
├─ service
│  ├─ include                       # Header files
│  └─ src                           # Source files
├─ sa_profile                       # SA profile
├─ ohos.build                       # Build code
└─ test                             # Test code
```

## Constraints<a name="section128mcpsimp"></a>

-   Programming language: JavaScript
-   Software constraints: this service needs to work with the telephony core service \(core\_service\).
-   Hardware constraints: the accommodating device must be equipped with a modem and a SIM card capable of independent cellular communication.
-   The API for registering an observer for the SIM card status takes effect only when SIM cards are in position. If SIM cards are removed, no callback events will be received. Your application can call the  **getSimState**  API to check whether SIM cards are in position.

## Usage<a name="section134mcpsimp"></a>

### Available APIs<a name="section136mcpsimp"></a>

**Table  1**  Registration APIs

<a name="table165976561598"></a>
<table><thead align="left"><tr id="row1059785615915"><th class="cellrowborder" valign="top" width="50.019999999999996%" id="mcps1.2.3.1.1"><p id="p81665114103"><a name="p81665114103"></a><a name="p81665114103"></a>API</p>
</th>
<th class="cellrowborder" valign="top" width="49.980000000000004%" id="mcps1.2.3.1.2"><p id="p916145121017"><a name="p916145121017"></a><a name="p916145121017"></a>Description</p>
</th>
</tr>
</thead>
<tbody><tr id="row137081297171"><td class="cellrowborder" valign="top" width="50.019999999999996%" headers="mcps1.2.3.1.1 "><p id="p570813931718"><a name="p570813931718"></a><a name="p570813931718"></a>function on(type: String, options: { slotId?: number }, callback: AsyncCallback&lt;T&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="49.980000000000004%" headers="mcps1.2.3.1.2 "><p id="p770811916175"><a name="p770811916175"></a><a name="p770811916175"></a>Registers an observer.</p>
</td>
</tr>
<tr id="row176541675174"><td class="cellrowborder" valign="top" width="50.019999999999996%" headers="mcps1.2.3.1.1 "><p id="p06544714174"><a name="p06544714174"></a><a name="p06544714174"></a>function off(type: String, callback?: AsyncCallback&lt;T&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="49.980000000000004%" headers="mcps1.2.3.1.2 "><p id="p26546716175"><a name="p26546716175"></a><a name="p26546716175"></a>Deregisters an observer.</p>
</td>
</tr>
<tr id="row1526612541718"><td class="cellrowborder" valign="top" width="50.019999999999996%" headers="mcps1.2.3.1.1 "><p id="p62673520171"><a name="p62673520171"></a><a name="p62673520171"></a>function once(type: String, options: { slotId?: number }, callback: AsyncCallback&lt;T&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="49.980000000000004%" headers="mcps1.2.3.1.2 "><p id="p152671855177"><a name="p152671855177"></a><a name="p152671855177"></a>Registers a one-time observer.</p>
</td>
</tr>
</tbody>
</table>

## Usage Guidelines<a name="section163mcpsimp"></a>

### Parameters of C APIs<a name="section1099113151207"></a>

Different subscription events are distinguished by the  **type**  parameter. The following table lists the related  **type**  parameters.

**Table  2**  Description of type parameters

<a name="table1234838197"></a>
<table><thead align="left"><tr id="row82351335191"><th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.1"><p id="p2023519312196"><a name="p2023519312196"></a><a name="p2023519312196"></a>Parameter</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.2"><p id="p1823516361916"><a name="p1823516361916"></a><a name="p1823516361916"></a>Description</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.3"><p id="p17904634202019"><a name="p17904634202019"></a><a name="p17904634202019"></a>Required Permission</p>
</th>
</tr>
</thead>
<tbody><tr id="row122350371913"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p22351321915"><a name="p22351321915"></a><a name="p22351321915"></a>networkStateChange</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p142353317193"><a name="p142353317193"></a><a name="p142353317193"></a>Network status change event</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p15933202217"><a name="p15933202217"></a><a name="p15933202217"></a>ohos.permission.GET_NETWORK_INFO</p>
</td>
</tr>
<tr id="row9235183101918"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p1523593201916"><a name="p1523593201916"></a><a name="p1523593201916"></a>signalInfoChange</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p1123553161910"><a name="p1123553161910"></a><a name="p1123553161910"></a>Signal change event</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p2904134182011"><a name="p2904134182011"></a><a name="p2904134182011"></a>None</p>
</td>
</tr>
<tr id="row823512391918"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p1823516319196"><a name="p1823516319196"></a><a name="p1823516319196"></a>cellularDataConnectionStateChange</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p8235103161914"><a name="p8235103161914"></a><a name="p8235103161914"></a>Cellular data connection status change event</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p1790403492014"><a name="p1790403492014"></a><a name="p1790403492014"></a>None</p>
</td>
</tr>
<tr id="row823510321915"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p1423523191911"><a name="p1423523191911"></a><a name="p1423523191911"></a>cellularDataFlowChange</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p423515361917"><a name="p423515361917"></a><a name="p423515361917"></a>Cellular data flow change event</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p1190463416203"><a name="p1190463416203"></a><a name="p1190463416203"></a>None</p>
</td>
</tr>
<tr id="row223563151918"><td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.1 "><p id="p2235153191910"><a name="p2235153191910"></a><a name="p2235153191910"></a>callStateChange</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.2 "><p id="p123513331917"><a name="p123513331917"></a><a name="p123513331917"></a>Call status change event, in which the value of <strong id="b82669496919"><a name="b82669496919"></a><a name="b82669496919"></a>phoneNumber</strong> is empty if the user does not have the required permission.</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p1828812257217"><a name="p1828812257217"></a><a name="p1828812257217"></a>ohos.permission.READ_CALL_LOG</p>
</td>
</tr>
</tbody>
</table>

### Sample Code<a name="section1558565082915"></a>

The function of registering an observer for call status change events is used as an example. The process is as follows:

1.  Call the  **on**  method or  **once**  method with the  **type**  parameter specified to register an observer for different types of events.
2.  Check whether the registration is successful. If  **err**  is empty in the received callback, the registration is successful. If  **err**  is not empty, the registration has failed. Obtain the required data from  **value**  if the registration is successful. 
3.  Call the  **off**  method to deregister the observer. After the observer is deregistered, no callback will be received.

    ```
    // Import the observer package.
    import observer from '@ohos.telephony.observer';

    // Registers an observer.
    observer.on('callStateChange', {slotId: 1}, (err, value) => {
      if (err) {
        // If the API call failed, err is not empty.
        console.error(`failed, because ${err.message}`);
        return;
      }
      // If the API call succeeded, err is empty.
      console.log(`success on. number is ` + value.number + ", state is " + value.state);
    });

    // Register a one-time observer.
    observer.once('callStateChange', {slotId: 1}, (err, value) => {
      if (err) {
        // If the API call failed, err is not empty.
        console.error(`failed, because ${err.message}`);
        return;
      }
      // If the API call succeeded, err is empty.
      console.log(`success once. number is ` + value.number + ", state is " + value.state);
    });

    // Deregister the observer.
    observer.off('callStateChange', (err, value) => {
      if (err) {
        // If the API call failed, err is not empty.
        console.error(`failed, because ${err.message}`);
        return;
      }
      // If the API call succeeded, err is empty.
      console.log(`success off`);
    });
    ```


## Repositories Involved<a name="section206mcpsimp"></a>

[Telephony](https://gitee.com/openharmony/docs/blob/master/en/readme/telephony.md)

**telephony_state_registry**

[telephony_core_service](https://gitee.com/openharmony/telephony_core_service/blob/master/README.md)

[telephony_cellular_call](https://gitee.com/openharmony/telephony_cellular_call/blob/master/README.md)

[telephony_call_manager](https://gitee.com/openharmony/telephony_call_manager/blob/master/README.md)