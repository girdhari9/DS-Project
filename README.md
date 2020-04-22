# DS-Project

##PART 1 - CAUSAL AND ARBITRARY SYSTEM
Start all the clients on different terminals. TO broadcast a message simply go to that client terminal and type a message without spaces, then press enter key. That particualar messages will be delivered to all other clients.

##Compile -
To compile server.cpp -
g++ server.cpp -pthread -o server

To compile client.cpp -
g++ client.cpp -pthread -o client

##Run -
To run server -
./server <port> 
example
./server 8080

To run client -
./client <clientid> <port>
example
./client 1 8080
./client 2 8080
./client 3 8080

##PART 2 - MUTUAL EXCLUSION ALGORITHM IMPACT
Types of msg -
1.  IsToken - a node has finished from critical section and sends the token to next requesting node. 
2. IsReq - a node requests to enter in critical section.
The nodes port numbers are given in config.txt file from which the program matches the it. After running all the nodes in different terminals, the program automatically generates random number for nodes request wait time and shows mutual exclusion.

##Compile -
gcc MutualExclusion.c -lm -lpthread -o MutualExclusion
 (OR)
make

##Run -
//first starting node 0
./MutualExclusion -p 8080
//starting node 1
./MutualExclusion -p 8081
//starting node 2
./MutualExclusion -p 8082
//starting node 3
./MutualExclusion -p 8083
