# 实验一 Thread
### Alarm Clock
##### *Reimplement timer_sleep(), defined in devices/timer.c. Although a working implementation is provided, it "busy waits," that is, it spins in a loop checking the current time and calling thread_yield() until enough time has gone by. Reimplement it to avoid busy waiting.*
##### 重新实现timer_sleep()，它在devices/timer.c中定义。虽然提供了一个工作实现，但是它“繁忙等待”，也就是说，它在一个循环中旋转，检查当前时间并调用thread_yield()，直到足够的时间过去为止。重新实现它，以避免繁忙的等待。
### Priority Scheduling
##### *Implement priority scheduling in Pintos. When a thread is added to the ready list that has a higher priority than the currently running thread, the current thread should immediately yield the processor to the new thread. Similarly, when threads are waiting for a lock, semaphore, or condition variable, the highest priority waiting thread should be awakened first. A thread may raise or lower its own priority at any time, but lowering its priority such that it no longer has the highest priority must cause it to immediately yield the CPU.*
##### 在Pintos中实现优先级调度。当一个线程被添加到就绪列表中，该列表的优先级高于当前正在运行的线程时，当前线程应该立即将处理器交给新线程。类似地，当线程等待锁、信号量或条件变量时，应该首先唤醒优先级最高的等待线程。一个线程可以在任何时候提高或降低它自己的优先级，但是降低它的优先级，使它不再具有最高的优先级，必须使它立即让出CPU。
### Advanced Scheduler
##### *Implement a multilevel feedback queue scheduler similar to the 4.4BSD scheduler to reduce the average response time for running jobs on your system.*
##### 实现一个类似于4.4BSD调度程序的多级反馈队列调度程序，以减少系统上运行作业的平均响应时间。
