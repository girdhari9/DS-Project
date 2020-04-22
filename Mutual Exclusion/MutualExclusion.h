#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h> 

#define FOR0(i,n) for(ll i=0;i<n;i++)
#define FOR1(i,n) for(ll i=1;i<=n;i++)
#define FORl(i,l,n) for(ll i=l;i<n;i++)
#define ll long long
#define ld long double
#define endl '\n'
#define intmax 1e14
ll power(ll num,ll g,ll mod){
  if(g==0)return 1;
  if(g%2==1)return (num*power((num*num)%mod,g/2,mod))%mod;
  return power((num*num)%mod,g/2,mod);
}

// Message Structure
struct message{
    char *type;
    int  req;
    int  NODEID;
    int  *queue;
    int  lenqueue;
    int  *last;
    int  lenlast;
};


// function Declarations
void *bind_thread();
void *accept_thread(void *accept_sock);

#define perror2(s,e) fprintf(stderr, "%s:%s\n" , s, strerror(e))

// Global variables

//algo for nodes=3
#define NODES_NO  4
// max queue length
#define MAX_QUEUE 100

char Nodes_Ips[NODES_NO][100];        
u_int16_t *Nodes_ports;               
int  broad_sockets[(NODES_NO -1 )];   
int  listen_socket;                   

int  hasToken = 0, err, isReq = 0;                     
int  req[NODES_NO], last[NODES_NO], queue[NODES_NO];

struct sockaddr_in fd_nodes[NODES_NO]; 
struct sockaddr *bindPtr , accept_node; 
struct sockaddr *accept_nodePtr;

int NODE_ID=-1; //store node id                        
int ID_PORT;  // store node port                          

struct sockaddr_in serv_addr,client_addr;
struct sockaddr *servPtr,*clientPtr ;

int sock; //socket descriptor

pthread_mutex_t locker;

pthread_mutex_t file_locker;
