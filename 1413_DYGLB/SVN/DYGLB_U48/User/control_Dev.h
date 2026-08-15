#ifndef CONTROL_DEV_H
#define CONTROL_DEV_H

#define POWER_ON            1
#define POWER_OFF           0

void control_Gpio_Init(void);

void operation_dev(void) ;
void stuAndwarning_Get(void) ;

void limitCurrent_Config(void) ;

void Init_limitCurrent_Config(void) ;


void default_Power_Config(void);

void selfRecoveTime(uint32_t overTime) ;

#endif

