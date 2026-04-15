#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include "hdf_sbuf.h"
#include "hdf_io_service_if.h"

#define LED_WRITE_READ 1
#define LED_SERVICE "hdf_led"

static int SendEvent(struct HdfIoService *serv, uint8_t eventData)
{
    int ret = 0;
    struct HdfSBuf *data = HdfSBufObtainDefaultSize();
    if (data == NULL)
    {
        printf("fail to obtain sbuf data!\r\n");
        return 1;
    }

    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();
    if (reply == NULL)
    {
        printf("fail to obtain sbuf reply!\r\n");
        ret = HDF_DEV_ERR_NO_MEMORY;
        goto out;
    }
    /* 写入数据 */
    if (!HdfSbufWriteUint8(data, eventData))
    {
        printf("fail to write sbuf!\r\n");
        ret = HDF_FAILURE;
        goto out;
    }
    /* 通过Dispatch发送到驱动 */
    // if(eventData == 3) {
    //     uint8_t tmp = 1;
    //     for(int i = 0; i < 10; ++i) {
    //         HdfSbufFlush(data);
    //         HdfSbufFlush(reply);
    //         HdfSbufWriteUint8(data, tmp);
    //         ret |= serv->dispatcher->Dispatch(&serv->object, LED_WRITE_READ, data, reply);
    //         tmp ^= 1;
    //         sleep(1);
    //     }
    // } else ret = serv->dispatcher->Dispatch(&serv->object, LED_WRITE_READ, data, reply);
    ret = serv->dispatcher->Dispatch(&serv->object, LED_WRITE_READ, data, reply);
    if (ret != HDF_SUCCESS)
    {
        printf("fail to send service call!\r\n");
        goto out;
    }

    int replyData = 0;
    /* 读取驱动的回复数据 */
    if (!HdfSbufReadInt32(reply, &replyData))
    {
        printf("fail to get service call reply!\r\n");
        ret = HDF_ERR_INVALID_OBJECT;
        goto out;
    }
    printf("\r\nGet reply is: %d\r\n", replyData);
out:
    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    return ret;
}

int main(int argc, char **argv)
{
    int i;
    
    /* 获取服务 */
    struct HdfIoService *serv = HdfIoServiceBind(LED_SERVICE);
    if (serv == NULL)
    {
        printf("fail to get service %s!\r\n", LED_SERVICE);
        return HDF_FAILURE;
    }

    for (i=0; i < argc; i++)
    {
        printf("\r\nArgument %d is %s.\r\n", i, argv[i]);
    }

    SendEvent(serv, atoi(argv[1]));

    HdfIoServiceRecycle(serv);
    printf("exit\r\n");

    return HDF_SUCCESS;
}
