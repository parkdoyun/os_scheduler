#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// 오퍼레이팅 시스템 homework (scheduler)
// 12161569 박도윤

// class number 1:sjf, 2:rr8, 3:rr20, 4:priority, 5:my own
// 큐 간 스케줄링 : 우선 순위 (queue1-> queue2-> queue3-> queue4-> queue5)

// process 구조체
typedef struct _process {
	int class_n; // class number
	int process_id; // process id
	int prior; // priority
	int cpu_burst; // cpu burst
	int exit_flag; // 해당 process가 종료되면 1, 수행 중이면 0
}process;

// pthread_create 함수에서 인자로 넘기기 위해 생성한 구조체.
typedef struct _arg_proc {
	process *proc; // process가 들어가는 큐.
	int total_t; // 큐에 있는 process들의 총 cpu burst
	int arr_n; // 큐에 있는 process들의 개수.
}arg_proc;


void* schedule_sjf(void *arg_p); // shortest job first scheduling
void* schedule_rr8(void *arg_p); // round-robin scheduling (time quantum : 8)
void* schedule_rr20(void *arg_p); // round-robin scheduling (time quantum : 20)
void* schedule_priority(void *arg_p); // priority scheduling
void* schedule_myOwn(void *arg_p); // my own scheduling

sem_t sem1; // semaphore.
sem_t sem2;
sem_t sem3;

int main(void)
{
	int total_proc = 0; // 전체 process의 개수
	printf("Enter number of total process.(max : 50)\n");
	scanf("%d", &total_proc); // 입력
	printf("Enter class num, priority, cpu burst\n");
	process queue_sjf[10];
	process queue_rr8[10];
	process queue_rr20[10]; // process 배열 형태로 큐 생성. (최대 10개)
	process queue_prior[10];
	process queue_myOwn[10];
	process total_process[50]; // 전체 process 입력받을 배열

	//total_proc개의 process 입력받기

	int sjf_cnt = 0; // 각 queue에 몇번째 index에 들어가야 하는지 알기 위해 생성한 변수.
	int rr8_cnt = 0;
	int rr20_cnt = 0;
	int prior_cnt = 0;
	int myOwn_cnt = 0;


	for (int i = 0; i < total_proc; i++)
	{
		// class number, priority, cpu burst 입력.
		// process id는 자동으로 생성(ex : 첫번째로 입력받은 process의 id -> 1)
		scanf("%d %d %d", &total_process[i].class_n, &total_process[i].prior, &total_process[i].cpu_burst);
		total_process[i].process_id = i + 1; // id는 자동으로 생성(들어오는 process의 순서로 생성)
		total_process[i].exit_flag = 0; // 모두 수행 종료가 되지 않은 상태로 초기화.

		if (total_process[i].class_n == 1) // class number에 따라 다른 스케줄링을 하는 큐에 들어감.
		{
			queue_sjf[sjf_cnt] = total_process[i];
			sjf_cnt++;
		}
		else if (total_process[i].class_n == 2)
		{
			queue_rr8[rr8_cnt] = total_process[i];
			rr8_cnt++;
		}
		else if (total_process[i].class_n == 3)
		{
			queue_rr20[rr20_cnt] = total_process[i];
			rr20_cnt++;
		}
		else if (total_process[i].class_n == 4)
		{
			queue_prior[prior_cnt] = total_process[i];
			prior_cnt++;
		}
		else if (total_process[i].class_n == 5)
		{
			queue_myOwn[myOwn_cnt] = total_process[i];
			myOwn_cnt++;
		}
	}


	//인자로 넘길 구조체 생성

	//sjf
	arg_proc *arg_sjf;
	arg_sjf = (arg_proc *)malloc(sizeof(arg_proc));
	arg_sjf->proc = queue_sjf;
	arg_sjf->arr_n = sjf_cnt;
	int t = 0;
	for (int i = 0; i < sjf_cnt; i++)
	{
		t += queue_sjf[i].cpu_burst;
	}
	arg_sjf->total_t = t;

	//rr8
	arg_proc *arg_rr8;
	arg_rr8 = (arg_proc *)malloc(sizeof(arg_proc));
	arg_rr8->proc = queue_rr8;
	arg_rr8->arr_n = rr8_cnt;
	t = 0;
	for (int i = 0; i < rr8_cnt; i++)
	{
		t += queue_rr8[i].cpu_burst;
	}
	arg_rr8->total_t = t;

	//rr20
	arg_proc *arg_rr20;
	arg_rr20 = (arg_proc *)malloc(sizeof(arg_proc));
	arg_rr20->proc = queue_rr20;
	arg_rr20->arr_n = rr20_cnt;
	t = 0;
	for (int i = 0; i < rr20_cnt; i++)
	{
		t += queue_rr20[i].cpu_burst;
	}
	arg_rr20->total_t = t;

	//priority
	arg_proc *arg_prior;
	arg_prior = (arg_proc *)malloc(sizeof(arg_proc));
	arg_prior->proc = queue_prior;
	arg_prior->arr_n = prior_cnt;
	t = 0;
	for (int i = 0; i < prior_cnt; i++)
	{
		t += queue_prior[i].cpu_burst;
	}
	arg_prior->total_t = t;

	//my own
	arg_proc *arg_myOwn;
	arg_myOwn = (arg_proc *)malloc(sizeof(arg_proc));
	arg_myOwn->proc = queue_myOwn;
	arg_myOwn->arr_n = myOwn_cnt;
	t = 0;
	for (int i = 0; i < myOwn_cnt; i++)
	{
		t += queue_myOwn[i].cpu_burst;
	}
	arg_myOwn->total_t = t;

	sem_init(&sem1, 0, 0); // semaphore 초기화
	sem_init(&sem2, 0, 0);
	sem_init(&sem3, 0, 0);

	pthread_t t_sjf; // 스레드 생성.
	pthread_t t_rr8;
	pthread_t t_rr20;
	pthread_t t_prior;
	pthread_t t_myOwn;

	pthread_create(&t_sjf, NULL, schedule_sjf, (void *)arg_sjf);
	pthread_create(&t_rr8, NULL, schedule_rr8, (void *)arg_rr8);
	pthread_create(&t_rr20, NULL, schedule_rr20, (void *)arg_rr20);
	pthread_create(&t_prior, NULL, schedule_priority, (void *)arg_prior);
	pthread_create(&t_myOwn, NULL, schedule_myOwn, (void *)arg_myOwn);

	pthread_join(t_sjf, NULL);
	pthread_join(t_rr8, NULL);
	pthread_join(t_rr20, NULL);
	pthread_join(t_prior, NULL);
	pthread_join(t_myOwn, NULL);

	free(arg_sjf);
	free(arg_rr8);
	free(arg_rr20);
	free(arg_prior); // 인자로 쓰기 위해 동적으로 할당했던 포인터 해제.
	free(arg_myOwn);

	sem_destroy(&sem1); // semaphore 관련 리소스 소멸.
	sem_destroy(&sem2);
	sem_destroy(&sem3);

	return 0;
}

