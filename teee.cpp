//MM1算法实现
//author：唐帅
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include<random>
#include<algorithm>
#include <string> 
#include<queue>
using namespace std;

uniform_real_distribution<float> randomFloats(0.0, 1.0);
default_random_engine generator;
double expon(double lambda)
{
	double u;
	do
	{
		u = randomFloats(generator);
	}
	while ((u == 0) || (u == 1)); //special cases that we want to avoid

	return -log(1-u)/lambda;
}

struct pack
{
	double size;
	double arrive_time;
	double leave_time;
	pack(double s, double a_t, double l_t)
	{
		size = s; arrive_time = a_t; leave_time = l_t;
	}
};

std::queue<pack>m_queue;

double lambda0, mu0;
double sim_time = 0;
double end_time = 1e8;

int next_event_type;

double time_next_event[2];

int num_events = 2;

double wait_time = 0;
double wait_gap = 1;

const int x_max = 1000;//横坐标数组大小

double q_lenth[x_max];
double q_wait[x_max];

double last_event_time = 0;
int num_of_come = 0;
void init()
{
	next_event_type = 0;
	time_next_event[0] = expon(lambda0);
	time_next_event[1] = time_next_event[0]+expon(mu0);

	m_queue.push(pack(1, time_next_event[0], time_next_event[1]));

	for (int i = 0; i < x_max; i++)
	{
		q_lenth[i] = 0;
		q_wait[i] = 0;
	}
}
void timing()
{
	double min_time_next_event = DBL_MAX;
	for (int i = 0; i < num_events; i++)
	{
		if (time_next_event[i] < min_time_next_event)
		{
			min_time_next_event = time_next_event[i];
			next_event_type = i;
		}
		sim_time = min_time_next_event;
	}
}
void switch_q()
{
	//单队列，无调度
}
void Arrive()
{
	if (m_queue.size() == 1)
		q_lenth[0] += sim_time - last_event_time;
	else 
		q_lenth[m_queue.size() - 2] += sim_time - last_event_time;

	last_event_time = sim_time;

	//此处注意是根据当前到达时间随机出下一个到达
	num_of_come++;
	time_next_event[next_event_type] = sim_time + expon(lambda0);
	m_queue.push(pack(1, time_next_event[next_event_type], DBL_MAX));
}
void Depart()
{
	q_lenth[m_queue.size()-2] += sim_time - last_event_time;
	last_event_time = sim_time;

	double time = m_queue.front().leave_time - m_queue.front().arrive_time;
	wait_time += time;
	q_wait[int(time / wait_gap)]+=1;
	m_queue.pop();

	//队列非空
	if (m_queue.front().arrive_time < time_next_event[next_event_type])
	{
		time_next_event[1] = time_next_event[1] + expon(mu0);
	}
	else//队列空
	{
		time_next_event[1] = m_queue.front().arrive_time + expon(mu0);
	}
	m_queue.front().leave_time = time_next_event[1];
}
int main()
{
	cout << "input lambda and mu" << endl;
	cin >> lambda0 >> mu0;
	
	cout << "运行中" << endl;
	
	init();
	
	while (sim_time < end_time)
	{
		timing();
		switch (next_event_type)
		{
		case 0:
			Arrive();
			break;
		case 1:
			Depart();
			break;
		}
	}

	double rho = lambda0 / mu0;

	cout << "等待时间理论值:" << 1 / (mu0 - lambda0) << endl;
	cout << "等待时间实际值:" << (wait_time / (float)num_of_come) << endl;

	double all = 0;
	double all_time = 0;

	for (int i = 0; i < x_max; i++)
	{
		all += q_lenth[i] * i;
		all_time += q_lenth[i];
	}

	cout << "队长理论值:" << all / all_time << endl;
	cout << "队长实际值:" <<(lambda0*lambda0) / (mu0*(mu0 - lambda0)) << endl;

	ofstream wait_time_distribution("wait_time_distribution.txt");
	ofstream queue_length_distribution("queue_length_distribution.txt");

	for (int j = 0; j < x_max; j++)
	{
		wait_time_distribution << q_wait[j]/num_of_come << ' ';
		queue_length_distribution << q_lenth[j] / all_time << ' ';
	}

	wait_time_distribution.close();
	queue_length_distribution.close();
}
