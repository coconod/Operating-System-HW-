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
##### 4.1.1　源代码

`Sender.c`

```c
/*
 * Filename: Sender.c
 * Description: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string.h>

int main(int argc, char *argv[])
{
    key_t  key;
    int shm_id;
    int sem_id;
    int value = 0;

    //1.Product the key
    key = ftok(".", 0xFF);

    //2. Creat semaphore for visit the shared memory
    sem_id = semget(key, 1, IPC_CREAT|0644);
    if(-1 == sem_id)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    //3. init the semaphore, sem=0
    if(-1 == (semctl(sem_id, 0, SETVAL, value)))
    {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    //4. Creat the shared memory(1K bytes)
    shm_id = shmget(key, 1024, IPC_CREAT|0644);
    if(-1 == shm_id)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    //5. attach the shm_id to this process
    char *shm_ptr;
    shm_ptr = shmat(shm_id, NULL, 0);
    if(NULL == shm_ptr)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    //6. Operation procedure
    struct sembuf sem_b;
    sem_b.sem_num = 0;      //first sem(index=0)
    sem_b.sem_flg = SEM_UNDO;
    sem_b.sem_op = 1;           //Increase 1,make sem=1
    
    while(1)
    {
        if(0 == (value = semctl(sem_id, 0, GETVAL)))
        {
            printf("\nNow, snd message process running:\n");
            printf("\tInput the snd message:  ");
            scanf("%s", shm_ptr);

            if(-1 == semop(sem_id, &sem_b, 1))
            {
                perror("semop");
                exit(EXIT_FAILURE);
            }
        }

        //if enter "end", then end the process
        if(0 == (strcmp(shm_ptr ,"end")))
        {
            printf("\nExit sender process now!\n");
            break;
        }
    }

    shmdt(shm_ptr);

    return 0;
}
```



`Receiver.c`

```c
/*
 * Filename: Receiver.c
 * Description: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string.h>

int main(int argc, char *argv[])
{
    key_t  key;
    int shm_id;
    int sem_id;
    int value = 0;

    //1.Product the key
    key = ftok(".", 0xFF);

    //2. Creat semaphore for visit the shared memory
    sem_id = semget(key, 1, IPC_CREAT|0644);
    if(-1 == sem_id)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    //3. init the semaphore, sem=0
    if(-1 == (semctl(sem_id, 0, SETVAL, value)))
    {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    //4. Creat the shared memory(1K bytes)
    shm_id = shmget(key, 1024, IPC_CREAT|0644);
    if(-1 == shm_id)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    //5. attach the shm_id to this process
    char *shm_ptr;
    shm_ptr = shmat(shm_id, NULL, 0);
    if(NULL == shm_ptr)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    //6. Operation procedure
    struct sembuf sem_b;
    sem_b.sem_num = 0;      //first sem(index=0)
    sem_b.sem_flg = SEM_UNDO;
    sem_b.sem_op = -1;           //Increase 1,make sem=1
    
    while(1)
    {
        if(1 == (value = semctl(sem_id, 0, GETVAL)))
        {
            printf("\nNow, receive message process running:\n");
            printf("\tThe message is : %s\n", shm_ptr);

            if(-1 == semop(sem_id, &sem_b, 1))
            {
                perror("semop");
                exit(EXIT_FAILURE);
            }
        }

        //if enter "end", then end the process
        if(0 == (strcmp(shm_ptr ,"end")))
        {
            printf("\nExit the receiver process now!\n");
            break;
        }
    }

    shmdt(shm_ptr);
    //7. delete the shared memory
    if(-1 == shmctl(shm_id, IPC_RMID, NULL))
    {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }

    //8. delete the semaphore
    if(-1 == semctl(sem_id, 0, IPC_RMID))
    {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    return 0;
}
```



##### 4.1.2　说明

程序主循环中首先判断表示共享内存访问情况的信号量确定共享内存已经写入消息，可以读取，如果为1的话输出该消息，输出后将信号量减1，通知Sender可以再次写入消息。并且定义一个`end`命令退出当前进程。

