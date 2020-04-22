// #include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <random>
#include <chrono>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
#include <string>
#include<iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
#include <string>
using namespace std;
struct client_info {
	int sockno;
	char ip[INET_ADDRSTRLEN];
};
struct holdmessage
{
	int sender; //,
	int p1;     //@
	int p2;		//#
	int p3;		//$
	string message; //!
};
std::vector<holdmessage> holdback_queue[3];

int clients[100];
int n = 0;
vector<int> port_process_mapping(3,0);
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void sendtoall(string s,int curr)
{
	int i;
	pthread_mutex_lock(&mutex);
	for(i = 0; i < n; i++) {
		if(clients[i] != curr) {
			if(send(clients[i],s.c_str(),s.size(),0) < 0) {
				perror("sending failure");
				continue;
			}
		}
	}
	pthread_mutex_unlock(&mutex);
}
void *recvmg(void *sock)
{
	struct client_info cl = *((struct client_info *)sock);
	char msg[500];
	int len;
	int i;
	int j;

	//updating
	char msg2[2]; 
	int len2;
	int pid=-1;
	if((len2 = recv(cl.sockno,msg2,2,0))>0)
	{
		// string x=(string)msg2;
		pid=stoi(msg2);
		port_process_mapping[pid-1]=cl.sockno;
		// cout<<msg<<" "<<cl.sockno<<" "<<pid<<endl;
		printf("%d %d %d\n",pid,cl.sockno,pid);
		memset(msg2,'\0',sizeof(msg));
	}
	fflush(stdout);

	while((len = recv(cl.sockno,msg,500,0)) > 0) 
	{
		msg[len] = '\0';
		printf("%s--->\n",msg);
		sendtoall(msg,cl.sockno);
		for(int i=1;i<=3;i++)
		{
			if(i!=pid)
			{
				printf("chill\n");
				string line = (string)msg;
				size_t first = line.find(':');
				int processid_recv=stoi(line.substr(0,first));
			    size_t second = line.find('#');
			    int p1=stoi(line.substr(first+1,second-first-1));
			    size_t third = line.find('/');
			    int p2=stoi(line.substr(second+1,third-second-1));
			    size_t fourth = line.find('$');
			    int p3=stoi(line.substr(third+1,fourth-third-1));
			    string message=line.substr(fourth+1);
			    // holdback_queue[i-1].push_back(make_Pair(processid_recv,p1,p2,p3,message));
			    fflush(stdout);
			    string p=to_string(i);
			    string filename2=p+"_holdback.txt";
			    printf("%s\n",filename2.c_str());
				ofstream temp2;
				temp2.open(filename2.c_str(),std::ofstream::out | std::ofstream::app);
				string sender1=to_string(processid_recv);
				string e1=to_string(p1);
				string e2=to_string(p2);
				string e3=to_string(p3);
				message=message.substr(0,message.size()-1);
				string l2=sender1+","+e1+"@"+e2+"#"+e3+"$"+message+"!";
				printf("%s\n",l2.c_str());
				temp2<<l2<<"\n";
				temp2.close();
			}
		}
		memset(msg,'\0',sizeof(msg));
	}

	pthread_mutex_lock(&mutex);
	printf("%s disconnected\n",cl.ip);
	for(i = 0; i < n; i++) {
		if(clients[i] == cl.sockno) {
			j = i;
			while(j < n-1) {
				clients[j] = clients[j+1];
				j++;
			}
		}
	}
	n--;
	pthread_mutex_unlock(&mutex);
}
int main(int argc,char *argv[])
{
	struct sockaddr_in my_addr,their_addr;
	int my_sock;
	int their_sock;
	socklen_t their_addr_size;
	int portno;
	pthread_t sendt,recvt;
	char msg[500];
	int len;
	struct client_info cl;
	char ip[INET_ADDRSTRLEN];;
	;
	if(argc > 2) {
		printf("too many arguments");
		exit(1);
	}
	portno = atoi(argv[1]);
	my_sock = socket(AF_INET,SOCK_STREAM,0);
	memset(my_addr.sin_zero,'\0',sizeof(my_addr.sin_zero));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(portno);
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	their_addr_size = sizeof(their_addr);

	if(bind(my_sock,(struct sockaddr *)&my_addr,sizeof(my_addr)) != 0) {
		perror("binding unsuccessful");
		exit(1);
	}

	if(listen(my_sock,5) != 0) {
		perror("listening unsuccessful");
		exit(1);
	}

	while(1) {
		if((their_sock = accept(my_sock,(struct sockaddr *)&their_addr,&their_addr_size)) < 0) {
			perror("accept unsuccessful");
			exit(1);
		}
		pthread_mutex_lock(&mutex);
		inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);
		printf("%s connected\n",ip);
		cl.sockno = their_sock;
		strcpy(cl.ip,ip);
		clients[n] = their_sock;
		n++;
		pthread_create(&recvt,NULL,recvmg,&cl);
		pthread_mutex_unlock(&mutex);
	}
	return 0;
}