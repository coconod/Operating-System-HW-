#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//辅助函数
int show_openlist();				//显示打开文件表,返回打开文件个数
void directory();					//显示目录文件详细信息
void show_help();					//该文件系统的帮助
void show_ldisk();					//显示磁盘内容（辅助用）
//核心函数
void read_block(int, char *);		//文件系统与IO设备的接口函数，读取块
void write_block(int, char *);		//文件系统与IO设备的接口函数，写入块
void Init();						//初始化文件系统
int create(const char[]);					//创建文件
int destroy(const char[]);				//删除文件
int open(const char[]);					//打开文件
int close(int);						//关闭文件
int read(int, int, int);				//读文件
int write(int, int, int);				//写文件
int write_buffer(int, int);			//把缓冲区内容写入文件
int lseek(int, int);					//定位文件指针
void Init_block(char, int);			//初始化字符数组块（辅助）

void read_block(int i, char *p)
/**************************
读磁盘块
该函数把逻辑块i的内容读入到指针p 指向的内存位置
拷贝的字符个数为存储块的长度BLOCKSIZE。
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

void write_block(int i, char *p)
/**************************
写磁盘块
该函数把指针p 指向的内容写入逻辑块i
拷贝的字符个数为存储块的长度BLOCKSIZE。
***************************/
{
	char * temp = (char *)malloc(sizeof(char));
	int x = i / (H*B);
	int y = (i - x * (H*B)) / B;
	int z = i - x * (H*B) - y * B;
	temp = p;
	for (int a = 0; a < BLOCKSIZE;)
	{
		ldisk[x][y][z][a] = *temp;
		a++;
		temp++;
	}
}

void Init_block(char *temp, int length)
/**************************
初始化一个字符数组块
处理的字符数组块长度为length
内容填充为'\0'
***************************/
{
	int i;
	for (i = 0; i < length; i++)
	{
		temp[i] = '\0';
	}
}

int write_buffer(int index, int list)
{

	int i;
	int j;
	int freed;
	char temp[BLOCKSIZE];

	int buffer_length = BUFFER_LENGTH;
	for (i = 0; i < BUFFER_LENGTH; i++)
	{
		if (open_list[list].buffer[i] == '\0')
		{
			buffer_length = i;					//缓冲区有效长度
			break;
		}
	}

	int x = open_list[list].pointer[0];
	int y = open_list[list].pointer[1];
	int z = BLOCKSIZE - y;									//当前块空闲容量


	if (buffer_length < z)					//块容量可写入缓冲区不需要再分配
	{
		read_block(x, temp);
		strncat(temp + y, open_list[list].buffer, buffer_length);			//缓冲区接入

		write_block(x, temp);

		read_block(index + FILE_descriptor_AREA, temp);	//更改文件长度
		temp[1] += buffer_length;
		write_block(index + FILE_descriptor_AREA, temp);

		open_list[list].pointer[0] = x;
		open_list[list].pointer[1] = y + buffer_length;					//更新文件读写指针
	}
	else									//大于，需要分配新块
	{
		read_block(index + FILE_descriptor_AREA, temp);
		if (temp[2] + (buffer_length - z) / BLOCKSIZE + 1 > FILE_BLOCK_LENGTH)
		{
			printf("文件分配数组不够分配\n");
			return ERROR;
		}

		//分别三部分
		//第一部分 填满
		read_block(x, temp);
		strncat(temp + y, open_list[list].buffer, z);			//缓冲区接入的长度，填满当前块
		write_block(x, temp);

		//第二部分 还需要分配 (buffer_length - z)/BLOCKSIZE+1块 

		//******************寻找文件区（目录项之后）的空闲块，分配新块
		for (i = 0; i < (buffer_length - z) / BLOCKSIZE; i++)
		{
			for (j = K + FILE_NUM; j < L; j++)
			{
				read_block((j - K) / BLOCKSIZE, temp);
				if (temp[(j - K) % BLOCKSIZE] == FREE)
				{
					freed = j;
					//			printf("freed= : %d\n",freed);
					break;
				}
			}
			if (j == L)
			{
				printf("磁盘已满，分配失败\n");
				return ERROR;
			}

			Init_block(temp, BLOCKSIZE);
			strncpy(temp, (open_list[list].buffer + z + (i*BLOCKSIZE)), BLOCKSIZE);
			write_block(freed, temp);				//写入内容

			read_block((freed - K) / BLOCKSIZE, temp);			//更改位图状态
			temp[(freed - K) % BLOCKSIZE] = BUSY;
			write_block((freed - K) / BLOCKSIZE, temp);			//写入磁盘，位图文件内容所对应的)

			read_block(index + FILE_descriptor_AREA, temp);
			temp[2] ++;								//增加文件分配数组长度
			temp[2 + temp[2]] = freed;
			write_block(index + FILE_descriptor_AREA, temp);
		}

		//(buffer_length - z)%BLOCKSIZE 不满一块在尾部的部分
		for (j = K + FILE_NUM; j < L; j++)
		{
			read_block((j - K) / BLOCKSIZE, temp);
			if (temp[(j - K) % BLOCKSIZE] == FREE)
			{
				freed = j;
				break;
			}
		}
		if (j == L)
		{
			printf("磁盘已满，分配失败\n");
			return ERROR;
		}
		Init_block(temp, BLOCKSIZE);
		//	printf("(buffer_length - z)%BLOCKSIZE = %d\n",(buffer_length - z)%BLOCKSIZE);
		strncpy(temp, (open_list[list].buffer + z + (i*BLOCKSIZE)), (buffer_length - z) % BLOCKSIZE);
		write_block(freed, temp);				//写入内容

		read_block((freed - K) / BLOCKSIZE, temp);			//更改位图状态
		temp[(freed - K) % BLOCKSIZE] = BUSY;
		write_block((freed - K) / BLOCKSIZE, temp);			//写入磁盘，位图文件内容所对应的)

		read_block(index + FILE_descriptor_AREA, temp);
		temp[2] ++;								//增加文件分配数组长度
		temp[2 + temp[2]] = freed;
		write_block(index + FILE_descriptor_AREA, temp);


		read_block(index + FILE_descriptor_AREA, temp);
		temp[1] += buffer_length;				//更改文件长度
		write_block(index + FILE_descriptor_AREA, temp);

		open_list[list].pointer[0] = freed;
		open_list[list].pointer[1] = (buffer_length - z) % BLOCKSIZE;
	}
}
int lseek(int index, int pos)
/*
把文件的读写指针移动到pos 指定的位置。pos
是一个整数，表示从文件开始位置的偏移量。文件打开时，读写指针
自动设置为0。每次读写操作之后，它指向最后被访问的字节的下一
个位置。lseek 能够在不进行读写操作的情况下改变读写指针能位置。
*/
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


