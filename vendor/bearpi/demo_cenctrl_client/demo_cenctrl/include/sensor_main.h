
#include "socket_client_1.h"

#ifndef _SENSOR_MAIN_H_
#define _SENSOR_MAINP_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef int (*displayfunCallback) (int temp, int lux, int hum);
    
// the callback params
typedef struct _displayCallBackParam {
    displayfunCallback SensorDisplay;            // Display
}displayCallBackParam;

void RegisterSensorDisplsyCallback(displayCallBackParam* callParam);
void UnRegisterSensorDisplsyCallback();

void StartSensor(void);

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_MAIN_H */