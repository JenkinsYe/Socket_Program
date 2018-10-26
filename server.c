#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <pthread.h>
#include <time.h>


#define MAX_QUEUE 	50
#define ADDR_LENGTH  40
#define BUFFER_SIZE  4096

typedef enum
{
	Disconnected, Connected
}state;

char* alladdr[MAX_QUEUE];
int allport[MAX_QUEUE];
int allsock[MAX_QUEUE];
pthread_t thread[MAX_QUEUE];
state isconnected[MAX_QUEUE];

int Index = 0; //mark the index of the client ID

// function declarations
void* thread_run(void* p);
void state_init(state array[], int lenth);
//char ip[] = "127.0.0.1";
int port = 2807;

int main()
{
    //client* Client;
	char ip[] = "127.0.0.1";
	int port = 2807;
	//int client_socket;
	// set the socket, bind with ip and port
    state_init(isconnected, MAX_QUEUE);

    printf("Server is running!\n");
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_socket == -1){
        printf("Socket create Failed!\n");
    }
    else{
        printf("Socket create Successfully!\n");
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    //server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    // listen state with max waiting Queue
    listen(server_socket, MAX_QUEUE);

    int client_socket;

    while(1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);

    	client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
        printf("hello %d\n", client_socket);
        if(client_socket == -1)
        {
            printf("Accept Error!\n");
        }
        else
        {
        	printf("A new Connection!\n");
        	printf("The information of new client:\n");
        	printf("ip address: %s: prot: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            alladdr[Index] = (char*)malloc(sizeof(char) * ADDR_LENGTH);
            strcpy(alladdr[Index], inet_ntoa(client_addr.sin_addr));
            allport[Index] = ntohs(client_addr.sin_port);
            // switch connection state
            isconnected[Index] = Connected;

            int flag = pthread_create(&thread[Index], NULL, &thread_run,&client_socket);
            if(flag == -1){
                printf("Thread create Failed!\n");
                break;
            }
            Index++;
        }
    
    }
    
    //close(client_socket);
    close(server_socket);

    return 0;

}

void state_init(state array[], int lenth){
    for(int i = 0 ; i < lenth; i++)
        array[i] = Disconnected;
}

