#include<iostream>
#include<stdlib.h>
#include<time.h>
#include<string>
using namespace std;

#define MAX 1000000

int N = 64;  //虚拟尺寸=虚拟内存/页面大小
int memN = 32; //内存物理块数=实际内存/页面大小

int mem[MAX];      //内存初始化-1
int memcount = 0;//内存块计数。

int misscount = 0;//缺页次数
double pr = 0;//缺页率

clock_t t1, t2;     //开始时间以及结束时间

//页面访问序列随机生成
int list[MAX];    //页面访问随机序列

void PageAccessSeq() {
	int len = 0;
	int p = 0;//工作集的起始位置
	int e = 6;//工作集包含的页数
	int m = 10;//工作集移动率
	srand((unsigned)time(NULL));
	double t = rand()*1.0 / RAND_MAX;
	for (int i = 0; i < m; i++) {
		list[len++] = rand() % e + p;
	}

	while (len < MAX) {
		for (int i = 0; i < m; i++) {
			int x = rand() % e + p;
			if (x > N - 1)
				list[len++] = N - 1;
			else
				list[len++] = x;
		}
		double r = rand()*1.0 / RAND_MAX;
		if (r < t) {
			p = rand() % (N - e);
		}
		else {
			p = (p + 1) % N;
		}
	}
}

//内存数组初始化
void init() {
	//------------初始化------------
	for (int i = 0; i < memN; i++) {
		mem[i] = -1;
	}
	memcount = 0;
	misscount = 0;
}

//查找是否在内存中 存在-返回位置 不存在-返回-1
int judge(int pos) {
	int flag = -1;
	for (int i = 0; i < memN; i++) {
		if (mem[i] == list[pos]) {
			flag = i;
			break;
		}
	}
	return flag;
}

//查找list中最长时间不再被访问的元素并换页
void search(int tail) {
	int cur = 0;
	int last[32];
	int min = -1;
	for (int i = 0; i < memN; i++) {
		last[i] = MAX;
	}
	for (int j = 0; j < memN; j++) {
		for (int i = tail + 1; i < MAX; i++){
			if (mem[j] == list[i]) {
				last[j] = i;
				break;
			}
		}
	}
	for (int i = 0; i < memN; i++) {
		if (last[i] > min) {
			cur = i;
			min = last[i];
		}
	}
	mem[cur] = list[tail];
}

//最佳淘汰算法
void OPT() {
	init();
	t1 = clock();
	int flag = -1;
	for (int i = 0; i < MAX; i++) {
		flag = judge(i);
		if (memcount < memN ) {
			if(flag == -1)
				mem[memcount++] = list[i];
		}
		else {
			if (flag != -1) {
			}
			else {
				search(i);
				misscount++;
				//cout << "发生缺页中断" << endl;
			}
		}
	}
	t2 = clock();
	pr = misscount * 1.0 / MAX;
	cout << "OPT:\t缺页率:" << pr << "\t" << "时间开销:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
}

//最近最久未使用
void LRU() {
	init();
	t1 = clock();
	int order[32] = { 0 };
	int ord = 0;
	int min, temp;
	int flag = -1;
	for (int i = 0; i < MAX; i++) {
		flag = judge(i);
		if (memcount < memN) {
			if (flag == -1) {
				mem[memcount] = list[i];
				order[memcount++] = ord++;
			}
			else {
				order[flag] = ord++;
			}
		}
		else {
			if (flag != -1) {
				order[flag] = ord++;
			}
			else {
				min = MAX;
				temp = -1;
				for (int j = 0; j < memN; j++) {
					if (order[j] < min) {
						min = order[j];
						temp = j;
					}
				}
				mem[temp] = list[i];
				order[temp] = ord++;
				misscount++;
				//cout << "发生缺页中断" << endl;
			}
		}
	}
	t2 = clock();
	pr = misscount * 1.0 / MAX;
	cout << "LRU:\t缺页率:" << pr << "\t" << "时间开销:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
}

//先进先出
void FIFO() {
	init();
	t1 = clock();
	int order[32] = { 0 };
	int ord = 0;
	int min, temp;
	int flag = -1;
	for (int i = 0; i < MAX; i++) {
		flag = judge(i);
		if (memcount < memN) {
			if (flag == -1) {
				mem[memcount] = list[i];
				order[memcount++] = ord++;
			}
		}
		else {			
			if (flag != -1) {
				
			}
			else {
				min = MAX;
				temp = -1;
				for (int j = 0; j < memN; j++) {
					if (order[j] < min) {
						min = order[j];
						temp = j;
					}
				}
				mem[temp] = list[i];
				order[temp] = ord++;
				misscount++;
				//cout << "发生缺页中断" << endl;
			}
		}
	}
	t2 = clock();
	pr = misscount * 1.0 / MAX;
	cout << "FIFO:\t缺页率:" << pr << "\t" << "时间开销:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
}

