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
 
#include "sys_config.h"
#include "target_config.h"
#include "los_typedef.h"
#include <unistd.h>
#include "stdlib.h"
#include "stdio.h"
#include "los_process_pri.h"
#include "disk.h"
#include "sys/mount.h"
#include "los_rootfs.h"

#include "los_config.h"
#include "gic_common.h"
#include "los_printf.h"
#include "los_smp.h"
#include "los_vm_map.h"
#include "los_vm_zone.h"
#include "los_vm_boot.h"
#include "los_mmu_descriptor_v6.h"
#include "los_init.h"

#include "mtd_partition.h"

#ifdef LOSCFG_FS_VFS
#include "disk.h"
#endif
#include "los_rootfs.h"
#ifdef LOSCFG_SHELL
#include "shell.h"
#include "shcmd.h"
#endif

#ifdef LOSCFG_DRIVERS_MEM
#include "los_dev_mem.h"
#endif

#ifdef LOSCFG_DRIVERS_HDF
#include "devmgr_service_start.h"
#endif

#ifdef LOSCFG_DRIVERS_HDF_PLATFORM_WATCHDOG
#include "watchdog_if.h"
#endif

#include "los_task.h"
#include "los_bootargs.h"
#include "los_rootfs.h"

#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"

UINT32 OsRandomStackGuard(VOID)
{
#ifdef LOSCFG_DRIVERS_RANDOM
    UINT32 stackGuard = 0;

    HiRandomHwInit();
    (VOID)HiRandomHwGetInteger(&stackGuard);
    HiRandomHwDeinit();
    return stackGuard;
#else
    return 0;
#endif
}

#ifdef LOSCFG_DRIVERS_MEM
int mem_dev_register(void)
{
    return DevMemRegister();
}
#endif

VOID HalClockIrqClear(VOID) {}

int mount_rootfs(void)
{
    int ret = 0;

    ret = add_mtd_partition("spinor", 0, DDR_RAMFS_REAL_SIZE, 0);
    if (ret) {
        dprintf("add_mtd_partition fail.");
        return ret;
    }

    dprintf("mount /dev/spinorblk0 / ...\n");
    if (mount("/dev/spinorblk0", "/", "jffs2", 0, NULL))
    {
        PRINT_ERR("mount failed.\n");
    }

    return 0;
}

CHAR *OsGetArgsAddr(VOID)
{
#define BOOTARGS        "root=emmc fstype=vfat rootaddr=0x2280000 rootsize=0x2000000 useraddr=0x4280000 usersize=0x3200000"
#define BOOTARGSLEN     (strlen(BOOTARGS))

    static int i = 0;
    static CHAR bootargs[512] = {0};

    if (i == 0) {
        strncpy_s(bootargs, 512, BOOTARGS, BOOTARGSLEN);
        i++;
    }

    return bootargs;
}

#ifdef LOSCFG_DRIVERS_HDF_PLATFORM_WATCHDOG
void feed_dog_task(void)
{
    int32_t ret;
    DevHandle wd = WatchdogOpen(1);
    if (wd == NULL) {
        dprintf("WatchdogOpen fail.\r\n");
        return;
    }
    while(1) {
        ret = WatchdogFeed(wd);
        if (ret != HDF_SUCCESS) {
            dprintf("WatchdogFeed fail.\r\n");
        }
        sleep(10);
    }
}
UINT32 os_feed_dog_task_create(VOID)
{
    UINT32 taskID;
    TSK_INIT_PARAM_S sysTask;
    (VOID)memset_s(&sysTask, sizeof(TSK_INIT_PARAM_S), 0, sizeof(TSK_INIT_PARAM_S));
    sysTask.pfnTaskEntry = (TSK_ENTRY_FUNC)feed_dog_task;
    sysTask.uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    sysTask.pcName = "feed_dog_task";
    sysTask.usTaskPrio = 5;
    // sysTask.uwResved = LOS_TASK_STATUS_DETACHED;
    return LOS_TaskCreate(&taskID, &sysTask);
}
#endif

void net_init(void)
{
extern void tcpip_init(tcpip_init_done_fn initfunc, void *arg);
    dprintf("\ntcpip_init start\n");
    tcpip_init(NULL, NULL);
    dprintf("\ntcpip_init end\n");
    PRINTK("Ethernet start.");
}