void Init()
/**************************
初始化磁盘 将磁盘全部置空
然后创建0号文件描述符为根目录的文件描述符
初始化位图区
***************************/
{

	int i;
	char temp[BLOCKSIZE];


	for (i = 0; i < L; i++)
	{
		Init_block(temp, BLOCKSIZE);
		write_block(i, temp);			//初始化磁盘
	}

	for (i = K; i < L; i += BLOCKSIZE)						//初始化保留区中的位图
	{
		read_block((i - K) / BLOCKSIZE, temp);
		for (int j = 0; j < BLOCKSIZE; j++) {
			temp[(i + j - K) % BLOCKSIZE] = FREE;
		}
		write_block((i - K) / BLOCKSIZE, temp);
	}

	//**************初始化目录的文件标识符
	file_descriptor temp_descriptor;
	temp_descriptor.file_descriptor_flag = 1;
	temp_descriptor.file_length = 0;					//文件长度初始为0					
	temp_descriptor.file_block = FILE_BLOCK_LENGTH;	//分配FILE_BLOCK_LENGTH*B的空间（在数据区前段）用于储存目录项

	Init_block(temp, BLOCKSIZE);
	temp[0] = temp_descriptor.file_descriptor_flag;
	temp[1] = temp_descriptor.file_length;
	temp[2] = temp_descriptor.file_block;

	for (i = 0; i < FILE_BLOCK_LENGTH; i++)
	{
		temp[i + 3] = K + i;									//默认数据区的第前FILE_BLOCK_LENGTH被根目录占用
	}
	write_block(FILE_descriptor_AREA, temp);


}


