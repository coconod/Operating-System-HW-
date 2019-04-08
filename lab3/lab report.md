# 实验三 同步与通信

### 1. 通过fork的方式，产生4个进程P1,P2,P3,P4，每个进程打印输出自己的名字，例如P1输出“I am the process P1”。要求P1最先执行，P2、P3互斥执行，P4最后执行。通过多次测试验证实现是否正确。

##### 1.1 实验源码

`1.c`

```c
#include<stdio.h>
#include<semaphore.h>
#include<fcntl.h>
#include<unistd.h>

#include<stdlib.h>
#include<pthread.h>

void main()
{
	sem_t *p1_signal,*p2_signal,*p3_signal;
	//主函数中的进程是p1
	pid_t p2,p3,p4; 
	p1_signal=sem_open("p1_signal",O_CREAT,0666,0);
	p2_signal=sem_open("p2_signal",O_CREAT,0666,0);
	p3_signal=sem_open("p3_signal",O_CREAT,0666,0);
	p2=fork();
        if(p2<0){printf("进程p2创建错误！\n");}
	if(p2>0){
		printf("I am the process P1\n");
		sem_post(p1_signal);
		sleep(1);
	}
	if(p2==0){
		p3=fork();
		if(p3<0){printf("进程p3创建错误！\n");}
		if(p3>0){
			sleep(1);
			sem_wait(p1_signal);
			printf("I am the process P2\n");
			sem_post(p1_signal);
			sem_post(p2_signal);
		}
		if(p3==0){
			p4=fork();
			if(p4<0){printf("进程p4创建错误！\n");}
			if(p4>0){
				sem_wait(p1_signal);
				printf("I am the process P3\n");
				sem_post(p3_signal);
				sem_post(p1_signal);
				sleep(1);
			}
			if(p4==0){
				sem_wait(p2_signal);
				sem_wait(p3_signal);
				printf("I am the process P4\n");
			}
		}
}
sem_close(p1_signal);
sem_close(p2_signal);
sem_close(p3_signal);
sem_unlink("p1_signal");
sem_unlink("p2_signal");
sem_unlink("p3_signal");
return;
}


```



##### 1.2 流程示意图

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/4_1%E6%B5%81%E7%A8%8B%E5%9B%BE.png)

四个进程必须是P1最先执行，P2、P3在P1执行完后互斥执行，P4最后执行。所以设置了三个信号量，p1执行完成后p1_signal为１，此时p2,p3可以进行，当其任意一个执行后，分别给postp2_signal或p3_signal加一，并重新给p1_signal减一，以让另一个进程能够进入，当p2,p3全部执行完后，p4由sem_wait(p2_signal)sem_wait(p3_signal）控制将最晚执行．

##### 1.3 实验截图

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/4_1.png)

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/4_1(2).png)

### 2.火车票余票数ticketCount初始值为1000，有一个售票线程，一个退票线程，各循环执行多次。添加同步机制，使得结果始终正确。要求多次测试添加同步机制前后的实验效果。
##### 2.1.1 未同步的实验源码

`2.c`

```c
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>

int sales=110;
int returns=20;
int ticketCount=1000;
int i=0,j=0;

void* sale(){
	int temp;
	while(i<sales){
		if(i%10==0){printf("售出%d张票，退回%d张票，剩余%d张票\n",i,j,ticketCount);}
		i++;
		temp=ticketCount;
		pthread_yield();
		temp=temp-1;
		pthread_yield();
		ticketCount=temp;		
	}
}

void* refund(){
	int temp;
	while(j<returns){
		if(j%10==0){printf("售出%d张票，退回%d张票，剩余%d张票\n",i,j,ticketCount);}
		j++;
		temp=ticketCount;
		pthread_yield();
		temp=temp+1;
		pthread_yield();
		ticketCount=temp;
	}
}

int main()
{
	printf("初始票数为：%d\n",ticketCount);
	printf("原有1000张票，售票110张，退票20张\n");
	pthread_t p1,p2;
	pthread_create(&p1,NULL,sale,NULL);
	pthread_create(&p2,NULL,refund,NULL);
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	printf("最终票数为：%d\n",ticketCount);
	return 0;
}
```

##### 2.1.2 说明

由于没有利用信号量进行控制，售票退票进程执行过程中会因为混乱而在读取修改ticketCount时出现错误，而导致最终票数与正确值偏差较大。



##### 2.1.3 运行截图

第一次运行：

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E6%9C%AA%E6%B7%BB%E5%8A%A0%E5%90%8C%E6%AD%A51.png)

第二次运行：

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E6%9C%AA%E6%B7%BB%E5%8A%A0%E5%90%8C%E6%AD%A52.png)

##### 2.2.1 添加同步的源代码

`2_1.c`

```c
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>
int sales=110;
int returns=20;
int ticketCount=1000;
int i=0,j=0;
sem_t *mutex;
void* sale(){
	int temp;
	while(i<sales){
		sem_wait(mutex);
		if(i%10==0){printf("售出%d张票，退回%d张票，剩余%d张票\n",i,j,ticketCount);}
		i++;
		temp=ticketCount;
		pthread_yield();
		temp=temp-1;
		pthread_yield();
		ticketCount=temp;
		sem_post(mutex);		
	}
}

void* refund(){
	int temp;
	while(j<returns){
		sem_wait(mutex);
		if(j%10==0){printf("售出%d张票，退回%d张票，剩余%d张票\n",i,j,ticketCount);}
		j++;
		temp=ticketCount;
		pthread_yield();
		temp=temp+1;
		pthread_yield();
		ticketCount=temp;
		sem_post(mutex);
	}
}

int main()
{
	mutex=sem_open("mutex",O_CREAT,0666,1);
	printf("初始票数为：%d\n",ticketCount);
	printf("原有1000张票，售票110张，退票20张\n");
	pthread_t p1,p2;
	pthread_create(&p1,NULL,sale,NULL);
	pthread_create(&p2,NULL,refund,NULL);
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	printf("最终票数为：%d\n",ticketCount);
	sem_close(mutex);
	sem_unlink("mutex");
	return 0;
}
```


