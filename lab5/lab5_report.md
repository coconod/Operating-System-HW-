# 实验五 文件系统
## 1 实验简介
##### &#8195;&#8195;本实验要求在模拟的I/O系统之上开发一个简单的文件系统。用户通过create, open,read等命令与文件系统交互。文件系统把磁盘视为顺序编号的逻辑块序列，逻辑块的编号为0至L −1。I/O系统利用内存中的数组模拟磁盘。
##### &#8195;&#8195;宏定义数据
```c
#define C 6//柱面号
#define H 10//磁道号
#define B 12//扇区号
#define BLOCKSIZE	10			//存储块长度
#define L			720			//存储块总数
#define K			100			//保留区大小
#define BUSY		1
#define FREE		0
#define OK			1
#define ERROR		0
#define FILE_BLOCK_LENGTH		(BLOCKSIZE-3)				//文件分配磁盘块号数组长度
#define FILE_NAME_LENGTH		(BLOCKSIZE-1)				//最长文件名长度
#define FILE_descriptor_AREA			((L-1-K)/BLOCKSIZE+1)		//保留区中文件标识符起e始块号(位图之后)
#define FILE_NUM				FILE_BLOCK_LENGTH	//目录内最多文件数目
#define BUFFER_LENGTH			25					//打开文件表目中的缓冲区长度
#define INPUT_LENGTH			100					//写文件时最大输入长度
#define OUTPUT_LENGTH			100					//读文件时最大读出长度

```
## 2 I/O系统
##### &#8195;&#8195;实际物理磁盘的结构是多维的：有柱面、磁头、扇区等概念。I/O系统的任务是隐藏磁盘的结构细节，把磁盘以逻辑块的面目呈现给文件系统。逻辑块顺序编号，编号取值范围为0至L−1，其中L表示磁盘的存储块总数。实验中，我们可以利用数组ldisk[C][H][B]构建磁盘模型，其中CHB分别表示柱面号，磁头号和扇区号。每个扇区大小为512字节。I/O系统从文件系统接收命令，根据命令指定的逻辑块号把磁盘块的内容读入命令指定的内存区域，或者把命令指定的内存区域内容写入磁盘块。文件系统和I/O系统之间的接口由如下两个函数定义：
##### &#8195;&#8195;•read_block(inti, char *p);
##### &#8195;&#8195;&#8195;&#8195;该函数把逻辑块i的内容读入到指针p指向的内存位置，拷贝的字符个数为存储块的长度B。
```c
void read_block(int i, char *p)
/**************************
.读磁盘块
.该函数把逻辑块i的内容读入到指针p 指向的内存位置
.拷贝的字符个数为存储块的长度BLOCKSIZE。
***************************/
{
	char * temp = (char *)malloc(sizeof(char));
	temp = p;
	int x = i / (H*B);
	int y = (i - x * (H*B)) / B;
	int z = i - x * (H*B) - y * B;
	for (int a = 0; a < BLOCKSIZE;)
	{
		*temp = ldisk[x][y][z][a];
		a++;
		temp++;
	}
}
```
##### &#8195;&#8195;•write block(inti, char *p);
##### &#8195;&#8195;&#8195;&#8195;该函数把指针p指向的内容写入逻辑块i，拷贝的字符个数为存储块的长度B。此外，为了方便测试，我们还需要实现另外两个函数：一个用来把数组ldisk存储到文件；另一个用来把文件内容恢复到数组。


## 3 文件系统
##### &#8195;&#8195;文件系统位于I/O系统之上。
### 3.1 用户与文件系统之间的接口
##### &#8195;&#8195;文件系统需提供如下函数；create, destroy, open, read, write。