去掉信号量后输出不再等待输入不断重复输出。

并且两个进程的共享地址不一样。

##### 4.1.3　运行截图

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/a_1.png)

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/a_2%E5%8E%BB%E6%8E%89%E4%BA%92%E6%96%A5.png)

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/4%E6%89%93%E5%8D%B0%E5%9C%B0%E5%9D%80.png)

#### b)有名管道和无名管道通信系统调用是否已经实现了同步机制？通过实验验证，发送者和接收者如何同步的。比如，在什么情况下，发送者会阻塞，什么情况下，接收者会阻塞？

##### 4.2.1　 无名管道源代码

`pipe.c`

```c
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <assert.h> 
#include <unistd.h> 
int main() { 
	int fd[2]; 
	char massage[100]={0};
	pipe(fd);//fd[0]是读，fd[1]是写 
	if(fork() != 0) { 
		close(fd[0]); 
		while(1){
			printf("parent: ");
			scanf("%s",massage);
			write(fd[1],massage,sizeof(massage)); 
			if(strcmp(massage,"end")==0)
				break;
		}
	} 
	else {
		close(fd[1]);
		while(1){ 
			char buf[100] = {0}; 
			if(read(fd[0],buf,sizeof(buf))>0){
				printf("chlid:%s\n",buf);
			}
			else{printf("nothing in pipe!\n");}
			sleep(1);		
			if(strcmp(buf,"end")==0)
					break;
			memset(buf,0,sizeof(buf));
		} 
	} 
return 0; 
}


```

##### 4.2.2 　说明

通过pipe函数创建管道，函数传递的参数是一个整形数组fd，fd[0]用于读取数据，fd[1]用于写数据。两个描述符相当远管道的两端，一段负责写，一段负责读。

代码中父进程是发送方，子进程是接收方，当父进程没有输入时，子进程就会阻塞，无名管道实现了同步机制。

##### 4.2.3　 运行截图

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E6%97%A0%E5%90%8D%E7%AE%A1%E9%81%93.png)

##### 4.2.4　有名管道源代码

`fifo_wb.c`

