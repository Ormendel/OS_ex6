/*
https://www.geeksforgeeks.org/queue-linked-list-implementation/
*/

// A C program to demonstrate linked list based implementation of queue

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

/* this file is a server... include more libraries */
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>

#define BACKLOG 10 /* max threads running in parallel */
#define HIST 1
#define PORT "3490"
#define MAXLENGTH 1024
static int id = 1; //global variable

pthread_mutex_t lock;
pthread_cond_t cond_AO1;
pthread_cond_t cond_AO2;
pthread_cond_t cond_AO3;

/* ========================= SERVER ========================= */
char* transformString(char* s)
{
	char* ans = (char*)malloc(MAXLENGTH);
	if(strlen(s)==0)
	{
		puts("empty string, nothing to transform");
		return "";
	}
	int i=0;
	for(;i<strlen(s);++i)
	{
		if(islower(s[i]))
			ans[i] = toupper(s[i]);
		else // is upper case
			ans[i] = tolower(s[i]);
	}
	ans[i] = '\0';
	return ans;
}

char* caesarString(char* s)
{
	char* ans = (char*)malloc(MAXLENGTH);
	if(strlen(s)==0)
	{
		puts("empty string, nothing to transform");
		return "";
	}
	int i=0;
	for(;i<strlen(s);++i)
	{
		if(s[i]=='Z')
			ans[i] = 'A';
		else if(s[i]=='z')
			ans[i] = 'a';
		else
		{
			int oldValue = (int)(s[i]);
			int newValue = oldValue + HIST;
			ans[i] = (char)(newValue);
		}
	}
	ans[i]='\0';
	return ans;
}


void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* ========================= END ========================= */


// A linked list (LL) node to store a queue entry
struct QNode 
{
	void* value;
	struct QNode* next;
};

// The queue, front stores the front node of LL and rear stores the
// last node of LL
struct Queue 
{
	int idOfQueue; //tag to identify which queue are we talking about
    struct QNode *front, *rear;
};

typedef struct QNode QNode; //renaming
typedef struct Queue Queue; //renaming

/* checking if a queue is empty */
bool is_empty(Queue* q)
{
	return (q->front == NULL) ? true : false; 
}

/* printing a queue in list form */
void queue_status(Queue* q)
{
	if(is_empty(q))
	{
		printf("status of queue #%d: empty\n",q->idOfQueue);
		return;
	}
    QNode* temp = q->front;
	printf("status of queue #%d : ",q->idOfQueue);
    while(temp!=NULL)
    {
        printf("%s -> ",temp->value);
        temp = temp->next;
    }
    printf("null\n"); 
}

// A utility function to create a new linked list node.
QNode* new_QNode(void* v)
{
	QNode* temp = (QNode*)malloc(sizeof(QNode));
	temp->value = v;
	temp->next = NULL;
	return temp;
}

// A utility function to create an empty queue, and insert it to the list
Queue* createQ()
{
	Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
	q->idOfQueue = id++;
	puts("=========== A new queue was created successfully ===========");
    return q;
}

// The function to add a value v to q
void enQ(Queue* q, void* v)
{
	pthread_mutex_lock(&lock);
	// Create a new LL node
    QNode* temp = new_QNode(v);
    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL) 
	{
        q->front = q->rear = temp;
		printf("enqueued %s to queue #%d\n",temp->value, q->idOfQueue);
		pthread_mutex_unlock(&lock);
		return;
    }
 
    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;

	printf("enqueued %s to queue #%d\n",temp->value, q->idOfQueue);
	pthread_mutex_unlock(&lock);
}

