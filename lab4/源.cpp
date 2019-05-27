#include<iostream>
#include<stdlib.h>
#include<time.h>
#include<string>
using namespace std;

#define MAX 1000000

int N = 64;  //����ߴ�=�����ڴ�/ҳ���С
int memN = 32; //�ڴ��������=ʵ���ڴ�/ҳ���С

int mem[MAX];      //�ڴ��ʼ��-1
int memcount = 0;//�ڴ�������

int misscount = 0;//ȱҳ����
double pr = 0;//ȱҳ��

clock_t t1, t2;     //��ʼʱ���Լ�����ʱ��

//ҳ����������������
int list[MAX];    //ҳ������������

void PageAccessSeq() {
	int len = 0;
	int p = 0;//����������ʼλ��
	int e = 6;//������������ҳ��
	int m = 10;//�������ƶ���
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

//�ڴ������ʼ��
void init() {
	//------------��ʼ��------------
	for (int i = 0; i < memN; i++) {
		mem[i] = -1;
	}
	memcount = 0;
	misscount = 0;
}

//�����Ƿ����ڴ��� ����-����λ�� ������-����-1
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

//����list���ʱ�䲻�ٱ����ʵ�Ԫ�ز���ҳ
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

//�����̭�㷨
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
				//cout << "����ȱҳ�ж�" << endl;
			}
		}
	}
	t2 = clock();
	pr = misscount * 1.0 / MAX;
	cout << "OPT:\tȱҳ��:" << pr << "\t" << "ʱ�俪��:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
}

//������δʹ��
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
				//cout << "����ȱҳ�ж�" << endl;
			}
		}
	}
	t2 = clock();
	pr = misscount * 1.0 / MAX;
	cout << "LRU:\tȱҳ��:" << pr << "\t" << "ʱ�俪��:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
}

//�Ƚ��ȳ�
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
				//cout << "����ȱҳ�ж�" << endl;
			}
		}
	}
	t2 = clock();
	pr = misscount * 1.0 / MAX;
	cout << "FIFO:\tȱҳ��:" << pr << "\t" << "ʱ�俪��:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
}

int A[32] = { -1 };//����λ
int M[32] = { -1 };//�޸�λ

//�Ľ�ʱ���㷨
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
			M[i] = 1;//��д��ʽ����
			A[i] = 0;
		}
		else {
			M[i] = 0;//�Զ���ʽ����
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
	cout << "M_CLOCK:\tȱҳ��:" << pr << "\t" << "ʱ�俪��:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
}
#define buffmax 5
int buff[buffmax] = { -1 };
int bufforder[buffmax] = { -1 };
int bufford = 0;
//�����Ƿ��ڻ����� ����-����λ�� ������-����-1
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
				//�ڴ���� 
			}
			else if (flag == -1 && flag2 != -1) {
				//�ڴ治���ڣ��������
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
				//�ڴ治���ڣ����治����
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
	cout << "PBA:\tȱҳ��:" << pr << "\t" << "ʱ�俪��:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
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