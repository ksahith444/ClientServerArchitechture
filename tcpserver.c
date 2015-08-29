#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <pthread.h>
#define RCVBUFSIZE 1024
#define THREADS 20
#define listen 10    /* Maximum outstanding connection requests */

struct thread_data
{
   int	clientsock;
   int  receivemsgsize;
   char *message;
};

struct thread_data thread[10];

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}  /* Error handling function */
void *HandleTCPClient(void *threaddata)
{
    char *buffer;        
    int recvMsgSize; 	/* Size of received message */
	char *token,*p[3];
	struct hostent *remoteserver;
	int sock1;
	 struct sockaddr_in echoServAddr1;
	 char details[RCVBUFSIZE];
	char url[100];
	struct thread_data *my_data;
	int clntSocket;
	 my_data = (struct thread_data *) threaddata;
	 
	  pthread_detach(pthread_self());
	  
   clntSocket  = my_data->clientsock;
   recvMsgSize = my_data->receivemsgsize;
   buffer = my_data->message;
	
     token = strtok(buffer,":");
		
	int j=0;
   while( token != NULL )
   {
       p[j] = token;
	  // printf("%s \n",p[j]);
      token = strtok(NULL, ":");
		j++;
   }
   printf("message received form client host: %s port : %s file: %s \n ",p[0],p[1],p[2]);
   
   remoteserver = gethostbyname(p[0]);
   if ( remoteserver == NULL )
    DieWithError("socket() failed");
   
   if ((sock1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
																	
																		 /* Construct the server address structure */
    memset(&echoServAddr1, 0, sizeof(echoServAddr1));                 	 /* Zero out structure */
    echoServAddr1.sin_family      = AF_INET;                          	 /* Internet address family */
    struct in_addr ip_addr;
	ip_addr = *(struct in_addr *) remoteserver->h_addr;             
	echoServAddr1.sin_addr.s_addr = inet_addr(inet_ntoa(ip_addr));  		/* Server IP address */
    echoServAddr1.sin_port        = htons(atoi(p[1]));                       /* Server port */

																	/* Establish the connection to the server */
    if (connect(sock1, (struct sockaddr *) &echoServAddr1, sizeof(echoServAddr1)) < 0)
        DieWithError("connect() failed");
	
			printf("socket connected to %s webserver \n" ,p[0]);
		//sprintf(details, "GET http://%s/%s HTTP/1.1\r\nConnection: close\r\n\r\n",p[0],p[2]);
		sprintf(details, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",p[2],p[0]);
																				
     int stringlen = strlen(details);											/* Determine input length */																	
    if (send(sock1, details , stringlen, 0) != stringlen)					/* Send the string to the server */
        DieWithError("send() sent a different number of bytes than expected");
       printf(" Sent message to  %s web-server \n",p[0]);
	   printf("%s",details);
	   
	if( (recvMsgSize = recv(sock1,buffer,RCVBUFSIZE,0))<0)
	DieWithError("RECEIVE FAILED");
    /* Send received string and receive again until end of transmission */
    while (recvMsgSize > 0)      /* zero indicates end of transmission */
    {
        /* Echo message back to client */
        if (send(clntSocket, buffer, recvMsgSize, 0) != recvMsgSize)
            DieWithError("send() failed");

        /* See if there is more data to receive */
        if ((recvMsgSize = recv(sock1, buffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");
    }

    close(clntSocket);  	/* Close client socket */
	pthread_exit(NULL);
}   /* TCP client handling function */

int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
	char buffer[RCVBUFSIZE];
	unsigned int clntLen;
	pthread_t threads[THREADS];
	int recvMsgSize; 
	fd_set mainset;
	fd_set read_fds;
	int max;
	
	FD_ZERO(&mainset);
	FD_ZERO(&read_fds); 
	
    if (argc != 2)     /* Test for correct number of arguments */
		DieWithError("enter 2 arguments");

    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons( atoi(argv[1]));      /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");
		
    /*  listen for incoming connections */
    if (listen(servSock, listen) < 0)
        DieWithError("listen() failed");
		printf("socket is in listening \n ");
		
	FD_SET(servSock, &mainset);
	max = servSock;
	
    for (;;) /* Run forever */
    {
		read_fds = mainset;
		
		if(select(max+1, &read_fds, NULL, NULL, NULL) == -1)
			DieWithError("select function errorMessage");
			
			int i;
			for(i = 0; i <= max; i++)
					{
						if(FD_ISSET(i, &read_fds))
							{ /* we got one... */
								if(i == servSock)
									{
			
								clntLen = sizeof(echoClntAddr);

											/* Wait for a client to connect */
				if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
						DieWithError("accept() failed");
						
						//printf("established connection to client with client socket id is %d \n",clntSock);
						
								FD_SET(clntSock, &mainset); /* add to mainset set */
										if(clntSock > max)
											max = clntSock;	
												
						}
        /* clntSock is connected to a client! */
			else{
			
        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
		
		 if ((recvMsgSize = recv(i, buffer, RCVBUFSIZE, 0)) < 0)
		DieWithError("recv() failed");
		
		buffer[recvMsgSize - 1 ] = '\0' ;
		
		 thread[i].clientsock = i;
		 thread[i].receivemsgsize = recvMsgSize;
         thread[i].message = buffer;
		 
		 pthread_create(&threads[i], NULL, HandleTCPClient, (void *) &thread[i]);
        
		
		FD_CLR(i, &mainset);
		
    }
    /* NOT REACHED */
	}
	}
	}
}