void* schedule_sjf(void * arg_p) // shortest job first.
{
	printf("-----shortest job first scheduling-----\n");
	printf("---------------------------------------\n");
	arg_proc *arg = (arg_proc *)arg_p;
	int arr_n = arg->arr_n;
	int total_t = arg->total_t;
	process *proc = arg->proc;
	printf("number of process : %2d\n", arr_n);

	int min_execute_t = proc[0].cpu_burst; // 최소 cpu burst 담는 변수.
	int tmp_idx = 0; // 현재 보고 있는 process의 index.

	// 최소 cpu burst 갖는 process search.(idx 0부터)
	for (int i = 1; i < arr_n; i++)
	{
		if (min_execute_t > proc[i].cpu_burst)
		{
			min_execute_t = proc[i].cpu_burst;
			tmp_idx = i;
		}
	}
	int execute_cnt = 0; // 해당 process의 cpu burst를 얼마만큼 수행했는지.
	for (int i = 0; i < total_t; i++)
	{
		proc[tmp_idx].cpu_burst--; // 한번 수행했으므로 남은 cpu burst 1 감소.
		min_execute_t--; // 최소 cpu burst 갱신.
		execute_cnt++; // 수행한 cpu burst 1 증가.
		if (proc[tmp_idx].cpu_burst == 0) // 만약 실행 다 끝났다면(남은 cpu burst가 0이라면)
		{
			printf("[process %2d] burst time : %3d\n", proc[tmp_idx].process_id, execute_cnt); // 정보 출력
			execute_cnt = 0; // 초기화
			proc[tmp_idx].exit_flag = 1; // process 종료.
			// 다음에 실행할 최소 cpu burst 갖는 process search.
			min_execute_t = 10000; // 초기화 (cpu burst 최대 10000 가진다)
			tmp_idx = -1;
			for (int j = 0; j < arr_n; j++)
			{
				if (proc[j].exit_flag == 0)
				{
					if (proc[j].cpu_burst < min_execute_t)
					{
						tmp_idx = j;
						min_execute_t = proc[j].cpu_burst;
					}
				}
			}
		}

		// 모든 process가 0에 도착한다고 가정하므로 실행 중 새로 들어오는 process가 없다.
		// 그러므로 현재 실행되는 process가 종료될 때까지 계속 최소 cpu burst를 갖는다. 

	}
	printf("\n\n");
	sem_post(&sem1);
}


