#ifndef __CAN0_H
#define __CAN0_H
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <poll.h>
#include "data.h"
#include "Master.h"

pthread_t tid_canopen_can;//CANopen接收线程的句柄
void* CANopen_CAN_Task(void* arg);//CANopen接收线程

#endif