##### &#8195;&#8195;&#8195;&#8195;•lseek(index, pos): 把文件的读写指针移动到pos指定的位置。pos是一个整数，表示从文件开始位置的偏移量。文件打开时，读写指针自动设置为0。每次读写操作之后，它指向最后被访问的字节的下一个位置。lseek能够在不进行读写操作的情况下改变读写指针能位置。
```c
int lseek(int index, int pos)
/***************
把文件的读写指针移动到pos 指定的位置。pos
是一个整数，表示从文件开始位置的偏移量。文件打开时，读写指针
自动设置为0。每次读写操作之后，它指向最后被访问的字节的下一
个位置。lseek 能够在不进行读写操作的情况下改变读写指针能位置。
****************/
{
	int i;
	int list = -1;
	char temp[BLOCKSIZE];
	int pos_i = pos / BLOCKSIZE;				//在文件所有块数中的第X块
	int pos_j = pos % BLOCKSIZE;				//在第X块中的X个位置
	//***************根据index找表目
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].file_descriptor_num == index)
		{
			list = i;
			break;
		}
	}

	if (list == -1)					//没找到
	{
		printf("没找到当前索引号文件,操作失败...\n");
		return ERROR;
	}
	if (open_list[list].flag != BUSY)		//输入的index对应文件没被打开
	{
		printf("输入的索引号有误，操作失败...\n");
		return ERROR;
	}
	//**************根据表目中的index即文件描述符序找到具体内容
	read_block(open_list[list].file_descriptor_num + FILE_descriptor_AREA, temp);
	if (pos_i > temp[2] - 1)											//大于文件实际的块数时
	{
		printf("异常越界,定位失败\n");
		return ERROR;
	}

	//****************定位指针

	open_list[list].pointer[0] = temp[3 + pos_i];				//该文件的第X块的实际磁盘地址
	open_list[list].pointer[1] = pos_j;


	return OK;
	//*****************************

}


```
##### &#8195;&#8195;&#8195;&#8195;•directory: 列表显示所有文件及其长度。
```c
void directory()
//列表显示所有文件及其长度。
{
	int i;
	int filenum;
	int filelength;
	char filename[FILE_NAME_LENGTH];
	char temp[BLOCKSIZE];
	char tempd[BLOCKSIZE];
	char temps[BLOCKSIZE];
	read_block(FILE_descriptor_AREA, temp);
	filenum = temp[1];						//实际存在的文件个数
	printf("\n");
	if (filenum == 0)
	{
		printf("\t\t\t\t该目录下没有文件\n");
	}

	for (i = 0; i < FILE_NUM; i++)
	{
		read_block(temp[3 + i], tempd);					//读取目录项
		if (tempd[0] != 0)
		{
			read_block(tempd[0] + FILE_descriptor_AREA, temps);		//读取文件描述符
			if (temps[0] == BUSY && tempd[0] != 0)
			{
				filelength = temps[1];
				strcpy(filename, tempd + 1);
				printf("%-10s\t\t%-2d字节\n", filename, filelength);
			}
		}
	}

	if (filenum != 0)
	{
		printf("\t\t\t\t共%d个文件\n", filenum);
	}
}
```

