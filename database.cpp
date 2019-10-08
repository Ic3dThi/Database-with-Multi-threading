// assignment1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//Thi Tran
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
using namespace std;
static pthread_mutex_t bsem;
static pthread_cond_t one = PTHREAD_COND_INITIALIZER;
static pthread_cond_t two = PTHREAD_COND_INITIALIZER;
static int grp1Req;
static int grp2Req;
static int start;
static int finish;
static int waitGroup;
static int waitLock;
static int firstGroup;
static int numUser = 1;
static bool segregation = false;
static int arr[10] = { 0,0,0,0,0,0,0,0,0,0 };
struct User
{
	int group = 0;
	int arrival = 0;
	int id = 0;
	int pos = 0;
	int busy = 0;
};
void* access(void* blank)
{
	User* temp = (User*)blank;
	if ((temp->group == 1 || temp->group == 2) && (temp->pos <= 10 || temp->pos >= 1))
	{
		if (temp->arrival != 0)
			sleep(temp->arrival);
		if (temp->group == 1)
			grp1Req++;
		else
			grp2Req++;
		pthread_mutex_lock(&bsem);
		cout << "User " << temp->id << " from Group " << temp->group << " arrives to the DBMS" << endl;
		if (!segregation)
		{
			if (arr[temp->pos - 1] == 0 && temp->group == start)
			{
				arr[temp->pos - 1] = temp->id;
			}
			else
			{
				if (temp->group == start)
				{
					waitLock++;
					cout << "User " << temp->id << " is waiting: position " << temp->pos << " of the database is being used by user " << arr[temp->pos - 1] << endl;
					pthread_cond_wait(&one, &bsem);
				}
				else if (temp->group != start)
				{
					waitGroup++;
					cout << "User " << temp->id << " is waiting due to its group" << endl;
					pthread_cond_wait(&two, &bsem);
				}
			}
		}
		if(segregation)
		{
			if (arr[temp->pos - 1] == 0 && temp->group == start)
			{
				arr[temp->pos - 1] = temp->id;
			}
			else if (temp->group == start)
			{
				waitLock++;
				cout << "User " << temp->id << " is waiting: position " << temp->pos << " of the database is being used by user " << arr[temp->pos - 1] << endl;
				pthread_cond_wait(&one, &bsem);
			}
			else if (temp->group != start)
			{
				waitGroup++;
				cout << "User " << temp->id << " is waiting due to its group" << endl;
				pthread_cond_wait(&two, &bsem);
			}
		}
		cout << "User " << temp->id << " is accessing the position " << temp->pos << " of the the database for " << temp->busy << " second(s)" << endl;
		pthread_mutex_unlock(&bsem);
		sleep(temp->busy);
		pthread_mutex_lock(&bsem);
		cout << "User " << temp->id << " finished its execution" << endl;
		arr[temp->pos - 1] = 0;


		if (arr[temp->pos] == 0)
			pthread_cond_broadcast(&one);
		if (temp->group == start)
			firstGroup--;
		if (firstGroup <= 0 && !(segregation))
		{
			cout << "\nAll users from Group " << start << " finished their execution" << endl;
			cout << "The users from Group " << finish << " start their execution\n" << endl;
			segregation = true;
			start = finish;
			pthread_cond_broadcast(&two);
		}
		pthread_mutex_unlock(&bsem);
	}
}

int main(int argc, char* argv[])
{
	int arrivalCounter = 0;
	vector <User> userLine;
	string line = "";

	//	ifstream input("input.txt");
	getline(cin, line);
	//	getline(input, line);
	stringstream ss(line);
	ss >> start;
	if (start == 1)
		finish = 2;
	else if (start == 2)
		finish = 1;
	while (getline(cin, line))
		//while (getline(input, line))
	{
		stringstream sd(line);
		int tempGroup;
		int tempID;
		int tempPos;
		int tempBusy;
		int tempArrival;
		User tempNode;
		sd >> tempGroup;
		sd >> tempPos;
		sd >> tempArrival;
		sd >> tempBusy;
		arrivalCounter += tempArrival;
		tempNode.group = tempGroup;
		tempNode.pos = tempPos;
		tempNode.arrival = arrivalCounter;
		tempNode.busy = tempBusy;
		tempNode.id = numUser;
		//cout << tempNode.id << " " << tempNode.group << " " << tempNode.pos << " " << tempNode.arrival << " " << tempNode.busy << endl;
		numUser++;
		userLine.push_back(tempNode);
		if (tempGroup == start)
			firstGroup++;
	}
	pthread_t tid[numUser];
	pthread_mutex_init(&bsem, NULL);
	for (int i = 0; i < numUser; i++)
	{
		if (pthread_create(&tid[i], NULL, &access, userLine.data() + i))
		{
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}
	}
	// Wait for the other threads to finish.
	for (int i = 0; i < numUser; i++)
		pthread_join(tid[i], NULL);
	cout << "Total Requests:\n\tGroup 1: " << grp1Req << "\n\tGroup 2: " << grp2Req << endl;
	cout << "\nRequests that waited:\n\tDue to its group: " << waitGroup << "\n\tDue to a locked position: " << waitLock << endl;

}
