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
