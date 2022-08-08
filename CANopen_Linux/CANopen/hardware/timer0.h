#ifndef __TIMER0_H
#define __TIMER0_H

#include "main.h"

#include <sys/time.h>
#include <sys/select.h>
#include <stdio.h>
#include "timerscfg.h"
#include "applicfg.h"
#include <stdbool.h>

pthread_t tid_canopen_timer;  //CANopen定时器线程句柄
void CANopen_Timer_Task(void);//CANopen定时器线程
#endif