// Function to remove a value from given queue q
void deQ(Queue* q)
{
	pthread_mutex_lock(&lock);
	int check_index = (((q->idOfQueue)-1)%3)+1;
	while(is_empty(q))
	{
		switch(check_index)
		{
			case 1: fprintf(stderr,"ERROR: The queue of first AO is empty, nothing to retrieve from it.\n");
					printf(". . . . . . waiting . . . . . .\n");
					pthread_cond_wait(&cond_AO1,&lock);
					break;

			case 2: fprintf(stderr,"ERROR: The queue of second AO is empty, nothing to retrieve from it.\n");
					printf(". . . . . . waiting . . . . . .\n");
					pthread_cond_wait(&cond_AO2,&lock);
					break;

			case 3: fprintf(stderr,"ERROR: The queue of third AO is empty, nothing to retrieve from it.\n");
					printf(". . . . . . waiting . . . . . .\n");
					pthread_cond_wait(&cond_AO3,&lock);
					break;

			default:
					break;
		}
	}

	// Store previous front and move front one node ahead
	QNode* temp = q->front;
	printf("removed %s from queue #%d\n",q->front->value, q->idOfQueue);
	q->front = q->front->next;

	// If front becomes NULL, then change rear also as NULL
	if (q->front == NULL)
		q->rear = NULL;

	free(temp); //At last - toss previous front
	pthread_mutex_unlock(&lock);
}

void destroyQ(Queue* q) //free allocated memory for queue q - from front to rear
{
	int id = q->idOfQueue;
	printf("======== Freeing allocated memory of Queue #%d ========\n",id);
    while(!is_empty(q))//As long q is not empty
	{
		deQ(q); //tosses all QNode's in q
	}
	free(q); //once q->front is null, we can free the whole queue
	printf("======== Queue #%d has been destroyed ! ========\n",id);
}

/*============================ Question 2 ============================*/
Queue* inputQ;
Queue* caesarQ;
Queue* transformQ;

static int* tempSocket = -1; //global variable that each active object uses


struct ActiveObject
{
	int i;
	Queue* q;
	void (*startFunc)();
	void (*endFunc)();
};
typedef struct ActiveObject ActiveObject; //renaming

static ActiveObject* ao1 = NULL;
static ActiveObject* ao2 = NULL;
static ActiveObject* ao3 = NULL;

void start_1stAO()
{
	printf("Start function of first AO, input is: %s\n",ao1->q->front->value);

}
void finish_1stAO()
{
	char* temp = caesarString((char*)(ao1->q->front->value));
	temp[strlen(temp)] = '\0';
	printf("Finish function of first AO, str after caesar: %s\n",temp);
	enQ(ao2->q, temp);
	pthread_cond_signal(&cond_AO2);
	printf("Finish function of first AO, converting to caesar string - done\n");
	
}

void start_2ndAO()
{
	printf("Start function of second AO, after caesar the string is: %s\n",ao2->q->front->value);
}
void finish_2ndAO()
{
	char* temp= transformString(ao2->q->front->value);
	temp[strlen(temp)] = '\0';
	printf("Finish function of second AO, str after transform: %s\n",temp);
	enQ(ao3->q, temp);
	pthread_cond_signal(&cond_AO3);
    printf("Finish function of second AO, transform string - done\n");
}

void start_3rdAO()
{
	if(send(*tempSocket, ao3->q->front->value , strlen(ao3->q->front->value), 0) == -1) //send back to client
	{
		perror("server error in send");
	}
	printf("Start function of third AO, sending back to client - done\n");
}
void finish_3rdAO()
{
	printf("Finish function of third AO, final string: %s\n",ao3->q->front->value);
	close(*tempSocket);
}

void run(ActiveObject** ao) /* Each thread will go to this function */
{
	Queue* q = (*ao)->q;
	while(!is_empty(q))
	{
		(*ao)->startFunc();
		(*ao)->endFunc();
		deQ(q);
	}

}

