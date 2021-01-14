//Self-clocked Fair Queueing算法实现
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
	double packLen;
	double timestamp;

	int flowID;

	double arrive_time;
	double remainTtransmitTime;
	bool operator<(const pack &b)const
	{
		if (timestamp == b.timestamp)
		{
			return arrive_time > b.arrive_time;
		}

		return timestamp > b.timestamp;
	}
};

priority_queue<pack>SCFQ_queue;

double lambda[3];
double mu;

double end_time = 1e8;

//queue arrive:0 1 2 wfq depature:3 ffs depature:4
int next_event_type;

double time_next_event[4];
int num_events = 4;

double all_wait_time[3];
double wait_gap = 1;

const int wait_x_max = 1000;
const int length_x_max = 100;

double queue_lenth[3][length_x_max];
double queue_wait[3][wait_x_max];

double last_arrive_or_depart_time = 0;

int num_of_come[3];
int num_of_sent[3];

double w[3];

double sim_time = 0, virtual_time = 0, pre_sim_time = 0;

double lastTimeStamp[3];

int real_time_queue_length[3];
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

		time_next_event[i]=expon(lambda[i]);

		lastTimeStamp[i] = 0;

		real_time_queue_length[i] = 0;
	}
	time_next_event[3] = DBL_MAX;
}
void update_process()
{
	if (!SCFQ_queue.empty())
	{
		pack temp=SCFQ_queue.top();
		SCFQ_queue.pop();
		temp.remainTtransmitTime -= (sim_time - pre_sim_time);
		SCFQ_queue.push(temp);
	}
}
void timing()
{
	pre_sim_time = sim_time;

	double scfq_departure = 0;

	if (!SCFQ_queue.empty())
		scfq_departure = sim_time + SCFQ_queue.top().remainTtransmitTime;
	else
		scfq_departure = DBL_MAX;

	time_next_event[3] = scfq_departure;


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
	update_process();
}

double S(double t)
{
	double r = 0.0;

	for (int i = 0; i < 3; i++)
		if ( real_time_queue_length[i] > 0 )
			r += w[i];

	if ( r > 0 )
		return(1/r);
	else
		return(0.0);
}

void Arrive()
{
	if(!SCFQ_queue.empty())
		virtual_time = SCFQ_queue.top().timestamp;

	for (int i = 0; i < 3; i++)
	{
		queue_lenth[i][real_time_queue_length[i]] += sim_time - last_arrive_or_depart_time;
	}
	last_arrive_or_depart_time = sim_time;

	num_of_come[next_event_type]++;
	real_time_queue_length[next_event_type]++;

	pack pack_arrive;
	pack_arrive.packLen = expon(mu);
	pack_arrive.timestamp = max(virtual_time, lastTimeStamp[next_event_type]) + pack_arrive.packLen/w[next_event_type];
	pack_arrive.flowID = next_event_type;
	pack_arrive.arrive_time = sim_time;
	pack_arrive.remainTtransmitTime = pack_arrive.packLen;

	SCFQ_queue.push(pack_arrive);

	lastTimeStamp[next_event_type] = pack_arrive.timestamp;

	time_next_event[next_event_type] = sim_time + expon(lambda[next_event_type]);
}
void SCFQ_Depart()
{
	int queue_id = SCFQ_queue.top().flowID;

	double temp_wait_time = sim_time - SCFQ_queue.top().arrive_time-SCFQ_queue.top().packLen;
	all_wait_time[queue_id] += temp_wait_time;

	queue_wait[queue_id][int(temp_wait_time / wait_gap)]++;

	num_of_sent[queue_id]++;

	for (int i = 0; i < 3; i++)
	{
		queue_lenth[i][real_time_queue_length[i]] += sim_time - last_arrive_or_depart_time;
	}

	last_arrive_or_depart_time = sim_time;

	real_time_queue_length[queue_id]--;

	SCFQ_queue.pop();

	time_next_event[3] = DBL_MAX;
}
int main()
{
	cout << "input lambda 0 1 2" << endl;

	for (int i = 0; i < 3; i++)
		cin >> lambda[i];

	cout << "input mu" << endl;

	cin >> mu;

	cout << "输入队列权重 w 0 1 2的分母(如输入3会转化为1/3):" << endl;

	double a;
	for (int i = 0; i < 3; i++)
	{
		cin >> a;
		w[i] = 1 / a;
	}
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
			SCFQ_Depart();
			break;
		}
	}
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
