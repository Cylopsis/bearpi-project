/* 
 * Copyright (c) 2021 Nanjing Xiaoxiongpai Intelligent Technology CO., LIMITED.
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


#include "device_resource_if.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "osal_io.h"
#include "osal_mem.h"
#include "osal_spinlock.h"
#include "watchdog_core.h"
#include "watchdog_if.h"

#define HDF_LOG_TAG stm32mp1_iwdg

// #define BIT(n)  (0x1U<<(n))

/* IWDG registers */
#define IWDG_KR		0x00 /* Key register */
#define IWDG_PR		0x04 /* Prescaler Register */
#define IWDG_RLR	0x08 /* ReLoad Register */
#define IWDG_SR		0x0C /* Status Register */
#define IWDG_WINR	0x10 /* Windows Register */

/* IWDG_KR register bit mask */
#define KR_KEY_RELOAD	0xAAAA /* reload counter enable */
#define KR_KEY_ENABLE	0xCCCC /* peripheral enable */
#define KR_KEY_EWA	    0x5555 /* write access enable */
#define KR_KEY_DWA	    0x0000 /* write access disable */

/* IWDG_PR register */
#define PR_SHIFT	2
#define PR_MIN		BIT(PR_SHIFT)

/* IWDG_SR register bit mask */
#define SR_PVU	BIT(0) /* Watchdog prescaler value update */
#define SR_RVU	BIT(1) /* Watchdog counter reload value update */

/* set timeout to 100000 us */
#define TIMEOUT_US	100000
#define SLEEP_US	1000

struct Stm32Mp1Iwdg {
    struct WatchdogCntlr wdt;
    volatile unsigned char *regBase;
    uint32_t phyBase;
    uint32_t regStep;
    OsalSpinlock lock;
};

static inline void RegWrite(void *base, uint32_t reg, uint32_t val)
{
    writel(val, (uint32_t)base + reg);
}

static int32_t Stm32mp1IwdgStart(struct WatchdogCntlr *wdt)
{
    struct Stm32Mp1Iwdg *hwdt = NULL;

    if (wdt == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    hwdt = (struct Stm32Mp1Iwdg *)wdt;
    
    // start 
    
    return HDF_SUCCESS;
}

#define HIWDT_CLOCK_HZ (3 * 1000 * 1000)
static int32_t Stm32mp1IwdgSetTimeout(struct WatchdogCntlr *wdt, uint32_t seconds)
{
    unsigned int value;
    unsigned int maxCnt = ~0x00;
    unsigned int maxSeconds = maxCnt / HIWDT_CLOCK_HZ;
    struct Stm32Mp1Iwdg *hwdt = NULL;

    if (seconds == 0 || seconds > maxSeconds) {
        value = maxCnt;
    } else {
        value = seconds * HIWDT_CLOCK_HZ;
    }

    if (wdt == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    hwdt = (struct Stm32Mp1Iwdg *)wdt;

    // set time out

    return HDF_SUCCESS;
}

static int32_t Stm32mp1IwdgGetTimeout(struct WatchdogCntlr *wdt, uint32_t *seconds)
{
    // unsigned int value;
    struct Stm32Mp1Iwdg *hwdt = NULL;

    if (wdt == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    hwdt = (struct Stm32Mp1Iwdg *)wdt;

    // get timeout
    
    return HDF_SUCCESS;
}

static int32_t Stm32mp1IwdgFeed(struct WatchdogCntlr *wdt)
{
    struct Stm32Mp1Iwdg *hwdt = NULL;

    if (wdt == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    hwdt = (struct Stm32Mp1Iwdg *)wdt;

    /* reload watchdog */
    RegWrite((void *)hwdt->regBase, IWDG_KR, KR_KEY_RELOAD);
    return HDF_SUCCESS;
}

static struct WatchdogMethod g_stm32mp1IwdgMethod = {
    // .getStatus = NULL,
    .start = Stm32mp1IwdgStart,
    // .stop = NULL,
    .setTimeout = Stm32mp1IwdgSetTimeout,
    .getTimeout = Stm32mp1IwdgGetTimeout,
    .feed = Stm32mp1IwdgFeed,
};

static int32_t Stm32mp1IwdgReadDrs(struct Stm32Mp1Iwdg *hwdt, const struct DeviceResourceNode *node)
{
    int32_t ret;
    struct DeviceResourceIface *drsOps = NULL;

    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetUint32 == NULL) {
        HDF_LOGE("%s: invalid drs ops!", __func__);
        return HDF_FAILURE;
    }

    ret = drsOps->GetUint32(node, "regBase", &hwdt->phyBase, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read regBase fail!", __func__);
        return ret;
    }

    ret = drsOps->GetUint32(node, "regStep", &hwdt->regStep, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read regStep fail!", __func__);
        return ret;
    }

    return HDF_SUCCESS;
}

static int32_t Stm32mp1IwdgBind(struct HdfDeviceObject *device)
{
    int32_t ret;
    struct Stm32Mp1Iwdg *hwdt = NULL;

    HDF_LOGI("%s: Enter", __func__);
    if (device == NULL || device->property == NULL) {
        HDF_LOGE("%s: device or property is null!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    hwdt = (struct Stm32Mp1Iwdg *)OsalMemCalloc(sizeof(struct Stm32Mp1Iwdg));
    if (hwdt == NULL) {
        HDF_LOGE("%s: malloc hwdt fail!", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = Stm32mp1IwdgReadDrs(hwdt, device->property);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read drs fail:%d", __func__, ret);
        OsalMemFree(hwdt);
        return ret;
    }

    hwdt->regBase = OsalIoRemap(hwdt->phyBase, hwdt->regStep);
    if (hwdt->regBase == NULL) {
        HDF_LOGE("%s: ioremap regbase fail!", __func__);
        OsalMemFree(hwdt);
        return HDF_ERR_IO;
    }

    hwdt->wdt.priv = (void *)device->property;
    hwdt->wdt.ops = &g_stm32mp1IwdgMethod;
    hwdt->wdt.device = device;
    ret = WatchdogCntlrAdd(&hwdt->wdt);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: err add watchdog:%d", __func__, ret);
        OsalIoUnmap((void *)hwdt->regBase);
        OsalMemFree(hwdt);
        return ret;
    }
    HDF_LOGI("%s: dev service %s init success!", __func__, HdfDeviceGetServiceName(device));
    return HDF_SUCCESS;
}

static int32_t Stm32mp1IwdgInit(struct HdfDeviceObject *device)
{
    (void)device;
    return HDF_SUCCESS;
}

static void Stm32mp1IwdgRelease(struct HdfDeviceObject *device)
{
    struct WatchdogCntlr *wdt = NULL;
    struct Stm32Mp1Iwdg *hwdt = NULL;

    HDF_LOGI("%s: enter", __func__);
    if (device == NULL) {
        return;
    }

    wdt = WatchdogCntlrFromDevice(device);
    if (wdt == NULL) {
        return;
    }
    WatchdogCntlrRemove(wdt);

    hwdt = (struct Stm32Mp1Iwdg *)wdt;
    if (hwdt->regBase != NULL) {
        OsalIoUnmap((void *)hwdt->regBase);
        hwdt->regBase = NULL;
    }
    OsalMemFree(hwdt);
}

struct HdfDriverEntry g_watchDoDriverEntry = {
    .moduleVersion = 1,
    .Bind = Stm32mp1IwdgBind,
    .Init = Stm32mp1IwdgInit,
    .Release = Stm32mp1IwdgRelease,
    .moduleName = "HDF_PLATFORM_WATCHDOG",
};
HDF_INIT(g_watchDoDriverEntry);
