#include "MutualExclusion.h"

// Decoding the mesage type recieved - ISTOKEN or ISREQ
void decode(struct message *msg , char *string){
    msg->type=strdup(strtok(string, ","));

    //Check the type of the message
    if((strcmp(msg->type , "ISTOKEN"))==0){
        int len = 0;
        msg->lenqueue = atoi(strtok(NULL, ","));
        len = msg->lenqueue;
        
        if(len > 0)
            isReq = 1;

        
        FOR0(i, len){
            int tempQueue=atoi(strtok(NULL, ","));
            msg->queue[i]= tempQueue ;
            queue[i]=tempQueue;
        }

        msg->lenlast = atoi(strtok(NULL, ","));
        len = msg->lenlast;

       
        FOR0(i, len){
            int tempLast= atoi(strtok(NULL, ","));
            msg->last[i] =tempLast;
            last[i]=tempLast;
        }
        msg->NODEID=atoi(strtok(NULL, ","));
    }

    else if(strcmp(msg->type , "ISREQ")==0){
        msg->NODEID = atoi(strtok(NULL, ","));
        msg->req = atoi(strtok(NULL, ","));
    }

}

void signalHandler(){
    close(listen_socket);

    FOR0(i, NODES_NO)
        close(broad_sockets[i]);
    
    exit(0);
}

//Function to read config file info
void readConfigInfo(char *filename){
	FILE  *config;   //File descriptor 
    int MAX_NODES, port, i;   
    char node_name[100];                 

    //Opens config file - store info about processes
    if(!(config = fopen("config.txt","r"))){
        perror("\nfopen config()");
        exit(-1);
    }

    //Read number of processes
    fscanf(config, "%d", &MAX_NODES);
    //Allocate two arrays one for the name of nodes and one for the ports
    Nodes_ports = (u_int16_t *) malloc(MAX_NODES * sizeof(int));
    //Go through each line of the config file and retrieve ip address & port of each node.
    for(fscanf(config,"%s%d", node_name,&port), i = 0; !feof(config), i<MAX_NODES; fscanf(config,"%s%d", node_name,&port), i++){
        strcpy(Nodes_Ips[i] , node_name);
        Nodes_ports[i]=port;
    }

}

// Function to initialize sockets 
void inisializations(){
    FOR0(i, NODES_NO){
        if(ID_PORT == Nodes_ports[i]){
          NODE_ID = i;
          fd_nodes[i].sin_family = AF_INET;
          fd_nodes[i].sin_addr.s_addr = inet_addr(Nodes_Ips[i]);
          fd_nodes[i].sin_port = htons(Nodes_ports[i]);
        }
        else{
            fd_nodes[i].sin_family = AF_INET;
            fd_nodes[i].sin_addr.s_addr =  htonl(INADDR_ANY);
            fd_nodes[i].sin_port = htons(Nodes_ports[i]);
        }
    }

    bindPtr=(struct sockaddr *) &fd_nodes[NODE_ID];

    FOR0(i, NODES_NO){
        if(!i)  req[i] = 1;
        else    req[i] = 0;
        queue[i] = -1;
        last[i] = 0;
    }
    if(!NODE_ID)
        hasToken = 1;

    //Initialization of mutexes
    // for strtok in decode function
    pthread_mutex_init(&locker,NULL);
    // for file lock
    pthread_mutex_init(&file_locker,NULL);
}