int A[32] = { -1 };//访问位
int M[32] = { -1 };//修改位

//改进时钟算法
int Gloop(int n, int tail) {
	int temp;
	for (int i = n + 1;; i++) {
		temp = i % memN;
		if (A[temp] == 0 && M[temp] == 0) {
			mem[temp] = list[tail];
			A[temp] = 1;
			n = temp;
			break;
		}
		if (i > n + memN - 1) {
			if (A[temp] == 0 && M[temp] == 1) {
				mem[temp] = list[tail];
				A[temp] = 1;
				n = temp;
				break;
			}
			A[temp] = 0;
		}
		if (i >= n + 2 * memN - 1)
			i = n;
	}
	return n;
}

void MCLOCK() {
	init();
	int pos = -1;
	int flag = -1;
	double temp;
	for (int i = 0; i < 32; i++) {
		temp = (rand() % 10) *0.1;
		if (temp > 0.7) {
			M[i] = 1;//以写方式访问
			A[i] = 0;
		}
		else {
			M[i] = 0;//以读方式访问
			A[i] = 0;
		}
	}
	t1 = clock();
	for (int i = 0; i < MAX; i++) {
		flag = judge(i);
		if (memcount < memN) {
			if (flag == -1) {
				mem[memcount] = list[i];
				A[memcount++] = 1;
			}
		}
		else {
			if (flag != -1) {
				A[flag] = 1;
			}
			else {
				pos = Gloop(pos, i);
				misscount++;
			}
		}		
	}
	t2 = clock();
	pr = misscount * 1.0 / MAX;
	cout << "M_CLOCK:\t缺页率:" << pr << "\t" << "时间开销:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
}
#define buffmax 5
int buff[buffmax] = { -1 };
int bufforder[buffmax] = { -1 };
int bufford = 0;
//查找是否在缓存中 存在-返回位置 不存在-返回-1
int judge2(int pos) {
	int flag = -1;
	for (int i = 0; i < buffmax; i++) {
		if (buff[i] == list[pos]) {
			flag = i;
			bufforder[i] = bufford++;
			break;
		}
	}
	return flag;
}

void PBA() {
	init();
	t1 = clock();
	int order[32] = { 0 };
	int ord = 0;
	
	int flag = -1, flag2 = -1;
	int min, temp, min2, temp2;
	int buffcount = 0;
	for (int i = 0; i < MAX; i++) {
		flag = judge(i);
		flag2 = judge2(i);
		if (memcount < memN) {
			if (flag == -1) {
				mem[memcount] = list[i];
				order[memcount++] = ord++;
			}
		}
		else if (buffcount < buffmax) {
			min = MAX;
			temp = -1;
			for (int j = 0; j < memN; j++) {
				if (order[j] < min) {
					min = order[j];
					temp = j;
				}
			}
			buff[buffcount++] = mem[temp];
			mem[temp] = list[i];
			order[temp] = ord++;
		}
		else {
			if (flag != -1) {
				//内存存在 
			}
			else if (flag == -1 && flag2 != -1) {
				//内存不存在，缓存存在
				min = MAX;
				temp = -1;
				for (int j = 0; j < memN; j++) {
					if (order[j] < min) {
						min = order[j];
						temp = j;
					}
				}
				min2 = buffmax;
				temp2 = -1;
				for (int j = 0; j < buffmax; j++) {
					if (bufforder[j] < min2) {
						min2 = bufforder[j];
						temp2 = j;
					}
				}
				buff[temp2] = mem[temp];
				bufforder[temp2] = bufford++;
				mem[temp] = list[i];
				order[temp] = ord++;
			}
			else if (flag == -1 && flag2 == -1) {
				//内存不存在，缓存不存在
				min = MAX;
				temp = -1;
				for (int j = 0; j < memN; j++) {
					if (order[j] < min) {
						min = order[j];
						temp = j;
					}
				}
				min2 = buffmax;
				temp2 = -1;
				for (int j = 0; j < buffmax; j++) {
					if (bufforder[j] < min2) {
						min2 = bufforder[j];
						temp2 = j;
					}
				}
				buff[temp2] = mem[temp];
				bufforder[temp2] = bufford++;
				mem[temp] = list[i];
				order[temp] = ord++;
				misscount++;
			}
			
		}
	}
	t2 = clock();
	pr = misscount * 1.0 / MAX;
	cout << "PBA:\t缺页率:" << pr << "\t" << "时间开销:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
}

int main(){
	PageAccessSeq();
//	for (int i = 0; i < MAX; i++) {
	//	cout << list[i] << " ";
	//}
	OPT();
	FIFO();
	LRU();
	MCLOCK();
	PBA();
	system("pause");
	return 0;
}