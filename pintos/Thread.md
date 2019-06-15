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
##### 即对于Function: void thread_foreach (thread_action_func \*action, void \*aux)在调用激活函数前增加一个check的函数。e.g.
```c
 thread_foreach (blocked_thread_check, NULL);
```

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
##### 在一个线程获取一个锁的时候， 如果拥有这个锁的线程优先级比自己低就提高它的优先级，然后在这个线程释放掉这个锁之后把原来拥有这个锁的线程改回原来的优先级。 释放一个锁的时候， 将该锁的拥有者改为该线程被捐赠的第二优先级，若没有其余捐赠者， 则恢复原始优先级。
##### 因此必然需要一个数据结构来记录所有对这个线程有捐赠行为的线程，以及需要获取这个线程被锁于哪个线程。 
#### 总体要求
##### 1.  在一个线程获取一个锁的时候， 如果拥有这个锁的线程优先级比自己低就提高它的优先级，并且如果这个锁还被别的锁锁着， 将会递归地捐赠优先级， 然后在这个线程释放掉这个锁之后恢复未捐赠逻辑下的优先级。

##### 2. 如果一个线程被多个线程捐赠， 维持当前优先级为捐赠优先级中的最大值（acquire和release之时）。

##### 3. 在对一个线程进行优先级设置的时候， 如果这个线程处于被捐赠状态， 则对original_priority进行设置， 然后如果设置的优先级大于当前优先级， 则改变当前优先级， 否则在捐赠状态取消的时候恢复original_priority。

##### 4. 在释放锁对一个锁优先级有改变的时候应考虑其余被捐赠优先级和当前优先级。

##### 5. 将信号量的等待队列实现为优先级队列。

##### 6. 将condition的waiters队列实现为优先级队列。

##### 7. 释放锁的时候若优先级改变则可以发生抢占。

##### thread数据结构， 加入以下成员：
```c
 int base_priority;                  /* Base priority. */
 struct list locks;                  /* Locks that the thread is holding. */
 struct lock *lock_waiting;          /* The lock that the thread is waiting for. */
```
##### 然后给lock加入以下成员：
```c
 struct list_elem elem;      /* List element for priority donation. */
 int max_priority;          /* Max priority among the threads acquiring the lock. */
```
### Advanced Scheduler
##### *Implement a multilevel feedback queue scheduler similar to the 4.4BSD scheduler to reduce the average response time for running jobs on your system.*

##### 实现一个类似于4.4BSD调度程序的多级反馈队列调度程序，以减少系统上运行作业的平均响应时间。
##### 在timer_interrupt中固定一段时间计算更新线程的优先级，这里是每TIMER_FREQ时间更新一次系统load_avg和所有线程的recent_cpu， 每4个timer_ticks更新一次线程优先级， 每个timer_tick running线程的recent_cpu加一， 虽然这里说的是维持64个优先级队列调度， 其本质还是优先级调度， 我们保留之前写的优先级调度代码即可， 去掉优先级捐赠（之前donate相关代码已经对需要的地方加了thread_mlfqs的判断了）。
##### 浮点运算逻辑实现在fixed_point.h
