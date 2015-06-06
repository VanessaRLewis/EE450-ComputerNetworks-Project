#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define MYPORT "23348" // s3's tcp port that server3 will be connecting to 
#define MAXDATASIZE 100 // max number of bytes we can get at once 
#define BACKLOG 10

struct serverData
{
	char key[7];
	char val[8];
}sd3[4];

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}//Source: Beej's Guide to Network Programming

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}//Source: Beej's Guide to Network Programming	

int main()
{
	/*read from server2.txt & storage in a struct*/
	FILE *fp;
	fp = fopen("server3.txt","r");
	if(fp == NULL)
	{
		perror("Error opening file");
		return(-1);
	}
	
	char k[7];
	char v[8];
	int i=0;
	
	while(fscanf(fp,"%s %s",k,v)!=EOF)
	{
		strcpy(sd3[i].key,k);
		strcpy(sd3[i].val,v);
		i++;
	}
	
	fclose(fp);
	int end = 4;
	
	
	/*******CREATING TCP LISTENING PORT********/
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size, client_size;
    struct sigaction sa;
    int yes=1;
    char s3[INET6_ADDRSTRLEN];
	char s2[INET6_ADDRSTRLEN];
    int rv;
	int z=1;
	
	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	//hints.ai_flags = AI_PASSIVE; // use my IP
	
	if ((rv = getaddrinfo("nunki.usc.edu", MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}//Source: Beej's Guide to Network Programming
	
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}
		
		break;
	}//Source: Beej's Guide to Network Programming
	
	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}//Source: Beej's Guide to Network Programming
	
	freeaddrinfo(servinfo); // all done with this structure
	
	if (listen(sockfd, BACKLOG) == -1) 
	{
		perror("listen");
		exit(1);
	}//Source: Beej's Guide to Network Programming
	
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}//Source: Beej's Guide to Network Programming
	
	
	//displaying s3 IP Adressing
	void *addr;
	struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
	addr = &(ipv4->sin_addr);
	inet_ntop(p->ai_family, addr, s3, sizeof s3);
	printf("\nThe Server 3 has TCP port number is %s and IP address is %s.\n ", MYPORT, s3);
	
	//now accept the connection	
	
	
	while(1)
	{  
		//main accept() loop
		//printf("server 3: waiting for connections...\n");
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); //Creating child socket
		if (new_fd == -1) 
		{
			perror("accept");
			continue;
		}//Source: Beej's Guide to Network Programming
		
		int client_port;
		client_size = sizeof their_addr;
		getpeername(sockfd, (struct sockaddr*)&their_addr, &client_size);
		client_port =(int)ntohs((struct sockaddr_in *)(&their_addr))->sin_port;
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s2, sizeof s2);
		
		//printf("server: got connection from %s\n", s);
		
		
		int numbytes;
		char buf[MAXDATASIZE];
		if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) 
		{
			perror("recv");
			exit(1);
		}//Source: Beej's Guide to Network Programming
		buf[numbytes] = '\0';
		
		//recvs a GET msg from s2, needs to remove GET//
		int founds3=0;
		
		char temp[10];
		strcpy(temp,buf);
		char *KSearch;
		const char d[2]=" ";
		char *token;
		token = strtok(temp, d);
		KSearch = strtok(NULL, d);
		printf("\n\n\nThe Server 3 has received a request with key %s from Server 2 with port number %d and IP address %s.\n", KSearch, client_port,s2);
		
		char s2msg[10]="POST ";
		for(i=0;i<end;i++)
		{
			if((strcmp(sd3[i].key,KSearch))==0)
			{ 
				founds3=1;
				strcat(s2msg,sd3[i].val);
				break;
			}
			
		} //end of for loop searching for key
		
		
		int buflen=strlen(s2msg);
		s2msg[buflen]='\0';
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s2, sizeof s2);
		if(z==1)
		{printf("\nThe Server 3 sends the reply %s to the Server 2 with port number %d and IP address %s.\n", s2msg, client_port, s2);}
		else
		{printf("\nThe Server 3, sent the reply %s to the Server 2 with port number %d and IP address %s.\n", s2msg, client_port, s2);}
		
		if (send(new_fd, s2msg, buflen, 0) == -1) //Source: Beej's Guide to Network Programming
		perror("send");
		close(new_fd);
		z++;
		//printf("\n-------------------------------------\n");
	} //end of while(1)
	
	close(sockfd);
	return 0;
}
