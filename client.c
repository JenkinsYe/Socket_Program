#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>
#include <pthread.h>


#define BUFFER_SIZE 4096
#define ADDRESS_LENGTH 40
#define QUEUE_LENGTH 50
typedef enum{
	Disconnected, Connected
} State;


struct messageQueue
{
	int front, tail;
	char* queue[QUEUE_LENGTH];
};


struct sockaddr_in server_addr;
State state = Disconnected;
void* thread_run(void *arg);
int ID = -1;
// function declarations
void sendMessage(int socket);
void userAsk(int socket);
void nameAsk(int socket);
void timeAsk(int socket);
void CommandList(int* command);
void* thread_run(void *arg);
void connectionClose(int socket);
int queueEmpty();
void enqueue(char* s);
char* dequeue();

int input = 0;
struct messageQueue mq;
char* t = "\n";
int main(){
	printf("---------------------------------\n");
    printf("Welcome to the client!\n");
    printf("---------------------------------\n");
    printf("You want to Connect or Quit?\n1. Connect\n2. Quit\n");
    int choice = 0;
    pthread_t tid;
    int port;
    char server_ip[ADDRESS_LENGTH];
    scanf("%d", &choice);
    state = Disconnected;
    mq.front = 0;
    mq.tail = 1;
    enqueue(t);
    if(choice == 1)
    {
    	int sock = -1;
  		while(1){
  			int command = -1;
		    // if(! input) CommandList(&command);
		    // different requests
		    // connect
            // busy waiting
            int flag = 1;
            while(1)
            {
            	if(!queueEmpty()){
            		while(!queueEmpty()){
	            	printf("%s\n", dequeue());
	                }
	                flag = 0;
            	}
            	if(flag == 0) break;
            }
	            
		    while(input){

		    }
		    CommandList(&command);
		    
		    if(command == 1){
		    	// input the IP and Port information
		    	printf("Enter the server IP address:\n");
		    	scanf("%s", server_ip);
		    	printf("Enter the port:\n");
		    	scanf("%d", &port);

		    	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		    	//printf("sock == %d\n", sock);
		    	if( sock < 0){
		    		printf("Error: socket create failed!\n");
		    	}

				memset(&server_addr, 0, sizeof(server_addr));
				server_addr.sin_family = AF_INET;
				server_addr.sin_addr.s_addr = inet_addr(server_ip);
				server_addr.sin_port = htons(port);
				int flag = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
			    if(flag == 0){
			    	state = Connected;
	                printf("Connect Successfully!\n");
	                
	                int ret = pthread_create(&tid, NULL, thread_run, &sock);
	                if(ret != 0){
	                	printf("pthread_create Error\n");
	                }

			    }
			    else printf("Error : Connect Failed\n");
		    }
		    // disconnect
		    else if(command == 2){
	            if(state = Connected){
                    connectionClose(sock);
                    printf("Connection Closed!\n");
                    state = Disconnected;
	            }
		    }
		    // time request
		    else if(command == 3){
		    	//printf("sock == %d\n", sock);
                timeAsk(sock);
		    } 
		    // name request
		    else if(command == 4){
		    	nameAsk(sock);
		    }
		    // ask for users
		    else if(command == 5){
                userAsk(sock);
		    }
		    // send message
		    else if (command == 6){
		    	sendMessage(sock);
		    }
		    //quit
		    else{
		    	if(state == Disconnected) return 0;
		    	else{
		    		connectionClose(sock);
		    		state = Disconnected;
		    		return 0;
		    	}
		    }
		    
	    }
	}
	else return 0;
}

int queueEmpty(){
	if((mq.front + 1) == mq.tail) return 1;
	else return 0;
}

void enqueue(char* s){
	mq.queue[mq.tail] = (char*)malloc(BUFFER_SIZE * sizeof(char));
	strcpy(mq.queue[mq.tail], s);
	mq.tail++;
}

char* dequeue(){
	return mq.queue[++mq.front];
}

void sendMessage(int socket){
	int destin;
	char Msg[BUFFER_SIZE]; //message
	char header[BUFFER_SIZE + 5] = "send:";
    
    input = 1;
	printf("Please input the number of the client;\n");
	printf("If you don't konw the number of clients, use Command 5 to get more information\n");
	scanf("%d", &destin);
	printf("Please input the message to be sent\n");
	getchar();
	fgets(Msg, BUFFER_SIZE, stdin);
	//scanf("%s", Msg);
    //pack the message with the commander header
    sprintf(header, "send%d*", destin);
	strcat(header, Msg);
    input = 0;
    int flag = send(socket, header, sizeof(header), 0);
    if(flag == -1) printf("Error: Failed to send the message\n");
    else printf("Your Message has been sent!\n");
}

void userAsk(int socket){
	int flag = send(socket, "list", 4, 0);
	if( flag == -1) printf("Error : Failed to get the users list!\n");
}

void nameAsk(int socket){
	if(socket == -1) printf("Invalid socket!\n");
	int flag = send(socket, "name", 4, 0);
	if(flag == -1) printf("Error : Failed to get the name!\n");
}

void timeAsk(int socket){
	if(socket == -1) printf("Invalid socket!\n");
	int flag = send(socket, "time", 4, 0);
	if( flag == -1) printf("Error : Failed to get the time!\n");
}

void CommandList(int* command){
	int temp;
	//sleep(0.5);
	while(1){
		printf("Command List:\n");
	    printf("    1. Connect to server with ip address and port\n");
	    printf("    2. Disconnect.\n");
	    printf("    3. Ask for Time\n");
	    printf("    4. Ask for Name\n");\
	    printf("    5. Ask for User List\n");
	    printf("    6. Send the message to the connected site\n");
	    printf("    7. Quit\n");
	    //int temp;
	    scanf("%d", &temp);

	    if(temp>=1 && temp <=7) break;
	    else{
	    	printf("Error : Invalid Command!\n");
	    	printf("Please try again!\n");
	    }
    }

   *command = temp;
}

void* thread_run(void *arg)
{
    int socket;
    char receive_buffer[BUFFER_SIZE];
    socket = (int)(*(int*)arg);
    memset(receive_buffer, 0, BUFFER_SIZE);

	while(1)
	{
	    int flag = recv(socket, receive_buffer, sizeof(receive_buffer), 0);
	    if(flag > 0){
	    	enqueue(receive_buffer);
	    	/*
	    	printf("Message Received:\n");
	    	printf("%s\n", receive_buffer);
	   
	    	if(strncmp(receive_buffer, "Welcome", 7) == 0){
	    		//printf("Hello\n");
	    		//recv(socket, receive_buffer, sizeof(receive_buffer), 0);
	    		//printf("%c\n", receive_buffer[18]);
	    		//ID = atoi((char*)receive_buffer[18]);
	    		ID = receive_buffer[45] - 48;
	    		printf("ID = %d\n", ID);
	    	}*/
	    
	    	memset(receive_buffer, 0, sizeof(receive_buffer));
	    }
	    else if (flag < 0){
	    	printf("Error : Message received failed.\n");
	    	return NULL;
	    }
	    else return NULL;
	    //if(state == Disconnected) return NULL;
	    //if(option == 1)
	    //printf("Message from server: %s %d\n", buffer, index++);
	}
    return NULL;
}

void connectionClose(int socket){
	//printf("You want to disconnect? Please input you ID number");
	char number[5];
	sprintf(number , "%d", ID);

	char Message[BUFFER_SIZE] = "quit";
	strcat(Message, number);  
	send(socket, Message, BUFFER_SIZE, 0);
	close(socket);
    
    
}