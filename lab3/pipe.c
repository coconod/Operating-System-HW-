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


