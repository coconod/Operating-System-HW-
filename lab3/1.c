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