// Function to bind thread to a port and listening
void *bind_thread(){
    pthread_t tid;

    int newsock, clientlen; 

    accept_nodePtr = (struct sockaddr *) &accept_node;
    int nodelen=sizeof( accept_nodePtr);

    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(ID_PORT);

    servPtr=(struct sockaddr *) &serv_addr;

    if((sock=socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("Socket() failed"); 
        exit(1);
    }

    if(bind(sock,servPtr, sizeof(serv_addr)) < 0){
        perror("Bind() failed");
        exit(1);
    }

    if(listen(sock , MAX_QUEUE) < 0){
        perror("Listen() failed"); 
        exit(1);
    }
    printf("\nListening on port- %d \n" , ID_PORT);

     while(1){
        clientPtr=(struct sockaddr *) &client_addr;
        clientlen= sizeof(client_addr);

        if((newsock=accept(sock , clientPtr , &clientlen)) < 0){
            perror("accept() failed"); 
            exit(1);
        }
        if(err=pthread_create(&tid , NULL , &accept_thread ,  (void *) newsock)){
            perror2("pthread_create for accept_thread" , err);
            exit(1);
        }
     }
}


// Function to accept connection from a specific node
void *accept_thread(void *accept_sock){
    int acpt_sock;
    char buffer[256];
    int msg_size; 

    acpt_sock= ((int)(accept_sock));

    bzero(buffer,sizeof(buffer));
    if((msg_size = recv(acpt_sock, buffer, sizeof(buffer),0)) < 0){
        perror("Error received msg in bind_thread()");
        close(acpt_sock);
        exit(1);
    }
    printf("Message received- %s\n" , buffer);
    if(err = pthread_mutex_lock(&locker)){
        perror2("Failed to lock()",err);
    }
    struct message msg;
    msg.type=(char *)malloc(sizeof(char) * 15);
    msg.queue = (int *)malloc(sizeof(int) * (NODES_NO));
    msg.last = (int *)malloc(sizeof(int) *  (NODES_NO));

    decode(&msg, buffer);

    if(err = pthread_mutex_unlock(&locker))
        perror2("Failed to lock()",err);

    if(strcmp(msg.type , "ISREQ")==0){
        if(req[msg.NODEID] < msg.req ){
            req[msg.NODEID] = msg.req;

            if(err = pthread_mutex_lock(&locker))
                perror2("Failed to lock()",err);
            if(hasToken==1){
                for(int j=0; j < NODES_NO; j++){
                   if(queue[j]== -1){
                        if(!IsInQueue(msg.NODEID) && req[j] ==last[j] + 1 ){
                            queue[j]= msg.NODEID;
                            isReq=1;
                        }
                    }
                }
            }
            if(err = pthread_mutex_unlock(&locker))
                perror2("Failed to lock()",err);
        }
    }

    else if(strcmp(msg.type , "ISTOKEN")==0){
        printf("\nNode: %d received the token from : %d \n", NODE_ID, msg.NODEID);
        hasToken = 1;
    }
    close(acpt_sock);

    pthread_exit(0);
}

// function to generate random no for sleeping a node
int genNo(int rand_NoIn){
    int randNo = 0;
    int MAX_N = rand_NoIn;
    int MIN_N = 4;

    srand(time(NULL));
    randNo = rand() % (MAX_N - MIN_N + 1) + MIN_N;

    return randNo;
}

// function to broadcast a message to all other node
void broadcast(char *msg){ //msg 1
    char buf[256];
    bzero(buf,sizeof(buf));
    int msglen = 0;

    FOR0(i, NODES_NO){
        if( i != NODE_ID){
            if ((broad_sockets[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                perror("Socket() failed");
                exit(1);
            }

            if (connect(broad_sockets[i] , (struct sockaddr *) &fd_nodes[i], sizeof(fd_nodes[i])) < 0)
                printf("Unable to Connect to : %s ,  PORT: %d\n", inet_ntoa(fd_nodes[i].sin_addr), ntohs(fd_nodes[i].sin_port));
        }
    }

    FOR0(i, NODES_NO){
       if( NODE_ID != i){
            sprintf(buf,"ISREQ,%d,%d" , NODE_ID , req[NODE_ID]);
            // printf("\nNode- %d is sending a msg to Node- %d \n" , NODE_ID , i);
            printf("\nNode- %d is sending a Request_To_Enter_CS_Message to Node- %d \n" , NODE_ID , i);
            
            send(broad_sockets[i] , buf,sizeof(buf) , 0);
        }
    }

    FOR0(i, NODES_NO){
        if (NODE_ID != i)
            close(broad_sockets[i]);
    }
}

// function to send token to the requesting node
void sentToken(){
    if(err=pthread_mutex_lock(&locker))
        perror2("Failed to lock()",err);
    hasToken=0;

    if(err=pthread_mutex_unlock(&locker))
        perror2("Failed to lock()",err);

    char buf[256]; 
    char sendQueue[256];

    char sendLast[256];

    int sendTokenTo=-1;
    int calQueueLen=0; 

    bzero(sendQueue , sizeof(sendQueue));
    bzero(sendLast , sizeof(sendLast));

    char str[256];

    FOR0(i, NODES_NO){
        if(queue[i] != -1 ){
            sendTokenTo=queue[i];
            queue[i]=-1;
            break;
        }
    }

    FOR0(i, NODES_NO){
        if(queue[i] != -1){
            bzero(str,sizeof(str));
            calQueueLen +=1;
            sprintf(str,",%d" , queue[i]);
            strcat(sendQueue,str);
        }
    }

    bzero(str, sizeof(str));
    FOR0(i, NODES_NO){
        bzero(str,sizeof(str));
        sprintf(str,"%d" , last[i]);
        strcat(sendLast,str);

        if(i != (NODES_NO-1) )
            strcat(sendLast, ",");
    }

    FOR0(i, NODES_NO){
        last[i] = 0;
        queue[i] = -1;
    }

    if ((broad_sockets[NODE_ID] = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket() failed");
        exit(1);
    }
    if (connect(broad_sockets[NODE_ID] , (struct sockaddr *) &fd_nodes[sendTokenTo], sizeof(fd_nodes[1])) < 0)
        printf("Unable to Connect to : %s ,  PORT: %d\n", inet_ntoa(fd_nodes[sendTokenTo].sin_addr), ntohs(fd_nodes[sendTokenTo].sin_port));
    

    bzero(buf,sizeof(buf));
    if(calQueueLen !=0)
        sprintf(buf,"ISTOKEN,%d%s,%d,%s,%d" ,calQueueLen, sendQueue , NODES_NO , sendLast , NODE_ID );
    
    else
        sprintf(buf,"ISTOKEN,%d,%d,%s,%d" ,calQueueLen, NODES_NO , sendLast,NODE_ID );
    
    isReq = 0;
    // printf("\nNode: %d is gonna send a msg to Node: %d , the msg: %s\n" , NODE_ID , sendTokenTo , buf );
    printf("\nNode- %d is sending a Released_Token_Message to Node- %d , Message- %s\n" , NODE_ID , sendTokenTo , buf );
    
    send(broad_sockets[NODE_ID] , buf,sizeof(buf) , 0);
    close(broad_sockets[NODE_ID]);

}


// function to check if a node is in queue
int IsInQueue(int nodeIn){
    FOR0(i, NODES_NO){
      if( queue[i] == nodeIn)
          return 1;
    }
    return 0;
}


// function to calculate total number of request for CS recieved
int calTotalReq(){
    int total=0;
    FOR0(i, NODES_NO)
        total +=req[i];

    total -= req[NODE_ID];

    return total;
}



// void logExpirament(int totalReq , int loopsIn){
//     if(err=pthread_mutex_lock(&file_locker))
//         perror2("Failed to lock()",err);

//     FILE *fp;
//     char res[8];
//     bzero(res,sizeof(res));
//     sprintf(res,"%d\n" , (totalReq/loopsIn));

//     fp=fopen("output.txt", "a+");
//     fprintf(fp , res , sizeof(res));
//     fclose(fp);

//     if(err = pthread_mutex_unlock(&file_locker))
//         perror2("Failed to lock()",err);

//     printf("Average value:%d\n" , (totalReq/loopsIn));
// }

int main(int agrc , char *argc[]){

    printf("=========================================================================\n");
    printf("Types Of Messages- \n");
	printf("1. IsToken Message :Current node sends the token to next requesting node\n");
	printf("2. IsReq Message :Current node sends request to enter in critical section\n");
    printf("=========================================================================\n");


    ID_PORT=atoi(argc[2]);

    signal(SIGINT, signalHandler); 	// ctrl-c

    pthread_t tid[3];

    readConfigInfo("config.txt");

    inisializations();

    // printf("----------------------------------------------------------\n");
    printf("Started Node- %d on port- %d\n" , NODE_ID , ID_PORT);
    // printf("----------------------------------------------------------\n");
    // printf("\n");

    sleep(3);
    if(err=pthread_create(&tid[0] , NULL , &bind_thread , NULL)){
        perror2("pthread create" , err);
        exit(1);
    }
    sleep(20);

    int counter = 2, loop = 2, totalReqBefore =0, totalReq;

  while(counter){
    sleep(genNo(35));

    if(hasToken == 0){
        sleep(1);
        req[NODE_ID] = req[NODE_ID] + 1;

        totalReqBefore = calTotalReq();
        broadcast("1");

        while(hasToken == 0){
            sleep(1);
        }
        totalReq += calTotalReq() - totalReqBefore ;
    }

    printf("================================================\n");
    printf("Node: %d is entering in critical section.\n" , NODE_ID);
      sleep(genNo(10));

    last[NODE_ID]= req[NODE_ID];
    printf("Node: %d has exited the critical section.\n" , NODE_ID);
    printf("================================================\n");

    while(!isReq);

    sentToken();

    sleep(genNo(30));
    counter = counter - 1;
  }
  // logExpirament(totalReq,loop);
  printf("----------------------------------------------------\n");
  printf("Completed\n");
  printf("----------------------------------------------------\n");
  while(1);
  return 0;
}