void* schedule_rr8(void *arg_p) // time quantum : 8
{
	sem_wait(&sem1);

	printf("-----round-robin scheduling 8-----\n");
	printf("----------------------------------\n");
	arg_proc *arg = (arg_proc *)arg_p;
	int arr_n = arg->arr_n;
	int total_t = arg->total_t;
	process *proc = arg->proc;

	printf("number of process : %2d\n", arr_n);

	int tmp_idx = 0; // 현재 보고 있는 process의 index.
	int i = 0; // 총 얼마만큼의 cpu burst 수행했는지.

	while (i < total_t) // 총 cpu burst만큼 수행하면 종료.
	{
		for (int j = 0; j < 8; j++) // time quantum : 8
		{

			proc[tmp_idx].cpu_burst--; // 한번 수행했으므로 남은 cpu burst 1 감소.
			i++; // 총 cpu burst 수행 시간 1 증가.

			if (proc[tmp_idx].cpu_burst == 0) // 만약 실행 다 끝났다면(남은 cpu burst가 0이라면)
			{
				proc[tmp_idx].exit_flag = 1; // process 종료.
				printf("[process %2d] burst time - 기존 : %3d, 실행 : %3d, 나머지 : %3d\n", proc[tmp_idx].process_id, j + 1 + proc[tmp_idx].cpu_burst, j + 1, proc[tmp_idx].cpu_burst);
				break;
			}

		}
		if (proc[tmp_idx].exit_flag == 0) // 실행 후 남은 cpu burst 있다면.
		{
			printf("[process %2d] burst time - 기존 : %3d, 실행 : %3d, 나머지 : %3d\n", proc[tmp_idx].process_id, 8 + proc[tmp_idx].cpu_burst, 8, proc[tmp_idx].cpu_burst);
		}


		// 큐에 있던 process 전부 실행했을 경우 나간다.
		if (i == total_t) break;
		// 다음 index 중 종료되지 않은 process search.
		int find_cnt = (tmp_idx + 1) % arr_n;
		while (1)
		{
			if (proc[find_cnt].exit_flag == 0) // 만약 종료되지 않았다면.
			{
				tmp_idx = find_cnt;
				break;
			}
			find_cnt = (find_cnt + 1) % arr_n; // 다음 index로 넘어감. (순환 배열)
		}
	}
	printf("\n\n");
	sem_post(&sem2);
}

void* schedule_rr20(void *arg_p) // time quantum : 20
{
	sem_wait(&sem2);
	printf("-----round-robin scheduling 20-----\n");
	printf("-----------------------------------\n");
	arg_proc *arg = (arg_proc *)arg_p;
	int arr_n = arg->arr_n;
	int total_t = arg->total_t;
	process *proc = arg->proc;

	printf("number of process : %2d\n", arr_n);

	int tmp_idx = 0; // 현재 보고 있는 process의 index.
	int i = 0; // 총 얼마만큼의 cpu burst 수행했는지.

	while (i < total_t) // 총 cpu burst만큼 수행하면 종료.
	{
		for (int j = 0; j < 20; j++) // time quantum : 20
		{

			proc[tmp_idx].cpu_burst--; // 한번 수행했으므로 남은 cpu burst 1 감소.
			i++; // 총 cpu burst 수행 시간 1 증가.

			if (proc[tmp_idx].cpu_burst == 0) // 만약 실행 다 끝났다면(남은 cpu burst가 0이라면)
			{
				proc[tmp_idx].exit_flag = 1; // process 종료.
				printf("[process %2d] burst time - 기존 : %3d, 실행 : %3d, 나머지 : %3d\n", proc[tmp_idx].process_id, j + 1 + proc[tmp_idx].cpu_burst, j + 1, proc[tmp_idx].cpu_burst);
				break;
			}

		}
		if (proc[tmp_idx].exit_flag == 0) // 실행 후 남은 cpu burst 있다면.
		{
			printf("[process %2d] burst time - 기존 : %3d, 실행 : %3d, 나머지 : %3d\n", proc[tmp_idx].process_id, 20 + proc[tmp_idx].cpu_burst, 20, proc[tmp_idx].cpu_burst);
		}


		// 큐에 있던 process 전부 실행했을 경우 나간다.
		if (i == total_t) break;
		// 다음 index 중 종료되지 않은 process search.
		int find_cnt = (tmp_idx + 1) % arr_n;
		while (1)
		{
			if (proc[find_cnt].exit_flag == 0) // 만약 종료되지 않았다면.
			{
				tmp_idx = find_cnt;
				break;
			}
			find_cnt = (find_cnt + 1) % arr_n; // 다음 index로 넘어감. (순환 배열)
		}
	}
	printf("\n\n");
	sem_post(&sem1);
	sem_post(&sem2);
}