void destroyAO(ActiveObject* ao)
{

	/* using pthread_cancel for the thread of given ao */
	
	int check_index = ao->i; //can be 1, 2 or 3
	switch(check_index)
	{
		case 1: destroyQ(ao->q);
				if(pthread_cancel(pthread_self()==0))
				{
					printf("Cancelled thread of first AO successfully\n");
				}

				free(ao);
				printf("=========== First AO was destroyed successfully ===========\n");
				break;

		case 2: destroyQ(ao->q);
				if(pthread_cancel(pthread_self()==0))
				{
					printf("Cancelled thread of second AO successfully\n");
				}

				free(ao);
				printf("=========== Second AO was destroyed successfully ===========\n");
				break;

		case 3: destroyQ(ao->q);
				if(pthread_cancel(pthread_self()==0))
				{
					printf("Cancelled thread of third AO successfully\n");
				}

				free(ao);
				printf("=========== Third AO was destroyed successfully ===========\n");
				break;

		default:
				break;
	}
	
	

	
}
ActiveObject* newAO(Queue** q, int choice)
{
	
	ActiveObject* ao = (ActiveObject*)malloc(sizeof(ActiveObject));
	/* series of actions such as calling to handleQ and finishQ */
	ao->q = *q;
	switch(choice)
	{
		case 1: 
			ao->startFunc = &start_1stAO;
			ao->endFunc = &finish_1stAO;
			break;
		case 2:
			ao->startFunc = &start_2ndAO;
			ao->endFunc = &finish_2ndAO;
			break;
		case 3:
			ao->startFunc = &start_3rdAO;
			ao->endFunc = &finish_3rdAO;
			break;
		default:
			break;
	}
	ao->i = choice;
	return ao;
}

/*============================ END ============================*/


// This main is our server
int main(int argc, char *argv[])
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) 
	{
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

	 
    printf("server: waiting for connections...\n");

    pthread_t tid[BACKLOG];
    int i=0;
    while(1) 
    {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) 
        {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        char message[MAXLENGTH] = "Hello user, please enter a string (lower case or upper case letters only): ";
    	if (send(new_fd, message, strlen(message), 0) == -1)
    	{
        	perror("server error in send");
    	}
		else 
    	{
        	char client_message [MAXLENGTH];
        	/*Gets a message from client*/
        	if (recv(new_fd,client_message,sizeof(client_message),0) > 0)
        	{
            	sleep(1);
            	write(new_fd,client_message,strlen(client_message));
            	printf("------------------------------------\n");
            	printf("Server got client message: %s\n\n", client_message);
        	}
			/* ====== Start here AO chain ====== */
			pthread_mutex_init(&lock, NULL);
			pthread_cond_init(&cond_AO1,NULL);
			pthread_cond_init(&cond_AO2,NULL);
			pthread_cond_init(&cond_AO3,NULL);
			inputQ = createQ();
			caesarQ = createQ();
			transformQ = createQ();
			ao1 = newAO(&inputQ, 1); 
			ao2 = newAO(&caesarQ, 2); 
			ao3 = newAO(&transformQ, 3);
			tempSocket = &new_fd; //once we close tempSocket, so does new_fd
			int iret;	
			enQ(inputQ, client_message);
			//pthread_cond_signal(&cond_AO1);

			if ((iret=pthread_create(&tid[i%BACKLOG], NULL, &run, &ao1)) == 0)
			{
				++i;
			}
			else
			{
				printf("Failed to create thread\n");
			}

			if (pthread_join(tid[(i-1)%BACKLOG], NULL) != 0) 
			{
				perror("Failed to join thread");
			}

			if ((iret=pthread_create(&tid[i%BACKLOG], NULL, &run, &ao2)) == 0)
			{
				++i;
			}
			else
			{
				printf("Failed to create thread\n");
			}

			if (pthread_join(tid[(i-1)%BACKLOG], NULL) != 0) 
			{
				perror("Failed to join thread");
			}
			if ((iret=pthread_create(&tid[i%BACKLOG], NULL, &run, &ao3)) == 0)
			{
				++i;
			}
			else
			{
				printf("Failed to create thread\n");
			}

			if (pthread_join(tid[(i-1)%BACKLOG], NULL) != 0) 
			{
				perror("Failed to join thread");
			}
			// for(int j=i-3;j<i; ++j)
			// {
			// 	if (pthread_join(tid[j%BACKLOG], NULL) != 0) 
			// 	{
			// 		perror("Failed to join thread");
			// 	}
			// }
			sleep(1);
			printf("------------------------------------\n");

			sleep(5);
			destroyAO(ao1);
			destroyAO(ao2);
			destroyAO(ao3);
			pthread_mutex_destroy(&lock);
			pthread_cond_destroy(&cond_AO1);
			pthread_cond_destroy(&cond_AO2);
			pthread_cond_destroy(&cond_AO3);
			printf("------------------------------------\n");
		}
	}
	return 0;
}
