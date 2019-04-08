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
