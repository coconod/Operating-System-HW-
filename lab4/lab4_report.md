# ***页面置换算法课题 实验报告***
## ***实验目的及基本要求***
##### &#8195;&#8195;设计和实现最佳置换算法、先进先出置换算法、最近最久未使用置换算法、改进型Clock置换算法、页面缓冲置换算法；通过页面访问序列随机发生器实现对上述算法的测试及性能比较。
## ***课题假设前提说明***
##### &#8195;&#8195;模拟的虚拟内存的地址为16位，页面大小为1K<br>&#8195;&#8195;模拟的物理内存有32K
##### &#8195;&#8195;页表用整数数组或结构数组来表示<br>&#8195;&#8195;页面访问序列串是一个整数序列，整数的取值范围为0到N - 1。页面访问序列串中的每个元素p表示对页面p的一次访问
### ***页面访问序列随机生成说明***
#### ***符合局部访问特性的随机生成算法***
##### &#8195;&#8195;1.确定虚拟内存的尺寸N，工作集的起始位置p，工作集中包含的页数e，工作集移动率m（每处理m个页面访问则将起始位置p +1），以及一个范围在0和1之间的值t；<br>&#8195;&#8195;2.生成m个取值范围在p和p + e间的随机数，如果数大于N-1，则取N-1，并记录到页面访问序列串中；<br>&#8195;&#8195;3.生成一个随机数r，0 ≤ r ≤ 1；<br>&#8195;&#8195;4.如果r < t，则为p生成一个新值，否则p = (p + 1) mod N；<br>&#8195;&#8195;5.如果想继续加大页面访问序列串的长度，请返回第2步，否则结束。
## ***性能测评及问题说明***
##### &#8195;&#8195;测试不同的页面访问序列及不同的虚拟内存尺寸，并从缺页率、算法开销等方面对各个算法进行比较。<br>&#8195;&#8195;（同时请给出在给定页面访问序列的情况下，发生页面置换次数的平均值）

## ***实验过程***
### ***公共函数部分：***
#### **页面访问序列随机生成函数：**
##### **函数流程图：**
<img src="https://github.com/coconod/Operating-System-HW-/blob/master/lab4/images/Accessaqu().png?raw=true" height="40%" width="40%"> 

##### **函数代码：**
```c
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

```
#### **内存数组初始化函数：**
```c
void init() {
	//------------初始化------------
	for (int i = 0; i < memN; i++) {
		mem[i] = -1;
	}
	memcount = 0;
	misscount = 0;
}
```
#### **判断函数：**
```c
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
```
### ***最佳(OPT)置换算法***
#### **函数流程图：**
<img src="https://github.com/coconod/Operating-System-HW-/blob/master/lab4/images/OPT.png?raw=true" height="50%" width="50%"> 

#### **基本思想：**
##### &#8195;&#8195;选择永不使用或是在最长时间内不再被访问（即距现在最长时间才会被访问）的页面淘汰出内存
#### **评价：**
##### &#8195;&#8195;理想化算法，具有最好性能（对于固定分配页面方式，本法可保证获得最低的缺页率），但实际上却难于实现，故主要用于算法评价参照
#### **代码：**
```c
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
```
### ***先进先出(FIFO)置换算法***
#### **函数流程图：**
<img src="https://github.com/coconod/Operating-System-HW-/blob/master/lab4/images/FIFO.png?raw=true" height="50%" width="50%"> 

#### **基本思想：**
##### &#8195;&#8195;选择最先进入内存即在内存驻留时间最久的页面换出到外存<br>&#8195;&#8195;进程已调入内存的页面按进入先后次序链接成一个队列，并设置替换指针以指向最老页面;<br>&#8195;&#8195;为了方便，我增加了一个order数组记录次序，没有使用链表。

#### **评价：**
##### &#8195;&#8195;简单直观，但不符合进程实际运行规律，性能较差，故实际应用极少
#### **代码：**
```c
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
```
### ***最近最久未使用(LRU)算法及其近似算法***
#### **函数流程图：**
<img src="https://github.com/coconod/Operating-System-HW-/blob/master/lab4/images/LRU.png?raw=true" height="50%" width="50%"> 