int create(const char filename[])
/**************************根据文件名创建文件。
.找一空闲文件描述符
.在文件目录里为新创建的文件分配一个目录项，（可能需要为目录文件分配新的磁盘块）
.在分配到的目录项里记录文件名以及描述符编号
.返回状态信息
***************************/
{
	int i;
	int frees;							//空闲的文件描述符位置	
	int	freed;							//空闲数据区位置（目录项）
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

int destroy(const char * filename)
/**************************删除指定文件。
.在目录里搜索该文件的描述符编号
.删除该文件对应的目录项，并更新位图
.释放文件描述符
.返回状态信息
***************************/
{
	int i;
	int dtys;										//将要删除的文件的目录项的文件描述符位置
	int dtyd;										//将要删除的文件的目录项位置
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
				dtyd = i;								//找到文件目录项位置
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
int open(const char * filename)
/***************************打开文件。
该函数返回的索引号可用于后续的read, write, lseek, 或close 操作。
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
				opd = i;								//找到文件目录项位置
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

	open_list[list].file_descriptor_num = ops;								//写入文件描述符序号

	open_list[list].flag = BUSY;									//置标志位为占用

	index = open_list[list].file_descriptor_num;							//生成索引号(相当于该文件目录项的文件描述符序号)		
	lseek(index, 0);													//指针指向文件首部(即指针指为0)

	Init_block(open_list[list].buffer, BUFFER_LENGTH);				//初始化缓冲区
	read_block(open_list[list].pointer[0], temp);					//读文件的首块	
	strncpy(open_list[list].buffer, temp, BUFFER_LENGTH);				//把文件的第一块写入缓冲区

	return OK;

}

int close(int index)
/***************************关闭文件。
.把缓冲区的内容写入磁盘
.释放该文件再打开文件表中对应的表目
.返回状态信息
***************************/
{
	int i;
	int list = -1;
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

	write_buffer(index, list);			//将当前list打开文件表对应的缓冲区入index索引号的文件

	//****************清楚操作 释放表目
	Init_block(open_list[list].buffer, BUFFER_LENGTH);				//清除缓冲区
	open_list[list].file_descriptor_num = FREE;								//清除文件描述符
	open_list[list].flag = FREE;									//清除占用标志位
	open_list[list].pointer[0] = NULL;								//清楚指针
	open_list[list].pointer[1] = NULL;
	return OK;
}
int read(int index, int mem_area, int count)
/*
从指定文件顺序读入count 个字
节mem_area 指定的内存位置。读操作从文件的读写指针指示的位置
开始。
*/
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
		if (i == count)											//回车时终止1读入
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
	//write_buffer(index, list);
	return OK;
}

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

int show_openlist()
{

	int i, j;
	int openfile = 0;
	char temp[BLOCKSIZE];
	int index;
	printf("\n索引号\t\t大小\t\t文件名\n");
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].flag == BUSY)
		{
			index = open_list[i].file_descriptor_num;
			printf("  %-2d", index);
			openfile++;
			read_block(FILE_descriptor_AREA + index, temp);
			printf("\t\t %-2d", temp[1]);
			//根据index(文件描述符序号)找到其目录项
			for (j = K; j < K + FILE_NUM; j++)
			{
				read_block(j, temp);
				if (temp[0] == index)
				{
					printf("\t\t%-10s\n", temp + 1);
				}
			}
		}
	}
	return openfile;

}
void show_help()
{
	printf("**----------------------------------------------------------------------**\n");
	printf("**    命令                   说明                                       **\n");
	printf("** dir                  显示目录内容                                    **\n");
	printf("**                                                                      **\n");
	printf("** create+filename      新建以filename为文件名的文件                    **\n");
	printf("**                                                                      **\n");
	printf("** delete+filename      删除以filename为文件名的文件                    **\n");
	printf("**                                                                      **\n");
	printf("** open+filename        打开以filename为文件名的文件                    **\n");
	printf("**                                                                      **\n");
	printf("** close                关闭index为索引号的文件                         **\n");
	printf("**                                                                      **\n");
	printf("** read                 请根据提示，接着要求输入索引号以及读取长度进行读**\n");
	printf("**                      取文件操作                                      **\n");
	printf("**                                                                      **\n");
	printf("** write                请根据提示，接着要求输入索引号以及写入长度进行写**\n");
	printf("**                      入文件操作                                      **\n");
	printf("**                                                                      **\n");
	printf("** lseek                请根据提示，接着要求输入一个不大于文件长度的数字**\n");
	printf("**                      用于定位读写指针                                **\n");
	printf("**                                                                      **\n");
	printf("** help                 帮助                                            **\n");
	printf("**                                                                      **\n");
	printf("** exit                 退出文件系统                                    **\n");
	printf("**----------------------------------------------------------------------**\n");
}



void show_ldisk()
{
	int a, b;
	a = 0;
	for (int c = 0; c < C; c++) {
		for (int h = 0; h < H; h++) {
			for (int b = 0; b < B; b++) {
				if (a < K + 30)
					printf("%-3d :", a++);
				else
					break;
				for (int i = 0; i < BLOCKSIZE; i++)
				{
					printf("%-3d ", ldisk[c][h][b][i]);
				}
				printf("\n");
			}
		}
	}

}
