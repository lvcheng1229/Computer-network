//Deficit Round Robin算法实现
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
	double wait_time;
	pack(double s, double wait_t=0)
	{
		size = s;  wait_time = wait_t;
	}
	void add_wait_time(double t)
	{
		wait_time += t;
	}
};
vector<queue<pack>>m_queues;

double lambda[3];
double mu;

double sim_time = 0;
double end_time = 1e7;

//queue arrive:0 1 2 depature:3
int next_event_type;

double time_next_event[4];
int num_events = 4;

double all_wait_time[3];
double wait_gap = 1;

const int wait_x_max = 1000;
const int length_x_max = 100;

double queue_lenth[3][length_x_max];
double queue_wait[3][wait_x_max];

double last_event_time = 0;

int num_of_come[3];
int num_of_sent[3];

double quantum[3];

double deficit_counter[3];

bool isServerBusy = false;

int now_queue = 0;
int last_queue = 5;

double mina = 0;
void init()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < wait_x_max; j++)
		{
			queue_wait[i][j] = 0;
		}
		for (int j = 0; j < length_x_max; j++)
		{
			queue_lenth[i][j] = 0;
		}

		all_wait_time[i] = 0;
		num_of_come[i] = 0;
		num_of_sent[i] = 0;

		deficit_counter[i] = 0;

		time_next_event[i]=expon(lambda[i]);

		queue<pack> q_temp;
		m_queues.push_back(q_temp);
	}
	time_next_event[3] = DBL_MAX;
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
	}
	sim_time = min_time_next_event;
}

void switch_queue()
{
	bool isEmpty = true;//是否全部为空队列
	for (int i = 0; i < 3; i++)
	{
		if (!m_queues[i].empty())
		{
			isEmpty = false;
			break;
		}
	}

	if (isEmpty)
		return;

	while (!isServerBusy)
	{
		if (now_queue != last_queue)
		{
			deficit_counter[now_queue] += quantum[now_queue];
		}

		last_queue = now_queue;
		
		if (!m_queues[now_queue].empty())
		{
			double pack_size = m_queues[now_queue].front().size;
			if (pack_size > mina)
			{
				mina = pack_size;
			}
			if (pack_size <= deficit_counter[now_queue])
			{
				deficit_counter[now_queue] -= pack_size;

				double wait_time = m_queues[now_queue].front().wait_time;

				queue_wait[now_queue][int(wait_time / wait_gap)]++;

				all_wait_time[now_queue] += wait_time;

				time_next_event[3] = sim_time + pack_size;

				isServerBusy = true;

				num_of_sent[now_queue]++;

				return;
			}
		}
		else
		{
			deficit_counter[now_queue] = 0;
		}

		now_queue++;
		now_queue = now_queue % 3;

	}

}
void Arrive()
{
	
	for (int i = 0; i < 3; i++)
	{
		int queue_size = m_queues[i].size();
		
		for (int j = 0; j < queue_size; j++)
		{
			pack pack_temp = m_queues[i].front();
			
			m_queues[i].pop();
			
			pack_temp.add_wait_time(sim_time - last_event_time);
			
			m_queues[i].push(pack_temp);
		}
		
		queue_lenth[i][m_queues[i].size()] += sim_time - last_event_time;
	}
	
	last_event_time = sim_time;
	
	num_of_come[next_event_type]++;

	m_queues[next_event_type].push(pack(expon(mu)));

	time_next_event[next_event_type] = sim_time + expon(lambda[next_event_type]);

	switch_queue();

}
void Depart()
{
	for (int i = 0; i < 3; i++)
	{
		int queue_size = m_queues[i].size();

		for (int j = 0; j < queue_size; j++)
		{
			pack pack_temp = m_queues[i].front();

			m_queues[i].pop();

			pack_temp.add_wait_time(sim_time - last_event_time);

			m_queues[i].push(pack_temp);
		}

		queue_lenth[i][m_queues[i].size()] += sim_time - last_event_time;
	}

	m_queues[now_queue].pop();

	last_event_time = sim_time;

	isServerBusy = false;

	time_next_event[3] = DBL_MAX;

	switch_queue();
}
int main()
{
	cout << "input lambda 0 1 2" << endl;
	
	cin >> lambda[0] >> lambda[1] >> lambda[2];
	
	cout << "input mu" << endl;

	cin >> mu;

	cout << endl << "input quantum 0 1 2" << endl;

	cin >> quantum[0]>> quantum[1]>> quantum[2];

	if (lambda[0] + lambda[1] + lambda[2] > mu*0.95)
	{
		cout << "wrong: lambda[0] + lambda[1] + lambda[2] > mu" << endl;
		cout << "please check Max-Min fairness" << endl;
	}
	cout << "running..." << endl;
	init();
	while (sim_time < end_time)
	{
		timing();

		switch (next_event_type)
		{
		case 0:
		case 1:
		case 2:
			Arrive();
			break;
		case 3:
			Depart();
			break;
		}
	}
	cout << mina << endl;
	ofstream wait_time_distribution;
	ofstream queue_length_distribution;


	for (int i = 0; i < 3; i++)
	{
		cout << "queue " << i << " :" << endl;

		cout << "num_of_sent:" << num_of_sent[i] << endl;

		cout << "mean wait time:" << all_wait_time[i] / num_of_come[i]<< endl;
		
		double all = 0;
		double all_time = 0;

		for (int j = 0; j < length_x_max; j++)
		{
			all += queue_lenth[i][j] * j;

			all_time += queue_lenth[i][j];
		}

		cout << "mean queue length:" << all / all_time<< endl;

		string file_name= "wait_time_distribution"+to_string(i)+".txt";;

		wait_time_distribution.open(file_name);
		
		for (int j = 0; j < wait_x_max; j++)
		{
			wait_time_distribution << queue_wait[i][j]/num_of_sent[i] << ' ';
		}
		cout << queue_wait[i][0] << endl;

		wait_time_distribution.close();

		file_name= "queue_length_distribution"+to_string(i)+".txt";

		queue_length_distribution.open(file_name);

		for (int j = 0; j < length_x_max; j++)
		{
			queue_length_distribution << queue_lenth[i][j] / all_time << ' ';
		}

		queue_length_distribution.close();
	}

	
	double res[3];
	cout << endl;
	for (int i = 0; i < 3; i++)
	{
		res[i] = num_of_sent[i] / lambda[i];
		cout << "w" << i << ": " << res[i] << endl;
	}

	double max_diff = -DBL_MAX;
	max_diff = (abs(res[0] - res[1]) > max_diff ? abs(res[0] - res[1]) : max_diff);
	max_diff = (abs(res[1] - res[2]) > max_diff ? abs(res[1] - res[2]) : max_diff);
	max_diff = (abs(res[2] - res[0]) > max_diff ? abs(res[2] - res[0]) : max_diff);
	cout << "max difference:" << max_diff << endl;
	cout << "max difference unit time:" << max_diff/end_time << endl;
}