##### 2.2.2 说明

添加信号量作为同步机制后，售票和退票操作都是原子操作，不再会由于操作混乱而令ticketCount出错，在此代码下，输出结果正确。

##### 2.2.3实验截图

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E6%B7%BB%E5%8A%A0%E5%90%8C%E6%AD%A5.png)


### 3.一个生产者一个消费者线程同步。设置一个线程共享的缓冲区， char buf[10]。一个线程不断从键盘输入字符到buf,一个线程不断的把buf的内容输出到显示器。要求输出的和输入的字符和顺序完全一致。（在输出线程中，每次输出睡眠一秒钟，然后以不同的速度输入测试输出是否正确）。要求多次测试添加同步机制前后的实验效果。)
##### 3.1 实验源码

`3.c`

```c
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<fcntl.h>

char buf[10];
sem_t *empty1,*full;
void* input(void *arg){
	int i=0;
	while(1){
		sem_wait(empty1);
		scanf("%c",&buf[i]);
		i=(i+1)%10;	
		sem_post(full);	
		sleep(1);
	}
}

void* output(void *arg){
	int i=0;
       while(1){
		sem_wait(full);		
		printf("输出：%c\n",buf[i]);
		i=(i+1)%10;
		sem_post(empty1);
		sleep(1);
	}
}

int main(int argc,char *argv[]){
	empty1=sem_open("empty",O_CREAT,0666,10);
	full=sem_open("full",O_CREAT,0666,0);
	pthread_t p1,p2;
	pthread_create(&p1,NULL,input,NULL);
	pthread_create(&p2,NULL,output,NULL);
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	sem_close(empty1);
	sem_close(full);
	sem_unlink("empty");
	sem_unlink("full");
	return 0;
}
```



##### 3.2 说明

设置empty1和full两个信号量，empty初始设置为10，full初始为0。
每读进一个输入，sem_wait(empty1)和sem_post(full)分别令empty1减1和full加1，代表缓存空间空闲位置减一，被占用位置加一。
同样每输出一个，sem_wait(full)和sem_post(empty1)分别令full减1和empty1加1，代表缓存空间被占用位置减一，空闲位置加一。
以此达到同步的目的。
由于数组一共申请了十个char类型的空间，在输入输出时需要对游标i进行模10操作。

##### 3.3 实验截图

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/3.png)

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/3_2.png)


### 4.进程通信问题。阅读并运行共享内存、管道、消息队列三种机制的代码

##### 参见：

 [https://www.cnblogs.com/Jimmy1988/p/7706980.html](https://wx.qq.com/cgi-bin/mmwebwx-bin/webwxcheckurl?requrl=https%3A%2F%2Fwww.cnblogs.com%2FJimmy1988%2Fp%2F7706980.html&skey=%40crypt_d6a76cd8_008a1b761f9dfe4f032447dd33401b4e&deviceid=e797347475139229&pass_ticket=3gtpntX6AGa%252FtSCzQsozlcfmWDJEcAHHGdCnHyn33PgZhGfwn8er2bTi%252BidbE9id&opcode=2&scene=1&username=@4651237c60997f581d05b32d8ba4ee216f4d2e2b29c20c5ed7dba5283ecc5735)

 [https://www.cnblogs.com/Jimmy1988/p/7699351.html](https://wx.qq.com/cgi-bin/mmwebwx-bin/webwxcheckurl?requrl=https%3A%2F%2Fwww.cnblogs.com%2FJimmy1988%2Fp%2F7699351.html&skey=%40crypt_d6a76cd8_008a1b761f9dfe4f032447dd33401b4e&deviceid=e797347475139229&pass_ticket=3gtpntX6AGa%252FtSCzQsozlcfmWDJEcAHHGdCnHyn33PgZhGfwn8er2bTi%252BidbE9id&opcode=2&scene=1&username=@4651237c60997f581d05b32d8ba4ee216f4d2e2b29c20c5ed7dba5283ecc5735)

 [https://www.cnblogs.com/Jimmy1988/p/7553069.html](https://wx.qq.com/cgi-bin/mmwebwx-bin/webwxcheckurl?requrl=https%3A%2F%2Fwww.cnblogs.com%2FJimmy1988%2Fp%2F7553069.html&skey=%40crypt_d6a76cd8_008a1b761f9dfe4f032447dd33401b4e&deviceid=e797347475139229&pass_ticket=3gtpntX6AGa%252FtSCzQsozlcfmWDJEcAHHGdCnHyn33PgZhGfwn8er2bTi%252BidbE9id&opcode=2&scene=1&username=@4651237c60997f581d05b32d8ba4ee216f4d2e2b29c20c5ed7dba5283ecc5735)  

#### a）通过实验测试，验证共享内存的代码中，receiver能否正确读出sender发送的字符串？如果把其中互斥的代码删除，观察实验结果有何不同？如果在发送和接收进程中打印输出共享内存地址，他们是否相同，为什么？



#### b)有名管道和无名管道通信系统调用是否已经实现了同步机制？通过实验验证，发送者和接收者如何同步的。比如，在什么情况下，发送者会阻塞，什么情况下，接收者会阻塞？



#### c）消息通信系统调用是否已经实现了同步机制？通过实验验证，发送者和接收者如何同步的。比如，在什么情况下，发送者会阻塞，什么情况下，接收者会阻塞？