void* thread_run(void* p){
	
    int client = *((int*)p);
    int maxfd;
    struct timeval tv;
    // File Description Set
    fd_set rfds;
    int retval;

    char send_buffer[BUFFER_SIZE];
    char receive_buffer[BUFFER_SIZE];
    char temp_buffer[BUFFER_SIZE];
    int sendlen,recvlen;
    char hello[BUFFER_SIZE] = "Welcome to visit the server!";

    char localtime[30];
    char hostname[32];

    int i,j;
    int curIndex, destinIndex;
    char message_buffer[BUFFER_SIZE];
    char Index_message[BUFFER_SIZE] = "Your ID number is ";
    curIndex = Index - 1;
    allsock[Index] = client;
    
    char ID[10];

    sprintf(ID, "%d", Index);
    strcat(Index_message, ID);
    strcat(hello, Index_message);
    if( send(client, hello, strlen(hello),0) > 0){
        //send(client, Index_message, strlen(Index_message), 0);
        printf("Hello message has been sent Successfully!\n");
    }
    else{
        printf("Error : Hello message cannot been sent\n");
    }

    while(1){
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        maxfd = 0;
        FD_SET(client, &rfds); // add socketfd to set
        if(maxfd <= client)
        {
            maxfd = client;
        }

        // timeout set
        tv.tv_sec = 6;
        tv.tv_usec = 0;

        retval = select(maxfd + 1, &rfds, NULL, NULL, &tv); // Test whether fds are readable
        if( retval == -1)
        {
            printf("Select Error!\n");
            break;
        }
        else
        {
            if(retval == 0) continue;
            // server send message to client
            else{
                if(FD_ISSET(0, &rfds)){
                    fgets(send_buffer, BUFFER_SIZE, stdin); // input

                    // Quit Information Check
                    if(!strncasecmp(send_buffer, "quit", 4)){
                        printf("server requires to quit!\n");
                        break;
                    }
                    sendlen = send(client, send_buffer, strlen(send_buffer), 0);
                    if( sendlen > 0){
                        printf("data has been sent\n");
                    }
                    else{
                        printf("data send Error!\n");
                        break;
                    }
                }
            
            
                //receive message from client
                if(FD_ISSET(client, &rfds)){

                    recvlen = recv(client, receive_buffer, BUFFER_SIZE, 0);
                    // diffreent request
                    // time request
                    if(recvlen > 0){
                        printf("Message from client: %s\n", receive_buffer);

                        if(strncmp(receive_buffer, "time", 4) == 0){
                            // get the system time
                            time_t t;
                            time(&t);
                            ctime_r(&t, localtime);
                            
                            if(send(client, localtime, strlen(localtime),0) > 0)
                                printf("Time has been sent!\n");
                            else{
                                printf("Error : Time cannot be sent\n");
                                break;
                            }
                        }
                        // name request
                        else if(strncmp(receive_buffer, "name", 4) == 0){
                            gethostname(hostname, sizeof(hostname));
                            sprintf(hostname, "%s\n", hostname);

                            if( send(client, hostname, strlen(hostname), 0) > 0){
                                printf("Name Information has been sent!\n");
                            }
                            else{
                                printf("Error: Name Information cannot been sent!\n");
                                break;
                            }
                        }
                        
                        // list request
                        else if(strncmp(receive_buffer, "list", 4) == 0){
                            for(int i = 0; i< Index; i++){
                                bzero(send_buffer, BUFFER_SIZE);

                                if(isconnected[i] == Connected){
                                    sprintf(send_buffer, "userid: %d, addr: %s : %d\n", i+1, alladdr[i], allport[i]);
                                    if( send(client, send_buffer, strlen(send_buffer), 0) > 0){
                                        printf("all the user information has been sent Successfully!\n");
                                    }
                                    else{
                                        printf("Error: user Information cannot been sent!\n");
                                        break;
                                    }
                                }
                            }
                        }
                        // disconnect
                        else if(strncmp(receive_buffer, "quit", 4) == 0){
                            int ID;
                            ID = receive_buffer[4] - 48;
                            isconnected[ID - 1] = Disconnected;
                        }
                        // send request
                        else if(strncmp(receive_buffer, "send", 4) == 0){
                            // get information from receiver client

                            // get the number of destination
                            j = 0;
                            int mark = 0;
                            bzero(temp_buffer, BUFFER_SIZE);
                            for(int i = 4; i < sizeof(receive_buffer); i++){
                                if(receive_buffer[i] != '*'){
                                    temp_buffer[j] = receive_buffer[i];
                                    j++;
                                }
                                else {
                                    mark = i;
                                    break;
                                }
                            }

                            destinIndex = atoi(temp_buffer);
                            mark = mark + 1; // start of message to be sent.
                            j = 0;
                            bzero(message_buffer, BUFFER_SIZE);
                            for(; mark < sizeof(receive_buffer); mark++){
                                if(receive_buffer[mark] != '\n'){
                                    message_buffer[j] = receive_buffer[mark];
                                    j++;
                                }
                                else break;
                            }

                            bzero(send_buffer, BUFFER_SIZE);
                            sprintf(send_buffer, "Message from id: %d, addr: %s : %d\n%s\n", curIndex + 1, alladdr[curIndex], allport[curIndex], message_buffer);
                            if ( send(allsock[destinIndex], send_buffer, strlen(send_buffer), 0) > 0){
                                printf("data has been transmitted Successfully!\n");
                            }
                            else{
                                printf("Error: data cannot been transmitted!\n");
                                break;
                            }
                        }
                         
                    }
                }
            memset(receive_buffer, 0, BUFFER_SIZE);
            
            }      
        }
    }
    isconnected[Index] = Disconnected;
    close(client);
    pthread_exit(NULL);

}