void* schedule_priority(void *arg_p) // 우선순위 기반 스케줄링, 비선점 구조
{
	sem_wait(&sem1);
	sem_wait(&sem2);
	printf("-----priority scheduling-----\n");
	printf("-----------------------------\n");
	arg_proc *arg = (arg_proc *)arg_p;
	int arr_n = arg->arr_n;
	int total_t = arg->total_t;
	process *proc = arg->proc;

	printf("number of process : %2d\n", arr_n);

	int i = 0; // 총 얼마만큼의 cpu burst 수행했는지.
	int min_priority = 1000; // 가장 높은 우선순위 담는 변수. (최대 우선순위 1000, 작을수록 우선순위 높다)
	int tmp_idx = -1; // 현재 보고 있는 process의 index.
	while (i < total_t)
	{
		//최소 우선순위 가지는 process search.
		for (int j = 0; j < arr_n; j++)
		{
			if (proc[j].exit_flag == 1) continue; // 만약 종료된 process라면 continue.
			if (proc[j].prior < min_priority) // 우선순위 갱신.
			{
				tmp_idx = j;
				min_priority = proc[j].prior;
			}
		}
		int t = proc[tmp_idx].cpu_burst;
		int execute_cnt = 0; // 해당 process가 얼마만큼의 cpu burst를 수행했는지.
		for (int j = 0; j < t; j++) // 해당 process의 cpu burst만큼 실행.
		{
			proc[tmp_idx].cpu_burst--; // 한번 수행했으므로 남은 cpu burst 1 감소.
			i++; // 총 cpu burst 수행 시간 1 증가.
			execute_cnt++; // 수행한 cpu burst 1 증가.
		}
		proc[tmp_idx].exit_flag = 1; // process 종료
		printf("[process %2d] priority : %2d, burst time : %3d\n", proc[tmp_idx].process_id, proc[tmp_idx].prior, execute_cnt);

		min_priority = 1000; // 초기화
		tmp_idx = -1;
	}
	printf("\n\n");
	sem_post(&sem3);
}

// 나만의 스케줄러 구현
// round-robin + priority
// 우선순위가 높은 순서대로 수행한다.
// 한 time quantum 수행하고 해당 process의 우선순위를 1 증가시킨다. (starvation 방지하기 위해)
// time quantum : 15
void* schedule_myOwn(void *arg_p)
{
	sem_wait(&sem3);
	printf("-----my own scheduling-----\n");
	printf("---------------------------\n");

	arg_proc *arg = (arg_proc *)arg_p;
	int arr_n = arg->arr_n;
	int total_t = arg->total_t;
	process *proc = arg->proc;

	printf("number of process : %2d\n", arr_n);

	int tmp_idx = 0; // 현재 보고 있는 process의 index.
	int i = 0; // 총 얼마만큼의 cpu burst 수행했는지.

	while (i < total_t) // 총 cpu burst만큼 수행하면 종료.
	{
		//가장 높은 priority 갖는 process search.

		int min_prior = 1000; // 최소 priority 담는 변수. (최대 priority 1000)
		tmp_idx = -1; // 초기화.
		for (int j = 0; j < arr_n; j++)
		{
			if (proc[j].exit_flag == 0) // 종료된 process가 아닐 때
			{
				if (min_prior > proc[j].prior)
				{
					min_prior = proc[j].prior; // 가장 높은 priority 갱신.
					tmp_idx = j;
				}
			}
		}

		for (int j = 0; j < 15; j++) // time quantum : 15
		{

			proc[tmp_idx].cpu_burst--; // 한번 수행했으므로 남은 cpu burst 1 감소.
			i++; // 총 cpu burst 수행 시간 1 증가.

			if (proc[tmp_idx].cpu_burst == 0) // 만약 실행 다 끝났다면(남은 cpu burst가 0이라면)
			{
				proc[tmp_idx].exit_flag = 1; // process 종료.
				printf("[process %2d] priority : %2d | burst time - 기존 : %3d, 실행 : %3d, 나머지 : %3d\n", proc[tmp_idx].process_id, proc[tmp_idx].prior,
					j + 1 + proc[tmp_idx].cpu_burst, j + 1, proc[tmp_idx].cpu_burst);
				break;
			}

		}
		if (proc[tmp_idx].exit_flag == 0) // 실행 후 남은 cpu burst 있다면.
		{
			printf("[process %2d] priority : %2d | burst time - 기존 : %3d, 실행 : %3d, 나머지 : %3d\n", proc[tmp_idx].process_id, proc[tmp_idx].prior,
				15 + proc[tmp_idx].cpu_burst, 15, proc[tmp_idx].cpu_burst);
			proc[tmp_idx].prior++; // 실행 끝나고 우선순위 1 증가시킴.

		}


		// 큐에 있던 process 전부 실행했을 경우 나간다.
		if (i == total_t) break;
	}
	printf("\n\n");

}