### 3.2 文件系统的组织
##### &#8195;&#8195;磁盘的前k个块是保留区，其中包含如下信息：位图和文件描述符。位图用来描述磁盘块的分配情况。位图中的每一位对应一个逻辑块。创建或者删除文件，以及文件的长度发生变化时，文件系统都需要进行位图操作。前k个块的剩余部分包含一组文件描述符。
##### &#8195;&#8195;位图为0（free）时代表对应块没有被占用，位图为1（busy）代表对应块存有数据，在检索时有很大的作用。
##### &#8195;&#8195;每个文件描述符包含如下信息：
##### &#8195;&#8195;•文件长度，单位字节
##### &#8195;&#8195;•文件分配到的磁盘块号数组。该数组的长度是一个系统参数。在实验中我们可以把它设置为一个比较小的数，例如7。
```c
struct file_descriptor {					//文件描述符
	int file_length;					//文件长度
	int file_descriptor_flag;				//占用标识位
	int file_block;						//文件分配磁盘块号数组实际长度
	int file_block_ary[FILE_BLOCK_LENGTH];			//文件分配磁盘块号数组
};

```
### 3.3 目录
##### &#8195;&#8195;我们的文件系统中仅设置一个目录，该目录包含文件系统中的所有文件。除了不需要显示地创建和删除之外，目录在很多方面和普通文件相像。目录对应0号文件描述符。初始状态下，目录中没有文件，所有，目录对应的描述符中记录的长度应为0，而且也没有分配磁盘块。每创建一个文件，目录文件的长度便增加一分。目录文件的内容由一系列的目录项组成，其中每个目录项由如下内容组成：
##### &#8195;&#8195;•文件名
##### &#8195;&#8195;•文件描述符序号
```c
struct contents {					//目录项
	char filename[FILE_NAME_LENGTH];		//文件名
	int file_descriptor_num;			//文件描述符序号
};

```
### 3.4 文件的创建与删除
##### &#8195;&#8195;创建文件时需要进行如下操作；
##### &#8195;&#8195;&#8195;&#8195;•找一个空闲文件描述符(扫描ldisk [0]～ldisk [k - 1])
##### &#8195;&#8195;&#8195;&#8195;•在文件目录里为新创建的文件分配一个目录项（可能需要为目录文件分配新的磁盘块）
##### &#8195;&#8195;&#8195;&#8195;•在分配到的目录项里记录文件名及描述符编号．
##### &#8195;&#8195;&#8195;&#8195;•返回状态信息（如有无错误发生等）
##### &#8195;&#8195;&#8195;&#8195;•create(filename): 根据指定的文件名创建新文件。
```c
int create(const char filename[])
/**************************
.根据文件名创建文件。
.找一空闲文件描述符
.在文件目录里为新创建的文件分配一个目录项，（可能需要为目录文件分配新的磁盘块）
.在分配到的目录项里记录文件名以及描述符编号
.返回状态信息
***************************/
{
	int i;
	int frees;							//空闲的文件描述符位置	
	int freed;							//空闲数据区位置（目录项）
	int freed2;
	char temps[BLOCKSIZE];
	char tempc[BLOCKSIZE];
	char temp[BLOCKSIZE];
	//***************查看文件名是否存在
	for (i = K; i < K + FILE_NUM; i++)
	{
		read_block((i - K) / BLOCKSIZE, temp);//读位图看是否有文件
		//read_block(K, temp);
		if (temp[(i - K) % BLOCKSIZE] == BUSY)
		{
			read_block(i, temp);
			if (strncmp(temp + 1, filename, FILE_NAME_LENGTH) == 0)
			{
				printf("该目录已经-存在文件名为%s的文件。\n", filename);
				return ERROR;
			}
		}
	}
	//***************寻找保留区中空闲的文件描述符
	for (i = FILE_descriptor_AREA; i < K; i++)
	{
		read_block(i, temp);
		if (temp[0] == FREE)
		{
			frees = i;
			break;
		}
	}
	if (i == K)
	{
		printf("没有空闲的文件描述符\n");
		return ERROR;
	}
	//****************寻找数据区目录描述符所指定的中空闲的存储块
	for (i = K; i < K + FILE_NUM; i++)
	{
		read_block((i - K) / BLOCKSIZE, temp);
		if (temp[(i - K) % BLOCKSIZE] == FREE)
		{
			freed = i;
			break;
		}
	}
	if (i == K + FILE_NUM)
	{
		printf("文件个数已达上限T\n");
		return ERROR;
	}
	//******************寻找文件区（目录项之后）的空闲块，分配给新创建的文件
	for (i = K + FILE_NUM; i < L; i++)
	{
		read_block((i - K) / BLOCKSIZE, temp);
		if (temp[(i - K) % BLOCKSIZE] == FREE)
		{
			freed2 = i;
			break;
		}
	}
	if (i == L)
	{
		printf("磁盘已满，分配失败\n");
		return ERROR;
	}


	//*****************无问题后开始操作
	file_descriptor temp_file_descriptor;						//创建临时文件描述符
	contents temp_contents;						//创建临时目录项

	//**************构建文件描述符
	temp_file_descriptor.file_descriptor_flag = 1;
	temp_file_descriptor.file_length = 0;
	temp_file_descriptor.file_block = 1;


	Init_block(temps, BLOCKSIZE);
	temps[0] = temp_file_descriptor.file_descriptor_flag;
	temps[1] = temp_file_descriptor.file_length;
	temps[2] = temp_file_descriptor.file_block;
	temps[3] = freed2;
	for (i = 4; i < FILE_BLOCK_LENGTH; i++)
	{
		temps[i] = '\0';
	}
	write_block(frees, temps);				//写入磁盘，文件描述符区

	//*****************构建目录项插入目录文件描述符所指定的数组块
	temp_contents.file_descriptor_num = frees - FILE_descriptor_AREA;
	strncpy(temp_contents.filename, filename, FILE_NAME_LENGTH);

	Init_block(tempc, BLOCKSIZE);
	tempc[0] = temp_contents.file_descriptor_num;
	tempc[1] = '\0';
	strcat(tempc, temp_contents.filename);
	write_block(freed, tempc);				//写入磁盘，数据区

	//*****************更改位图状态
	read_block((freed - K) / BLOCKSIZE, temp);			//更改位图状态(目录项所对应的)
	temp[(freed - K) % BLOCKSIZE] = BUSY;
	write_block((freed - K) / BLOCKSIZE, temp);			//写入磁盘，位图

	read_block((freed2 - K) / BLOCKSIZE, temp);			//更改位图状态
	temp[(freed2 - K) % BLOCKSIZE] = BUSY;
	write_block((freed2 - K) / BLOCKSIZE, temp);			//写入磁盘，位图文件内容所对应的)

	//****************增加目录文件描述符中的长度项
	read_block(FILE_descriptor_AREA, temp);
	temp[1]++;
	write_block(FILE_descriptor_AREA, temp);

	return OK;


}
```
##### &#8195;&#8195;删除文件时需要进行如下操作（假设文件没有被打开）：
##### &#8195;&#8195;&#8195;&#8195;•在目录里搜索该文件的描述符编号
##### &#8195;&#8195;&#8195;&#8195;•删除该文件对应的目录项并更新位图
##### &#8195;&#8195;&#8195;&#8195;•释放文件描述符
##### &#8195;&#8195;&#8195;&#8195;•返回状态信息

