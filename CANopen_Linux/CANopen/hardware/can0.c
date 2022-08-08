// Includes for the Canfestival driver
#include "can0.h"
/**********************************can0.c*************************************/

pthread_t tid_canopen_can;//CANopen接收线程的句柄
void CAN_RX_Handler(void);//CAN数据接收的回调函数

int sockfd_can; // can的文件描述符

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

void CANopen_CAN_Task(void)
{
  CAN0_Init();
  struct pollfd fds;
  fds.fd = sockfd_can; // can
  fds.events = POLLIN;
  fds.revents = 0; //
  while (1)
  {
    int ret = poll(&fds, 1, -1); //使用poll进行接收数据 1表示fds里元素个数为1 -1为一直阻塞
    if (ret < 0)
      perror("poll error");

    if (fds.revents & POLLIN) //接受到can数据
    {
      CAN_RX_Handler();
    }
  }
}

void CAN_RX_Handler(void)
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
    for (int i = 0; i < rxm.len; i++)
      printf("%x--",rxm.data[i] );
    printf("%x\n\t",rxm.cob_id);
   
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

