#include "proxy_parse.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<semaphore.h>
#include<errno.h>
#include<time.h>
#include<pthread.h>
#define MAX_CLIENTS 10

typedef struct cache_element cache_element;

// cache element 
// will be implementing linked list
struct cache_element{
    char * data;
    int len;
    char * url;
    time_t lru_time_track;
    cache_element * next;
};

cache_element * find(char * url);
int add_cache_element(char * data , int size , char * url);
void remove_cache_element();

int port_number = 8080;
//global proxy socket id
int proxy_socketId;
//creating threads so that each client will have separate socket
pthread_t tid[MAX_CLIENTS];
sem_t semaphore;
pthread_mutex_t lock;

//cache defined
cache_element * head ;
int cache_size;


void * thread_fn(void * socketNew){
    
}

int main(int argc, char * argv[]){
    int client_socketId , client_len;
    struct sockaddr_in server_addr , client_addr;
    
    //initializing semaphore
    // semaphore , min_value, max_value
    sem_init(&semaphore,0, MAX_CLIENTS);

    pthread_mutex_init(&lock, NULL);

    if(argc == 2){
        port_number = atoi(argv[1]);
    }
    else {
        printf("too few arguments\n");
        exit(1);
    }

    printf("starting proxy server at port %d\n" , port_number);
    //creating proxy server
    proxy_socketId = socket(AF_INET, SOCK_STREAM,0);

    if(proxy_socketId < 0){
        perror("failed to create a socket\n");
        exit(1);
    }
    int reuse = 1;
    //setting up socket property to reuse the same socket
    if(setsockopt(proxy_socketId, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse , sizeof(reuse)) < 0){
        perror("setsockopt failed\n");
    }

    //setting up server properties
    bzero((char *) & server_addr , sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(proxy_socketId,(struct sockaddr *) &server_addr , sizeof(server_addr)) < 0){
        perror("port is not available\n");
        exit(1);
    }

    printf("binding on port %d\n" , port_number);
    int listen_status = listen(proxy_socketId,MAX_CLIENTS);

    if(listen_status < 0){
        perror("error in listening\n");
        exit(1);
    }

    //iterator define
    int i = 0;
    int Conneted_socketId[MAX_CLIENTS];

    while(1){
        bzero((char *) & client_addr , sizeof(client_addr));
        client_len = sizeof(client_addr);
        client_socketId = accept(proxy_socketId , (struct sockaddr *) & client_addr , (socklen_t *) & client_len);

        if(client_socketId < 0){
            printf("not able to connect\n");
            exit(1);
        }
        else{
            Conneted_socketId[i] = client_socketId;
        }

        // Getting IP address and port number of client
		struct sockaddr_in* client_pt = (struct sockaddr_in*)&client_addr;
		struct in_addr ip_addr = client_pt->sin_addr;
		char str[INET_ADDRSTRLEN];										// INET_ADDRSTRLEN: Default ip address size
		inet_ntop( AF_INET, &ip_addr, str, INET_ADDRSTRLEN );
		printf("Client is connected with port number: %d and ip address: %s \n",ntohs(client_addr.sin_port), str);

        pthread_create(&tid[i],NULL , thread_fn , (void *) & Conneted_socketId[i]);
        i++;
    
    }

    close(proxy_socketId);
    return 0;
}