##### &#8195;&#8195;&#8195;&#8195;•destroy(filename): 删除指定文件。
```c
int destroy(const char * filename)
/**************************删除指定文件。
.在目录里搜索该文件的描述符编号
.删除该文件对应的目录项，并更新位图
.释放文件描述符
.返回状态信息
***************************/
{
	int i;
	int dtys;									//将要删除的文件的目录项的文件描述符位置
	int dtyd;									//将要删除的文件的目录项位置
	int use_block;									//该文件实际使用的块数
	int index;
	char temp[BLOCKSIZE];
	char tempd[BLOCKSIZE];


	//***************依据文件名寻找文件的目录项和文件描述符
	for (i = K; i < K + FILE_NUM; i++)
	{
		read_block((i - K) / BLOCKSIZE, temp);
		if (temp[(i - K) % BLOCKSIZE] == BUSY)
		{
			read_block(i, temp);
			if (strncmp(temp + 1, filename, FILE_NAME_LENGTH) == 0)
			{
				dtyd = i;					//找到文件目录项位置
				dtys = temp[0] + FILE_descriptor_AREA;		//找到文件目录项对应的文件描述符位置(序号为temp[0])
				index = temp[0];
				break;
			}
		}
	}
	if (i == K + FILE_NUM)
	{
		printf("没有找到该文件\n");
		return ERROR;
	}

	//************根据文件描述符即（index）查看该文件是否打开
	int list = -1;
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].file_descriptor_num == index)
		{
			list = i;
			break;
		}
	}
	if (open_list[list].flag == BUSY && list != -1)
	{
		printf("该文件已经-被打开,需要关闭才能删除\n");
		return ERROR;
	}


	//****************找到位置后开始操作


	//****************依次更新文件块数组中指定的块的位图区
	read_block(dtys, temp);
	use_block = temp[2];
	for (i = 0; i < use_block; i++)
	{
		read_block((temp[i + 3] - K) / BLOCKSIZE, tempd);
		tempd[(temp[i + 3] - K) % BLOCKSIZE] = FREE;
		write_block((temp[i + 3] - K) / BLOCKSIZE, tempd);
	}
	//***************删除该目录项
	Init_block(temp, BLOCKSIZE);
	write_block(dtys, temp);




	//***************删除文件描述符
	Init_block(temp, BLOCKSIZE);
	write_block(dtyd, temp);

	//****************更改位图区
	read_block((dtyd - K) / BLOCKSIZE, temp);
	temp[(dtyd - K) % BLOCKSIZE] = FREE;
	write_block((dtyd - K) / BLOCKSIZE, temp);
	//**************目录文件描述符中的长度减1
	read_block(FILE_descriptor_AREA, temp);
	temp[1]--;
	write_block(FILE_descriptor_AREA, temp);


	return OK;

}
```
### 3.5 文件的打开与关闭
##### &#8195;&#8195;文件系统维护一张打开文件表．打开文件表的长度固定，其表目包含如下信息：
##### &#8195;&#8195;&#8195;&#8195;•读写缓冲区
##### &#8195;&#8195;&#8195;&#8195;•读写指针
##### &#8195;&#8195;&#8195;&#8195;•文件描述符号
##### &#8195;&#8195;文件被打开时，便在打开文件表中为其分配一个表目；文件被关闭时，其对应的表目被释放。读写缓冲区的大小等于一个磁盘存储块。打开文件时需要进行的操作如下：
##### &#8195;&#8195;&#8195;&#8195;•搜索目录找到文件对应的描述符编号
##### &#8195;&#8195;&#8195;&#8195;•在打开文件表中分配一个表目
##### &#8195;&#8195;&#8195;&#8195;•在分配到的表目中把读写指针置为０，并记录描述符编号
##### &#8195;&#8195;&#8195;&#8195;•读入文件的第一块到读写缓冲区中
##### &#8195;&#8195;&#8195;&#8195;•返回分配到的表目在打开文件表中的索引号
##### &#8195;&#8195;&#8195;&#8195;•open(filename): 打开文件。该函数返回的索引号可用于后续的read, write, lseek,或close操作。
```c
int open(const char * filename)
/***************************打开文件。
.该函数返回的索引号可用于后续的read, write, lseek, 或close 操作。
.搜索目录找到文件对应的描述符序号
.在打开文件表中分配一个表目
.在分配到的表目中把读写指针置为0，并记录描述符编号
.读入文件的第一块到读写缓冲区中
.返回分配到的表目在打开文件表中的索引号
***************************/
{
	int i;
	int opd;
	int ops;
	int list;
	char temp[BLOCKSIZE];
	int index;
	//***************依据文件名寻找文件的目录项和文件描述符
	for (i = K; i < K + FILE_NUM; i++)
	{
		read_block((i - K) / BLOCKSIZE, temp);
		if (temp[(i - K) % BLOCKSIZE] == BUSY)
		{
			read_block(i, temp);
			if (strncmp(temp + 1, filename, FILE_NAME_LENGTH) == 0)
			{
				opd = i;							//找到文件目录项位置
				ops = temp[0];							//找到文件目录项对应的文件描述符序号
			//	printf("opd: %d,ops: %d\n",opd,ops);
				break;
			}
		}
	}
	if (i == K + FILE_NUM)
	{
		printf("没有找到该文件\n");
		return ERROR;
	}

	//*************查看该文件是否被打开
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].file_descriptor_num == ops && open_list[i].flag == BUSY)
		{
			printf("该文件已经被打开\n");
			return ERROR;
		}
	}

	//**************找一块没使用的表目以便分配
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].flag != BUSY)
		{
			list = i;
			break;
		}
	}

	//******************对表目进行操作

	open_list[list].file_descriptor_num = ops;							//写入文件描述符序号

	open_list[list].flag = BUSY;									//置标志位为占用

	index = open_list[list].file_descriptor_num;							//生成索引号(相当于该文件目录项的文件描述符序号)		
	lseek(index, 0);										//指针指向文件首部(即指针指为0)

	Init_block(open_list[list].buffer, BUFFER_LENGTH);				//初始化缓冲区
	read_block(open_list[list].pointer[0], temp);					//读文件的首块	
	strncpy(open_list[list].buffer, temp, BUFFER_LENGTH);				//把文件的第一块写入缓冲区

	return OK;

}
```
##### &#8195;&#8195;&#8195;&#8195;关闭文件时需要进行的操作如下：
##### &#8195;&#8195;&#8195;&#8195;•把缓冲区的内容写入磁盘
##### &#8195;&#8195;&#8195;&#8195;•释放该文件在打开文件表中对应的表目
##### &#8195;&#8195;&#8195;&#8195;•返回状态信息

