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

### ***最近最久未使用(LRU)算法及其近似算法***

### ***改进型Clock置换算法***

### ***页面缓冲算法(PBA)***
