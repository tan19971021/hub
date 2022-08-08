#ifndef _CANOPEN_USER_H
#define _CANOPEN_USER_H
extern "C"{
#include <pthread.h>
#include <linux/can.h>
#include "can.h"
typedef  struct can_frame* pCAN_FRAME;
//#define DEBUG_ERR_CONSOLE_ON
//#define DEBUG_WAR_CONSOLE_ON
typedef void (*CANTASK)(int n);
typedef void (*CANTIMERTASK)(int n);
typedef void (*canRxCb)(void*);
 
class CCanopen
{
  public:
    CCanopen();
    ~CCanopen();
    void Release();
    virtual int CanOnRecv(Message CF){return 0;};
    void init_canOpen(void);


  private:
    pthread_t m_tid__timer;
    pthread_t m_tid_can;//CANopen接收线程的句柄
};

}
#endif