##### &#8195;&#8195;&#8195;&#8195;•close(index): 关闭制定文件。
```c
int close(int index)
/***************************
.关闭文件。
.把缓冲区的内容写入磁盘
.释放该文件再打开文件表中对应的表目
.返回状态信息
***************************/
{
	int i;
	int list = -1;
	char temp[BLOCKSIZE];
	//***************根据index找表目
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].file_descriptor_num == index)
		{
			list = i;
			break;
		}
	}
	if (list == -1)					//没找到
	{
		printf("没找到当前索引号文件,操作失败...\n");
		return ERROR;
	}
	if (open_list[list].flag != BUSY)		//输入的index对应文件没被打开
	{
		printf("输入的索引号有误，操作失败...\n");
		return ERROR;
	}

	//****************将缓冲区的内容写入磁盘
	/* //测试用
	for(i = 0 ; i < BUFFER_LENGTH-1; i++ )
	{
		open_list[list].buffer[i] = 5;
	}
	*/

	write_buffer(index, list);			//将当前list打开文件表对应的缓冲区入index索引号的文件

	//****************清楚操作 释放表目
	Init_block(open_list[list].buffer, BUFFER_LENGTH);				//清除缓冲区
	open_list[list].file_descriptor_num = FREE;								//清除文件描述符
	open_list[list].flag = FREE;									//清除占用标志位
	open_list[list].pointer[0] = NULL;								//清楚指针
	open_list[list].pointer[1] = NULL;
	return OK;
}
```
### 3.6 读写
##### &#8195;&#8195;文件打开之后才能进行读写操作．读操作需要完成的任务如下：
##### &#8195;&#8195;1. 计算读写指针对应的位置在读写缓冲区中的偏移
##### &#8195;&#8195;2. 把缓冲区中的内容拷贝到指定的内存位置，直到发生下列事件之一：
##### &#8195;&#8195;&#8195;&#8195;•到达文件尾或者已经拷贝了指定的字节数。这时，更新读写指针并返回相应信息
##### &#8195;&#8195;&#8195;&#8195;•到达缓冲区末尾。这时，把缓冲区内容写入磁盘，然后把文件下一块的内容读入磁盘。最后返回第2步。

