#include "timer0.h"
pthread_t tid_canopen_timer;

TIMEVAL last_counter_val = 0;
TIMEVAL elapsed_time = 0;

UNS16 timer_tick = 0; //定时器计数 16位

void CANopen_Timer_Task(void)
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