```c
/*
 *File: fifo_send.c
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <fcntl.h>


#define FIFO "/tmp/my_fifo"

int main()
{
    char buf[] = "hello,world";

    //`. check the fifo file existed or not
    int ret;
    ret = access(FIFO, F_OK);
    if(ret == 0)    //file /tmp/my_fifo existed
    {
        system("rm -rf /tmp/my_fifo");
    }

    //2. creat a fifo file
    if(-1 == mkfifo(FIFO, 0766))
    {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    //3.Open the fifo file
    int fifo_fd;
    fifo_fd = open(FIFO, O_WRONLY);
    if(-1 == fifo_fd)
    {
        perror("open");
        exit(EXIT_FAILURE);

    }

    //4. write the fifo file
    int num = 0;
    num = write(fifo_fd, buf, sizeof(buf));
    if(num < sizeof(buf))
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    printf("write the message ok!\n");

    close(fifo_fd);

    return 0;
}
```



`fifo_rb.c`

```c
/*
 *File: fifo_rcv.c
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <fcntl.h>


#define FIFO "/tmp/my_fifo"

int main()
{
    char buf[20] ;
    memset(buf, '\0', sizeof(buf));

    //`. check the fifo file existed or not
    int ret;
    ret = access(FIFO, F_OK);
    if(ret != 0)    //file /tmp/my_fifo existed
    {
        fprintf(stderr, "FIFO %s does not existed", FIFO);
        exit(EXIT_FAILURE);
    }

    //2.Open the fifo file
    int fifo_fd;
    fifo_fd = open(FIFO, O_RDONLY);
    if(-1 == fifo_fd)
    {
        perror("open");
        exit(EXIT_FAILURE);

    }

    //4. read the fifo file
    int num = 0;
    num = read(fifo_fd, buf, sizeof(buf));

    printf("Read %d words: %s\n", num, buf);

    close(fifo_fd);

    return 0;
}
```



##### 4.2.5　说明

以读进程为例，通过`fifo_fd=open(FIFO,O_RDONLY | O_NONBLOCK)`设置为非阻塞状态，`fifo_fd=open(FIFO,O_RDONLY)`设置为阻塞状态，对应四个进程分别为fifo_wb(阻塞)、fifo_rb(阻塞)、fifo_w(非阻塞)、fifo_r(非阻塞)


| 写进程  | 结果                            | 读进程  | 结果                       |
| ------- | ------------------------------- | ------- | -------------------------- |
| 阻塞1   | write the message ok!           | 阻塞2   | Read 12 words: hello,world |
| 阻塞2   | 无结果                          | 阻塞1   | 无结果                     |
| 阻塞1   | write the message ok!           | 非阻塞2 | Read -1 words:             |
| 阻塞2   | 无结果                          | 非阻塞1 | Read 0 words:              |
| 非阻塞1 | open: No such device or address | 阻塞2   | 无结果                     |
| 非阻塞2 | open: No such device or address | 阻塞1   | 无结果                     |
| 非阻塞1 | open: No such device or address | 非阻塞2 | Read 0 words:              |
| 非阻塞2 | open: No such device or address | 非阻塞1 | Read 0 words:              |



##### 4.2.6 截图

顺序如表格

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E5%8F%8C%E9%98%BB%E5%A1%9E%E5%85%88%E5%86%99%E5%90%8E%E8%AF%BB.png)

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E5%8F%8C%E9%98%BB%E5%A1%9E%E5%85%88%E8%AF%BB%E5%90%8E%E5%86%99.png)

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E9%98%BB%E5%86%99%E9%9D%9E%E9%98%BB%E8%AF%BB.png)

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E9%9D%9E%E9%98%BB%E8%AF%BB%E9%98%BB%E5%86%99.png)

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E9%9D%9E%E9%98%BB%E5%86%99%E9%98%BB%E8%AF%BB.png)

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E9%98%BB%E8%AF%BB%E9%9D%9E%E9%98%BB%E5%86%99.png)

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E9%9D%9E%E9%98%BB%E5%86%99%E9%9D%9E%E9%98%BB%E8%AF%BB.png)

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E9%9D%9E%E9%98%BB%E8%AF%BB%E9%9D%9E%E9%98%BB%E5%86%99.png)

##### c）消息通信系统调用是否已经实现了同步机制？通过实验验证，发送者和接收者如何同步的。比如，在什么情况下，发送者会阻塞，什么情况下，接收者会阻塞？
##### 4.3.1 源代码

`Client.c`

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <signal.h>

#define BUF_SIZE 128

//Rebuild the strcut (must be)
struct msgbuf
{
    long mtype;
    char mtext[BUF_SIZE];
};


int main(int argc, char *argv[])
{
    //1. creat a mseg queue
    key_t key;
    int msgId;
    
    printf("THe process(%s),pid=%d started~\n", argv[0], getpid());

    key = ftok(".", 0xFF);
    msgId = msgget(key, IPC_CREAT|0644);
    if(-1 == msgId)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    //2. creat a sub process, wait the server message
    pid_t pid;
    if(-1 == (pid = fork()))
    {
        perror("vfork");
        exit(EXIT_FAILURE);
    }

    //In child process
    if(0 == pid)
    {
        while(1)
        {
            alarm(0);
            alarm(100);     //if doesn't receive messge in 100s, timeout & exit
            struct msgbuf rcvBuf;
            memset(&rcvBuf, '\0', sizeof(struct msgbuf));
            msgrcv(msgId, &rcvBuf, BUF_SIZE, 2, 0);                
            printf("Server said: %s\n", rcvBuf.mtext);
        }
        
        exit(EXIT_SUCCESS);
    }

    else    //parent process
    {
        while(1)
        {
            usleep(100);
            struct msgbuf sndBuf;
            memset(&sndBuf, '\0', sizeof(sndBuf));
            char buf[BUF_SIZE] ;
            memset(buf, '\0', sizeof(buf));
            
            printf("\nInput snd mesg: ");
            scanf("%s", buf);
            
            strncpy(sndBuf.mtext, buf, strlen(buf)+1);
            sndBuf.mtype = 1;

            if(-1 == msgsnd(msgId, &sndBuf, strlen(buf)+1, 0))
            {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            
            //if scanf "end~", exit
            if(!strcmp("end~", buf))
                break;
        }
        
        printf("THe process(%s),pid=%d exit~\n", argv[0], getpid());
    }

    return 0;
}
```

`Server.c`

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <signal.h>

#define BUF_SIZE 128

//Rebuild the strcut (must be)
struct msgbuf
{
    long mtype;
    char mtext[BUF_SIZE];
};


int main(int argc, char *argv[])
{
    //1. creat a mseg queue
    key_t key;
    int msgId;
    
    key = ftok(".", 0xFF);
    msgId = msgget(key, IPC_CREAT|0644);
    if(-1 == msgId)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    printf("Process (%s) is started, pid=%d\n", argv[0], getpid());

    while(1)
    {
        alarm(0);
        alarm(600);     //if doesn't receive messge in 600s, timeout & exit
        struct msgbuf rcvBuf;
        memset(&rcvBuf, '\0', sizeof(struct msgbuf));
        msgrcv(msgId, &rcvBuf, BUF_SIZE, 1, 0);                
        printf("Receive msg: %s\n", rcvBuf.mtext);
        
        struct msgbuf sndBuf;
        memset(&sndBuf, '\0', sizeof(sndBuf));

        strncpy((sndBuf.mtext), (rcvBuf.mtext), strlen(rcvBuf.mtext)+1);
        sndBuf.mtype = 2;

        if(-1 == msgsnd(msgId, &sndBuf, strlen(rcvBuf.mtext)+1, 0))
        {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
            
        //if scanf "end~", exit
        if(!strcmp("end~", rcvBuf.mtext))
             break;
    }
        
    printf("THe process(%s),pid=%d exit~\n", argv[0], getpid());

    return 0;
}

```



##### 4.3.2 说明

```c
int msgrcv(int msqid, void  *ptr, size_t  length, long  type, int  flag);
int msgsnd(int msqid, const void *ptr, size_t length, int flag);
```

flag 为0表示阻塞方式，设置IPC_NOWAIT 表示非阻塞方式 

客户端的子进程负责接收消息，父进程主要负责发送消息；

客户端和服务器端都是以阻塞的方式读取和写入消息。

仅当客户端和服务器都阻塞时才能正常读写，其他情况都不能正常运行。



##### 4.3.3 运行截图

双阻塞

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E5%8F%8C%E9%98%BB%E5%A1%9E.png)

客户端阻塞服务器非阻塞

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E9%98%BB%E5%AE%A2%E6%88%B7%E9%9D%9E%E9%98%BB%E6%9C%8D%E5%8A%A1.png)

客户端非阻塞服务器阻塞

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E9%9D%9E%E9%98%BB%E5%AE%A2%E6%88%B7%E9%98%BB%E6%9C%8D%E5%8A%A1.png)

双非阻塞

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/%E5%8F%8C%E9%9D%9E%E9%98%BB%E5%AE%A2%E6%88%B7%E6%9C%8D%E5%8A%A1.png)

### 5.阅读Pintos操作系统，找到并阅读进程上下文切换的代码，说明实现的保存和恢复的上下文内容以及进程切换的工作流程。

 `void timer_sleep`er(int64_t ticks)

```ｃ
/* Sleeps for approximately TICKS timer ticks.  Interrupts must
   be turned on. */