##### &#8195;&#8195;&#8195;&#8195;•read(index, mem_area, count): 从指定文件顺序读入count个字节memarea指定的内存位置。读操作从文件的读写指针指示的位置开始。
```c
int read(int index, int mem_area, int count)
/**************
从指定文件顺序读入count 个字
节mem_area 指定的内存位置。读操作从文件的读写指针指示的位置
开始。
**************/
{
	int i;
	int list = -1;
	char temp[BLOCKSIZE];
	//***************根据index找打开文件表表目
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].file_descriptor_num == index)
		{
			list = i;
			break;
		}
	}
	if (list == -1)					//没找到
	{
		printf("没找到当前索引号文件,操作失败...\n");
		return ERROR;
	}
	if (open_list[list].flag != BUSY)		//输入的index对应文件没被打开
	{
		printf("输入的索引号有误，操作失败...\n");
		return ERROR;
	}

	//***************根据index找文件描述符
	char temp_output[OUTPUT_LENGTH];
	Init_block(temp_output, OUTPUT_LENGTH);
	char output[OUTPUT_LENGTH];
	Init_block(output, OUTPUT_LENGTH);

	read_block(FILE_descriptor_AREA + index, temp);
	int file_length = temp[1];					//文件长度
	int file_block = temp[2];					//文件实际使用块
	int file_area;

	//**********************拷贝文件内容至temp_output
	for (i = 0; i < file_block - 1; i++)
	{
		read_block(FILE_descriptor_AREA + index, temp);
		read_block(temp[3 + i], temp);
		strncpy(temp_output + i * BLOCKSIZE, temp, BLOCKSIZE);
	}
	read_block(FILE_descriptor_AREA + index, temp);
	read_block(temp[3 + i], temp);
	strncpy(temp_output + i * BLOCKSIZE, temp, BLOCKSIZE);

	//******************当前文件读写坐标
	int x = open_list[list].pointer[0];
	int y = open_list[list].pointer[1];

	for (i = 0; i < file_block; i++)
	{
		read_block(FILE_descriptor_AREA + index, temp);
		if (temp[3 + i] == x)
		{
			break;
		}
	}
	file_area = i * BLOCKSIZE + y;							//转换为文件内相对位置									

	for (i = 0; i < count; i++)
	{
		output[i + mem_area] = temp_output[i + file_area];
	}

	printf("%s\n", output + mem_area);
	return OK;
}
```
##### &#8195;&#8195;&#8195;&#8195;•write(index, mem_area, count): 把memarea指定的内存位置开始的count个字节顺序写入指定文件。写操作从文件的读写指针指示的位置开始。
```c
int write(int index, int mem_area, int count)
/*
把mem_area 指定的内存位置开
始的count 个字节顺序写入指定文件。写操作从文件的读写指针指示
的位置开始。
*/
{
	int i;
	int list = -1;
	int input_length;
	char temp[BLOCKSIZE];

	//*************根据index找到文件打开表
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].file_descriptor_num == index)
		{
			list = i;
			break;
		}
	}
	if (list == -1)					//没找到
	{
		printf("没找到当前索引号文件,操作失败...\n");
		return ERROR;
	}
	if (open_list[list].flag != BUSY)		//输入的index对应文件没被打开
	{
		printf("输入的索引号有误，操作失败...\n");
		return ERROR;
	}

	char input[INPUT_LENGTH];
	Init_block(input, INPUT_LENGTH);
	i = 0;
	fflush(stdin);
	getchar();
	while (scanf("%c", &input[i]))
	{
		if (i == count - 1)											//回车时终止1读入
		{
			input[i] = '\0';
			break;
		}
		i++;
	}
	input_length = count;	

	//*******************考虑输入串的长度 需要写入的长度为 input_length - mem_area
	//缓冲区容量可写入输入内容不需要再分配
	if (count <= BUFFER_LENGTH)
	{
		strncat(open_list[list].buffer, input + mem_area, count);			//存入缓冲区
	}
	//大于缓冲区长度分次写入，需要分块写入
	else
	{
		int rest;						//当前缓冲区空闲容量
		for (i = 0; i < BUFFER_LENGTH; i++)
		{
			if (open_list[list].buffer[i] == FREE)
			{
				rest = BUFFER_LENGTH - i;
				break;
			}
		}
		//第一部分，缓冲区有一定容量rest 将缓冲区写入文件，清空缓冲区		
		strncat(open_list[list].buffer + BUFFER_LENGTH - rest, input + mem_area, rest);
		write_buffer(index, list);
		Init_block(open_list[list].buffer, BUFFER_LENGTH);
		//第二部分，循-环 (input_length - mem_area)/BUFFER_LENGTH 块缓冲区写入文件
		for (i = 0; i < (count / BUFFER_LENGTH) - 1; i++)
		{
			strncpy(open_list[list].buffer, (input + mem_area) + rest + i * BUFFER_LENGTH, BUFFER_LENGTH);
	
			write_buffer(index, list);
			Init_block(open_list[list].buffer, BUFFER_LENGTH);
		}
		//第三部分，(count%BUFFER_LENGTH)剩余未满一块的写入缓冲区
		Init_block(open_list[list].buffer, BUFFER_LENGTH);
		strncpy(open_list[list].buffer, (input + mem_area) + rest + i * BUFFER_LENGTH, count%BUFFER_LENGTH);
		int buffer_start;
		
	}
	write_buffer(index, list);
	return OK;
}

```

## 4 测试
##### &#8195;&#8195;为了能够对我们的模拟系统进行测试，请编写一个操纵文件系统的外壳程序或者一个菜单驱动系统。为了能够对我们的模拟系统进行测试，请编写一个操纵文件系统的外壳程序或者一个菜单驱动系统。
