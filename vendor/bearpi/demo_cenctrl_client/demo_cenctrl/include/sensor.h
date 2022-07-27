

#ifndef _SENSOR_H_
#define _SENSOR_H_

#ifdef __cplusplus
extern "C"{
#endif

int InitSensor(void);
int ReadSensorData(char* replyData);
int DeiceCtrl(const char* cmd,const char* buf);;



#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_MAIN_H */