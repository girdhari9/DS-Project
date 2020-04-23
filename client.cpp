// How to start client
// g++ client.cpp -pthread
// ./a.out processid(starting from 1) server port

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
using namespace std;
vector<int> v(3,0);
int processid;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//datastructure of holdback_queue
struct holdmessage
{
	int sender; //,
	int p1;     //@
	int p2;		//#
	int p3;		//$
	string message; //!
};

holdmessage make_Pair(int s0,int s1,int s2,int s3,const string& s)
{
    holdmessage ret;
    ret.sender = s0;
    ret.p1 = s1;
    ret.p2 = s2;
    ret.p3 = s3;
    ret.message = s;
    return ret;
}

std::vector<holdmessage> holdback_queue;

void *holdbackmg2(void *xx)
{
		int xxx = *((int*)xx);
		// memset(msg,'\0',sizeof(msg));
		int xxxx=50;
		while(xxxx>0&&holdback_queue.size()>0)
		{
			for(int i=0;i<holdback_queue.size();i++)
			{
				// cout<<endl;
				// cout<<"inholdbackloop"<<endl;
				int processid_recv=holdback_queue[i].sender;
				int p1=holdback_queue[i].p1;
				int p2=holdback_queue[i].p2;
				int p3=holdback_queue[i].p3;;
				string message=holdback_queue[i].message;
				vector<int> vvv;
			    vvv.push_back(p1);
			    vvv.push_back(p2);
			    vvv.push_back(p3);
			    int current_proc_clock_value=v[processid_recv-1];
				if((current_proc_clock_value+1)==vvv[processid_recv-1])
				{
					int s=0;
					for(int j=0;j<3;j++)
					{
						if(j==processid_recv-1)
						{
							continue;
						}
						if(v[j]>=vvv[j])
						{
							s++;
						}
					}
					// cout<<s<<endl;
					if(s==2)
					{
						int ppp=message.size()-1;
						char pp=message[ppp];
						if(pp=='\n')
						{
							cout<<"From process holdback_queue (received from process "<<processid_recv<<")-> "<<message;
						}
						else
						{
							cout<<"From process holdback_queue (received from process "<<processid_recv<<")-> "<<message<<endl;
						}
						pthread_mutex_lock(&mutex);
						for(int j=0;j<3;j++)
						{
							v[j]=max(vvv[j],v[j]);
						}
						pthread_mutex_unlock(&mutex);
						holdback_queue.erase(holdback_queue.begin()+i);
					}
				}

			}
			xxxx=xxxx-1;
		}

}

void *msghandler(void *line2)
{

		string line = *((string*)line2);
		using namespace std::this_thread; // sleep_for, sleep_until
    	using namespace std::chrono; 
	    int num = (rand()%(10 - 1 + 1)) + 1; 
    	sleep_for(seconds(num));

    	
		size_t first = line.find(':');
		int processid_recv=stoi(line.substr(0,first));
	    size_t second = line.find('#');
	    int p1=stoi(line.substr(first+1,second-first-1));
	    size_t third = line.find('/');
	    int p2=stoi(line.substr(second+1,third-second-1));
	    size_t fourth = line.find('$');
	    int p3=stoi(line.substr(third+1,fourth-third-1));
	    string message=line.substr(fourth+1);
	    vector<int> vv;
	    vv.push_back(p1);
	    vv.push_back(p2);
	    vv.push_back(p3);

	    // cout<<"sender info.->"<<p1<<p2<<p3<<endl;

		int current_proc_clock_value=v[processid_recv-1];

		if((current_proc_clock_value+1)==vv[processid_recv-1])
		{
			int s=0;
			for(int i=0;i<3;i++)
			{
				if(i==processid_recv-1)
				{
					continue;
				}
				if(v[i]>=vv[i])
				{
					s++;
				}
			}
			// cout<<s<<endl;
			if(s==2)
			{
				cout<<"From process "<<processid_recv<<"-> "<<message;
				pthread_mutex_lock(&mutex);
				for(int i=0;i<3;i++)
				{
					v[i]=max(vv[i],v[i]);
				}
				pthread_mutex_unlock(&mutex);
				
			}
			else
			{
				//hold back queue
				// cout<<"s!=2"<<endl;
				holdback_queue.push_back(make_Pair(processid_recv,p1,p2,p3,message));
			}
		}
		else
		{
			//hold back queue
			// cout<<"current_proc_clock_value+1!=vv[processid_recv-1] for message ->"<<message;
			cout<<"In holdback-queue-> "<<message;
			holdback_queue.push_back(make_Pair(processid_recv,p1,p2,p3,message));
			
		}

		pthread_t holdbackvt;
		int xx=1;
		pthread_create(&holdbackvt,NULL,holdbackmg2,&xx);
		pthread_join(holdbackvt,NULL);
		// cout<<"current process updated info. -> "<<v[0]<<v[1]<<v[2]<<endl;
		// cout<<endl;

}