void timer_sleep (int64_t ticks)
{
  int64_t start = timer_ticks ();
  ASSERT (intr_get_level () == INTR_ON);
  while (timer_elapsed (start) < ticks)
    thread_yield();
}
```

  获取ticks的当前值返回，保证这个过程不被中断

![](https://github.com/coconod/Operating-System-HW-/blob/master/lab3/images/timer_sleep.png)



`thread.s`

```assembly
#### struct thread *switch_threads (struct thread *cur, struct thread *next);
####
#### Switches from CUR, which must be the running thread, to NEXT,
#### which must also be running switch_threads(), returning CUR in
#### NEXT's context.
####
#### This function works by assuming that the thread we're switching
#### into is also running switch_threads().  Thus, all it has to do is
#### preserve a few registers on the stack, then switch stacks and
#### restore the registers.  As part of switching stacks we record the
#### current stack pointer in CUR's thread structure.

.globl switch_threads
.func switch_threads
switch_threads:
    # Save caller's register state.
    #
    # Note that the SVR4 ABI allows us to destroy %eax, %ecx, %edx,
    # but requires us to preserve %ebx, %ebp, %esi, %edi.  See
    # [SysV-ABI-386] pages 3-11 and 3-12 for details.
    #
    # This stack frame must match the one set up by thread_create()
    # in size.
    pushl %ebx
    pushl %ebp
    pushl %esi
    pushl %edi

    # Get offsetof (struct thread, stack).
