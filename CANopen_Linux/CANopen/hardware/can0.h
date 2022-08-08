#ifndef __CAN0_H
#define __CAN0_H

#include "main.h"
#include "data.h"
#include "Master.h"

pthread_t tid_canopen_can;//CANopen接收线程的句柄
void CANopen_CAN_Task(void);//CANopen接收线程

#endif