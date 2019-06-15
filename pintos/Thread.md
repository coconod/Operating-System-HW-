# 实验一 Thread
### Alarm Clock
##### *Reimplement timer_sleep(), defined in devices/timer.c. Although a working implementation is provided, it "busy waits," that is, it spins in a loop checking the current time and calling thread_yield() until enough time has gone by. Reimplement it to avoid busy waiting.*
##### 重新实现timer_sleep()，它在devices/timer.c中定义。虽然提供了一个工作实现，但是它“繁忙等待”，也就是说，它在一个循环中，检查当前时间并调用thread_yield()，直到足够的时间过去为止。重新实现它，以避免繁忙的等待。
#### 思路
##### **目标：**重新实现timer_sleep()
##### 解决“busy waits“,避免sleep函数ticks时间内， 如果线程处于running状态就不断把他扔到就绪队列不让他执行而导致不断占用cpu的的问题。
##### timer_sleep()要实现的功能,当检测到要求个ticks过去，才唤醒该线程。
#### **主要构思：**
##### 为线程的结构体加上一个int64_t  ticks_blocked的成员，表示要休眠的时间
##### 表征当前线程是否应该被唤醒，当其为0则被唤醒，否则值减去1
##### 在每个线程执行前，先去检查其ticks_blocked的成员变量，当其为0，代表可执行才进去就绪队列，否则处于阻塞状态
##### 即对于Function: void thread_foreach (thread_action_func \*action, void \*aux)在调用激活函数前增加一个check的函数。


### Priority Scheduling
##### *Implement priority scheduling in Pintos. When a thread is added to the ready list that has a higher priority than the currently running thread, the current thread should immediately yield the processor to the new thread. Similarly, when threads are waiting for a lock, semaphore, or condition variable, the highest priority waiting thread should be awakened first. A thread may raise or lower its own priority at any time, but lowering its priority such that it no longer has the highest priority must cause it to immediately yield the CPU.*
##### 在Pintos中实现优先级调度。当一个线程被添加到就绪列表中，该列表的优先级高于当前正在运行的线程时，当前线程应该立即将处理器交给新线程。类似地，当线程等待锁、信号量或条件变量时，应该首先唤醒优先级最高的等待线程。一个线程可以在任何时候提高或降低它自己的优先级，但是降低它的优先级，使它不再具有最高的优先级，必须使它立即让出CPU。
#### 思路
##### **目标：** 维持就绪队列，确保其为一个优先级队列。也就是在插入线程到就绪队列的时候保证这个队列是一个优先级队列。
##### 线程需要进入就绪队列的情况：
##### 1. thread_unblock
##### 2. init_thread
##### 3. thread_yield
##### 即在函数中，修改将线程加入到就绪队列的函数，将单纯插入队尾的
```c
list_push_back (&ready_list, &t->elem);
```
##### 改为
```c
list_insert_ordered (&ready_list, &t->elem, (list_less_func *) &thread_cmp_priority, NULL);
```
##### 在修改线程优先级时显然也会对优先级队列产生影响
##### 在设置一个线程优先级要立即重新考虑所有线程执行顺序， 重新安排执行顺序。
##### 直接在线程设置优先级的时候调用thread_yield即可， 这样就把当前线程重新丢到就绪队列中继续执行，保证执行顺序。
```c
/* Sets the current thread's priority to NEW_PRIORITY. */
void
thread_set_priority (int new_priority)
{
  thread_current ()->priority = new_priority;
  thread_yield ();
}
```
##### 在创建新线程，也加上 thread_yield ();
```c
 if (thread_current ()->priority < priority)
 {
   thread_yield ();
 }
```

### Advanced Scheduler
##### *Implement a multilevel feedback queue scheduler similar to the 4.4BSD scheduler to reduce the average response time for running jobs on your system.*

##### 实现一个类似于4.4BSD调度程序的多级反馈队列调度程序，以减少系统上运行作业的平均响应时间。
