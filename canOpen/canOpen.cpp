/*********************main.c******************************/
extern "C"{
#include "canOpen.h"
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
#include "timer.h"
#include "data.h"
#include "Master.h"
#include "timerscfg.h"
#include "canfestival.h"


TIMEVAL last_counter_val = 0;
TIMEVAL elapsed_time = 0;

UNS16 timer_tick = 0; //定时器计数 16位
int sockfd_can; // can的文件描述符



//#include  <thread>
}
using namespace std;

/*void _RxPDO_EventTimers_Handler(CO_Data * d, UNS32 pdoNum){
    printf("nodeid-%x",d->bDeviceNodeId);
}*/
void CAN_RX_Handler(void *arg  );
void CAN0_Init(void);
canRxCb canRxHdl ;

CCanopen:: CCanopen(){
}
CCanopen:: ~CCanopen(){
}
void CCanopen:: 
Release(){
}
static void* CANopen_CAN_Task(void* arg)
{
  CAN0_Init();
  struct pollfd fds;
  fds.fd = sockfd_can; // can
  fds.events = POLLIN;
  fds.revents = 0; //

  canRxHdl = CAN_RX_Handler;
  while (1)
  {
    int ret = poll(&fds, 1, -1); //使用poll进行接收数据 1表示fds里元素个数为1 -1为一直阻塞
    if (ret < 0)
      perror("poll error");

    if (fds.revents & POLLIN) //接受到can数据
    {
    canRxHdl(arg);
    }
  }
}




void* CANopen_Timer_Task(void* arg)
{
    struct timeval t;
    TimeDispatch(); //先执行一遍定时任务
    while (1)
    {
        t.tv_sec = 0;
        t.tv_usec = 10000;               // 10ms。可减少0.1ms，因为下边运行需要时间
        select(0, NULL, NULL, NULL, &t); //每次阻塞10ms，模拟10ms一次中断

        timer_tick++;
        if (timer_tick == 65535)
        {
            elapsed_time = 0;
            TimeDispatch();
        }
    }
}


void CCanopen::
init_canOpen(void)
{
	 pthread_create(&m_tid_can, NULL, CANopen_CAN_Task, (void*)this);     //创建canopen接收线程
    pthread_create(&m_tid__timer, NULL, CANopen_Timer_Task, NULL); //创建canopen定时器线程

    sleep(1);
    unsigned char nodeID = 0x00; //节点ID
    setNodeId(&Master_Data, nodeID);
    setState(&Master_Data, Initialisation); //节点初始化
    setState(&Master_Data, Operational);

}




void CAN0_Init(void)
{
  /**************在开始下边之前，需要保证板子的CAN已经配置好*********************/
  sockfd_can = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  printf("sockfd_can\r\n");

  if (sockfd_can < 0)
    perror("socket error");

  /******************将套接字与 CAN 设备进行绑定********************************/
  struct ifreq ifr = {0};
  struct sockaddr_can can_addr = {};

  //指定can0设备
  strcpy(ifr.ifr_name, "can0");
  ioctl(sockfd_can, SIOCGIFINDEX, &ifr);

  can_addr.can_family = AF_CAN; //填充数据
  can_addr.can_ifindex = ifr.ifr_ifindex;

  int ret = bind(sockfd_can, (struct sockaddr *)&can_addr, sizeof(can_addr)); //绑定
  if (ret < 0)
    perror("bind error");
  /**************************关闭回环功能*************************************/
  int loopback = 0; // 0:close 1:open
  setsockopt(sockfd_can, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));
  //在本地回环功能开启的情况下，所有的发送帧都会被回环到与 CAN 总线接口对应的套接字上
}

void CAN_RX_Handler(void* arg)
{
  struct can_frame RxMessage;
  int ret = read(sockfd_can, &RxMessage, sizeof(struct can_frame)); //这里也有阻塞效果

  if (RxMessage.can_id & CAN_SFF_MASK) //只处理标准帧
  {
    Message rxm = {0};
    rxm.cob_id = RxMessage.can_id & CAN_SFF_MASK;
    rxm.len = RxMessage.can_dlc;
    for (int i = 0; i < rxm.len; i++)
      rxm.data[i] = RxMessage.data[i];
    ((CCanopen*)arg)->CanOnRecv(rxm);
   
    for (int i = 0; i < rxm.len; i++){
        printf("%x--",rxm.data[i]);
    }
    printf("\n\t");
    /*****canopen的数据接收处理部分*******/
    canDispatch(&Master_Data, &rxm); // CANopen自身的处理函数
  }
}


// The driver send a CAN message passed from the CANopen stack CAN_PORT dont use
unsigned char canSend(CAN_PORT port, Message *m)
{
  int i;
  struct can_frame TxMessage = {0};
  TxMessage.can_id = m->cob_id; //默认标准帧。如为扩展帧则can_id = CAN_EFF_FLAG | id
  TxMessage.can_dlc = m->len;

  for (i = 0; i < m->len; i++)
    TxMessage.data[i] = m->data[i];

  int ret = write(sockfd_can, &TxMessage, sizeof(TxMessage)); // write frame
  if (ret != sizeof(TxMessage))
    return 0; // error
  else
    return 1; // success
}







UNS16 get_Tick(void)
{
    return timer_tick;
}
void set_Tick(UNS16 num)
{
    timer_tick = num;
}

// 设置下一次定时任务
void setTimer(TIMEVAL value)
{
    UNS32 timer = get_Tick(); // 获取定时器时间
    elapsed_time += timer - last_counter_val;
    last_counter_val = 65535 - value;
    set_Tick(65535 - value);
    // printf("setTimer %lu, elapsed %lu\r\n", value, elapsed_time);
}

// Return the elapsed time to tell the Stack how much time is spent since last call.
TIMEVAL getElapsedTime(void)
{
    UNS32 timer = get_Tick(); // Copy the value of the running timer
    if (timer < last_counter_val)
        timer += 65535;
    TIMEVAL elapsed = timer - last_counter_val + elapsed_time;
    // printf("all elapsed:%lu-->timer:%lu last count:%lu this elapsed:%lu\r\n",
    //        elapsed, timer, last_counter_val, elapsed_time);
    return elapsed;
}

