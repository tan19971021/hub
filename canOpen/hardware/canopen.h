#ifndef _CANOPEN_H
#define _CANOPEN_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <termios.h>

/*******CAN驱动**************/
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

/*****CANopen 相关头文件********/
#include "can0.h"
#include "timer0.h"
#include "timer.h"
#include "data.h"
#include "Master.h"
#include "timerscfg.h"
#include "canfestival.h"
//#define DEBUG_ERR_CONSOLE_ON
//#define DEBUG_WAR_CONSOLE_ON
#endif