#### **基本思想：**
##### &#8195;&#8195;以“最近的过去”作为“最近的将来”的近似，选择最近一段时间最长时间未被访问的页面淘汰出内存。
#### **评价：**
##### &#8195;&#8195;适用于各种类型的程序，性能较好，但需要较多的硬件支持

#### **代码：**
```c
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
```

### ***改进型Clock置换算法***
#### **基本思想：**
##### &#8195;&#8195;① 从查寻指针当前位置起扫描内存分页循环队列，选择A=0且M=0的第一个页面淘汰；若未找到，转②<br>&#8195;&#8195;② 开始第二轮扫描，选择A=0且M=1的第一个页面淘汰，同时将经过的所有页面访问位置0；若不能找到，转①
#### **实现思想：**
##### &#8195;&#8195;给修改位M随机赋值1或0，访问位赋值0，
##### &#8195;&#8195;1类(A =0, M = 0)：表示该页面最近既未被访问，又未被修改，是最佳淘汰页。
##### &#8195;&#8195;2类(A =0, M = 1)：表示该页面最近未被访问，但已被修改，并不是很好的淘汰页。
##### &#8195;&#8195;3类(A =1, M = 0)：表示该页面最近已被访问，但未被修改，该页有可能再被访问。
##### &#8195;&#8195;4类(A =1, M = 1)：表示该页最近已被访问且被修改，该页可能再被访问。
##### &#8195;&#8195;从指针所指示的当前位置开始，扫描循环队列，寻找第一类页面，将所遇到的第一个页面作为所选中的淘汰页。在第一次扫描期间不改变访问位A.<br>&#8195;&#8195;如果第一步失败，即查找一周后未遇到第一类页面，则开始第二轮扫描，寻找第二类页面，将所遇到的第一个这类页面作为淘汰页。在第二轮扫描期间，将所有扫描过的页面的访问位都置0.<br>&#8195;&#8195;如果第二步也失败，亦即未找到第二类页面，则将指针返回到开始位置，并将所有访问位复0.返回第一步。
#### **评价：**
##### &#8195;&#8195;与简单Clock算法相比，可减少磁盘的I/O操作次数，但淘汰页的选择可能经历多次扫描，故实现算法自身的开销增大
#### **代码：**
```c
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
```

### ***页面缓冲算法(PBA)***
#### **基本思想：**
##### &#8195;&#8195;设立空闲页面链表<br>&#8195;&#8195;采用可变分配和基于先进先出的局部置换策略，并规定被淘汰页先不做物理移动，而是挂到空闲页面链表的末尾<br>&#8195;&#8195;这时页面在内存中并不做物理上的移动，而只是将页表中的表项移到上述两个链表之一中。
#### **实现思想：**
##### &#8195;&#8195;在FIFO算法上加以改进，增加一个buffer数组，模拟被淘汰的页面挂入的链表，当页面在buffer中时，直接调换位置，不发生缺页中断。
#### **评价：**
##### &#8195;&#8195;1.显著地降低了页面换进、换出的频率，使磁盘I/O的操作次数大大减少，因而减少了页面换进、换出的开销；2.正式由于换入换出的开销大幅度减小，才能使其采用一种较简单的置换策略。
#### **代码：**
```c
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
```
## ***实验运行结果分析***
#### **实验运行截图：**
##### 截图1
<img src="https://github.com/coconod/Operating-System-HW-/blob/master/lab4/images/%E6%88%AA%E5%9B%BE.PNG?raw=true" height="50%" width="50%"> 

##### 截图2
<img src="https://github.com/coconod/Operating-System-HW-/blob/master/lab4/images/%E6%88%AA%E5%9B%BE2.PNG?raw=true" height="50%" width="50%"> 

#### 结果分析
##### &#8195;&#8195;由结果明显得到OPT的缺页率最低，但是由于遍历剩下所有待访问页面时间开销很大，由于该算法不可实现，仅在此作为一个对比项。改进CLOCK、LRU、FIFO、PBA四种算法，时间开销相仿，PBA的时间开销略高（由于并没有I/O时间模拟），缺页率略微低一些，由于PBA的缓存块在实际中应该是多个进程公用，在此实验全部分配给一个进程，所以缺页率必然会有所降低，所以此结果并不具备明显代表性。