void *recvmg(void *sock)
{
	int their_sock = *((int*)sock);
	char msg[500];
	int len;
	pthread_t recmsg;
	while((len = recv(their_sock,msg,500,0)) > 0) 
	{
		string line=(string)msg; 
		pthread_create(&recmsg,NULL,msghandler,&line);
		// pthread_join(recmsg,NULL);
		memset(msg,'\0',sizeof(msg));
	}
}
// ./a.out server_port_number process_number
int main(int argc, char *argv[])
{
	struct sockaddr_in their_addr;
	int my_sock;
	int their_sock;
	int their_addr_size;
	int portno;
	pthread_t sendt,recvt;
	char msg[500];
	

	string res="";
	char ip[INET_ADDRSTRLEN];
	int len;
	if(argc > 3) {
		printf("too many arguments");
		exit(1);
	}
	portno = atoi(argv[2]);
	processid = atoi(argv[1]);
	// strcpy(processname,argv[1]);
	my_sock = socket(AF_INET,SOCK_STREAM,0);
	memset(their_addr.sin_zero,'\0',sizeof(their_addr.sin_zero));
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(portno);
	their_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(my_sock,(struct sockaddr *)&their_addr,sizeof(their_addr)) < 0) {
		perror("connection not esatablished");
		exit(1);
	}
	inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);
	printf("connected to %s, start chatting\n",ip);

	//sending the processid so that in server updation can be done.
	string sharingid=to_string(processid);
	len = write(my_sock,sharingid.c_str(),sharingid.size());
	if(len < 0) {
			perror("message not sent");
			exit(1);
	}

	//HANDLING FAULT TOLERANCE
	//FORMAT FOR STORING VECTOR CLOCK X,X$X

	string pid2=to_string(processid);
	string filename=pid2+".txt";
	ifstream  stream2(filename); 
	string line2;  
	if(stream2.is_open())
	{
		while (getline(stream2, line2))
		  {
		        size_t first = line2.find(',');
		        size_t second = line2.find('$');
		        size_t third = line2.find('#');
		        string p11=line2.substr(0,first);
		        string p22=line2.substr(first+1,second-first-1);
		        string p33=line2.substr(second+1,third-second-1);
		        v[0]=stoi(p11);
		        v[1]=stoi(p22);
		        v[2]=stoi(p33);
		  }
		  stream2.close();
	} 
	// cout<<v[0]<<v[1]<<v[2]<<endl;
  	// remove(seederlist.c_str());


	string filename2=pid2+"_holdback.txt";
	ifstream  stream3(filename2); 
	if(stream3.is_open())
	{
		while (getline(stream3,line2))
		  {
		        size_t first = line2.find(',');
		        size_t second = line2.find('@');
		        size_t third = line2.find('#');
		        size_t fourth = line2.find('$');
		        size_t fifth = line2.find('!');
		        int sendu=stoi(line2.substr(0,first));
		        string e11=line2.substr(first+1,second-first-1);
		        string e22=line2.substr(second+1,third-second-1);
		        string e33=line2.substr(third+1,fourth-third-1);
		        string mesgu=line2.substr(fourth+1,fifth-fourth-1);
		        int f1=stoi(e11);
		        int f2=stoi(e22);
		        int f3=stoi(e33);
		        holdback_queue.push_back(make_Pair(sendu,f1,f2,f3,mesgu));
		  }
		  stream2.close();

		  pthread_t holdbackvt;
		  int xx=1;
		  pthread_create(&holdbackvt,NULL,holdbackmg2,&xx);
	} 
	// pthread_t holdbackvt;
 //    int xx=1;
 // //    pthread_create(&holdbackvt,NULL,holdbackmg2,&xx);
	// for(int i=0;i<holdback_queue.size();i++)
	// {
	// 	cout<<holdback_queue[i].sender<<" "<<holdback_queue[i].p1<<holdback_queue[i].p2<<holdback_queue[i].p3<<" "<<holdback_queue[i].message<<endl;
	// }

	pthread_create(&recvt,NULL,recvmg,&my_sock);
	cout<<endl;
	cout<<"For broadcasting the message simply type it:-"<<endl<<endl;
	while(fgets(msg,500,stdin) > 0) 
	{
		// printf("%s\n",msg);
		string pid=to_string(processid);
		string msg2=(string) msg;
		string msg3=msg2.substr(0,4);
		// cout<<msg3<<" -->"<<endl;
		if(msg3.compare("exit")==0)
		{
			//dumping the vector clock
			ofstream temp;
			temp.open(filename);
			string x1=to_string(v[0]);
			string x2=to_string(v[1]);
			string x3=to_string(v[2]);
			string vclock=x1+","+x2+"$"+x3+"#";
			temp<<vclock;
			temp.close();


			//dumping the holdback queue
			ofstream temp2;
			temp2.open(filename2);
			for(int i=0;i<holdback_queue.size();i++)
			{
				string sender1=to_string(holdback_queue[i].sender);
				string e1=to_string(holdback_queue[i].p1);
				string e2=to_string(holdback_queue[i].p2);
				string e3=to_string(holdback_queue[i].p3);
				string message1=holdback_queue[i].message;
				message1=message1.substr(0,message1.size()-1);
				string l2=sender1+","+e1+"@"+e2+"#"+e3+"$"+message1+"!";
				temp2<<l2<<"\n";
				// temp2<<"cool"<<"\n";
			}
			temp2.close();
			exit(1);

		}

		res=res+pid;
		res=res+":";
		// strcat(res,":");
		v[processid-1]++;
		// string msg2=(string) msg;
		res=res+to_string(v[0])+"#"+to_string(v[1])+"/"+to_string(v[2])+"$"+msg2;
		len = write(my_sock,res.c_str(),res.size());
		if(len < 0) {
			perror("message not sent");
			exit(1);
		}
		memset(msg,'\0',sizeof(msg));
		res="";
		// cout<<"messgae to broadcast---->";
	}
	pthread_join(recvt,NULL);
	close(my_sock);

}