void SystemInit(void)
{
#ifdef LOSCFG_DRIVERS_MEM
    dprintf("mem dev init ...\n");
    extern int mem_dev_register(void);
    mem_dev_register();
#endif

    dprintf("Date:%s.\n", __DATE__);
    dprintf("Time:%s.\n", __TIME__);

#ifdef LOSCFG_DRIVERS_HDF
    dprintf("DeviceManagerStart start ...\n");
    if (DeviceManagerStart()) {
        PRINT_ERR("No drivers need load by hdf manager!");
    }
    dprintf("DeviceManagerStart end ...\n");
#endif
    net_init();
    sleep(1);

#ifdef LOSCFG_PLATFORM_ROOTFS
    dprintf("OsMountRootfs start ...\n");
    if (LOS_ParseBootargs()) {
        PRINT_ERR("parse bootargs error!\n");
    }
    if (OsMountRootfs()) {
        PRINT_ERR("mount rootfs error!\n");
    }
    dprintf("OsMountRootfs end ...\n");
#endif

    dprintf("Before PLATFORM_UART ...\n");

#ifdef LOSCFG_DRIVERS_HDF_PLATFORM_UART
    if (virtual_serial_init(TTY_DEVICE) != 0) {
        PRINT_ERR("virtual_serial_init failed");
    }
    if (system_console_init(SERIAL) != 0) {
        PRINT_ERR("system_console_init failed\n");
    }
#endif

    dprintf("After PLATFORM_UART ...\n");

#ifdef LOSCFG_DRIVERS_HDF_PLATFORM_WATCHDOG
    dprintf("OsFeedDogTaskCreate start ...\n");
    os_feed_dog_task_create();
    dprintf("OsFeedDogTaskCreate end ...\n");
#endif

    if (OsUserInitProcess()) {
        PRINT_ERR("Create user init process faialed!\n");
        return;
    }
    dprintf("cat log shell end\n");
    return;
}

#ifdef LOSCFG_KERNEL_MMU
LosArchMmuInitMapping g_archMmuInitMapping[] = {
    {
        .phys = SYS_MEM_BASE,
        .virt = KERNEL_VMM_BASE,
        .size = KERNEL_VMM_SIZE,
        .flags = MMU_DESCRIPTOR_KERNEL_L1_PTE_FLAGS,
        .name = "KernelCached",
    },
    {
        .phys = SYS_MEM_BASE,
        .virt = UNCACHED_VMM_BASE,
        .size = UNCACHED_VMM_SIZE,
        .flags = MMU_INITIAL_MAP_NORMAL_NOCACHE,
        .name = "KernelUncached",
    },
    {
        .phys = PERIPH_PMM_BASE,
        .virt = PERIPH_DEVICE_BASE,
        .size = PERIPH_DEVICE_SIZE,
        .flags = MMU_INITIAL_MAP_DEVICE,
        .name = "PeriphDevice",
    },
    {
        .phys = PERIPH_PMM_BASE,
        .virt = PERIPH_CACHED_BASE,
        .size = PERIPH_CACHED_SIZE,
        .flags = MMU_DESCRIPTOR_KERNEL_L1_PTE_FLAGS,
        .name = "PeriphCached",
    },
    {
        .phys = PERIPH_PMM_BASE,
        .virt = PERIPH_UNCACHED_BASE,
        .size = PERIPH_UNCACHED_SIZE,
        .flags = MMU_INITIAL_MAP_STRONGLY_ORDERED,
        .name = "PeriphStronglyOrdered",
    },
    {
        .phys = GIC_PHY_BASE,
        .virt = GIC_VIRT_BASE,
        .size = GIC_VIRT_SIZE,
        .flags = MMU_INITIAL_MAP_DEVICE,
        .name = "GIC",
    },
    {
        .phys = DDR_RAMFS_ADDR,
        .virt = DDR_RAMFS_VBASE,
        .size = DDR_RAMFS_SIZE,
        .flags = MMU_INITIAL_MAP_DEVICE,
        .name = "Sbull",
    },
    {
        .phys = FB_PHY_BASE,
        .virt = FB_VIRT_BASE,
        .size = FB_SIZE,
        .flags = MMU_INITIAL_MAP_DEVICE,
        .name = "FB",
    },
    {0}
};

struct ArchMmuInitMapping *OsGetMmuPartition(const char *name)
{
    UINT32 index = 0;
    UINT32 mmuBlockCount = sizeof(g_archMmuInitMapping) / sizeof(g_archMmuInitMapping[0]);

    for(index = 0; index < (mmuBlockCount - 1); index++)
    {
        if(strncmp(name, g_archMmuInitMapping[index].name, 20) == 0)
        {
            return &g_archMmuInitMapping[index];
        }
    }
    return NULL;
}

#endif
