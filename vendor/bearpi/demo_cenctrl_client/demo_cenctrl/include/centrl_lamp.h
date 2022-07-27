
#include "socket_client_1.h"

#ifndef _CENTEL_LAMP_H_
#define _CENTEL_LAMP_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef void (*funLightCallback)(char *devName, int status);
    
// the callback params
typedef struct _lampCallBackParam {
    funLightCallback On;            // light on
    funLightCallback Off;           // light off
}lampCallBackParam;

void RegisterLampCallback(lampCallBackParam* callParam);
void UnRegisterLampCallback();

void StartLam(void);

#ifdef __cplusplus
}
#endif

#endif /* __CENTEL_LAMP_H */