.globl thread_stack_ofs
    mov thread_stack_ofs, %edx

    # Save current stack pointer to old thread's stack, if any.
    movl SWITCH_CUR(%esp), %eax
    movl %esp, (%eax,%edx,1)

    # Restore stack pointer from new thread's stack.
    movl SWITCH_NEXT(%esp), %ecx
    movl (%ecx,%edx,1), %esp

    # Restore caller's register state.
    popl %edi
    popl %esi
    popl %ebp
    popl %ebx
        ret
.endfunc


/* switch_thread()'s stack frame. */
struct switch_threads_frame 
  {
    uint32_t edi;               /*  0: Saved %edi. */
    uint32_t esi;               /*  4: Saved %esi. */
    uint32_t ebp;               /*  8: Saved %ebp. */
    uint32_t ebx;               /* 12: Saved %ebx. */
    void (*eip) (void);         /* 16: Return address. */
    struct thread *cur;         /* 20: switch_threads()'s CUR argument. */
    struct thread *next;        /* 24: switch_threads()'s NEXT argument. */
  };
```



全局变量thread_stack_ofs记录线程和栈之间的关系， 线程切换有个保存现场的过程，先把当前的线程指针放到eax中， 并把线程指针保存在相对基地址偏移量为edx的地址中。切换到下一个线程的线程栈指针， 保存在ecx中， 再把这个线程相对基地址偏移量edx地址（上一次保存现场的时候存放的）放到esp当中继续执行。

这里ecx, eax起容器的作用， edx指向当前现场保存的地址偏移量。就是保存当前线程状态， 恢复新线程之前保存的线程状态。

然后再把4个寄存器拿出来， 这个是硬件设计要求的， 必须保护switch_threads_frame里面的寄存器才可以destroy掉eax, edx, ecx。然后到现在eax(函数返回值是eax)就是被切换的线程棧指针。

由此得到一个结论， schedule先把当前线程放到就绪队列，然后把线程切换如果下一个线程和当前线程不一样的情况下。

###### 总而言之

thread_schedule_tail　获取当前线程， 分配恢复之前执行的状态和现场， 如果当前线程死亡就清空资源。 

schedule　拿下一个线程切换过来继续运行。

thread_yield其实就是把当前线程加入就绪队列里， 然后重新schedule， 注意这里如果ready队列为空的话当前线程会继续在cpu执行。

最后回溯到我们最顶层的函数逻辑： timer_sleep就是在ticks时间内， 如果线程处于running状态就不断把他扔到就绪队列不让他执行。
