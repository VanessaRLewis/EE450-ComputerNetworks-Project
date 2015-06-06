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

#define MYPORT "22348" // s2's tcp port that server1 will be connecting to 
#define MAXDATASIZE 100 // max number of bytes we can get at once
#define TCPPORT "23348" // the TCPPORT server2 will be connecting to ,ie, server3
#define BACKLOG 10

struct serverData
{
	char key[7];
	char val[8];
}sd2[8];

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
	fp = fopen("server2.txt","r");
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
		strcpy(sd2[i].key,k);
		strcpy(sd2[i].val,v);
		i++;
	}
	
	fclose(fp);
	int end = 4;
	
	/*******Case 2- CREATING TCP LISTENING PORT********/
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // client's address information
	socklen_t sin_size, client_size2;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	char s2[INET6_ADDRSTRLEN];
	
	int rv;
	char buf[MAXDATASIZE];
	char buf2[MAXDATASIZE];
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	//hints.ai_flags = AI_PASSIVE; // use my IP
	
	if ((rv = getaddrinfo("nunki.usc.edu", MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}//Source: Beej's Guide to Network Programming
	
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
		p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
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
	
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}//Source: Beej's Guide to Network Programming
	
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}//Source: Beej's Guide to Network Programming
	
	//displaying IP Adressing
	void *addr;
	struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
	addr = &(ipv4->sin_addr);
	inet_ntop(p->ai_family, addr, s2, sizeof s2);
	
	printf("\nThe Server 2 has TCP port number %s and IP address %s.\n", MYPORT, s2);
	
	//now accept the connection	
	while(1) 
	{  
		//main accept() loop
		//printf("server 2: waiting for connections...\n");
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); //Creating child socket
		if (new_fd == -1) {
			perror("accept");
			continue;
		}//Source: Beej's Guide to Network Programming
		
		
		int client_port;
		client_size2 = sizeof their_addr;
		getpeername(sockfd, (struct sockaddr*)&their_addr, &client_size2);
		client_port =(int)ntohs((struct sockaddr_in *)(&their_addr))->sin_port;
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		
		int numbytes;
		if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) 
		{
			perror("recv");
			exit(1);
		}//Source: Beej's Guide to Network Programming
		
		//recv from s1 - need to remove get 
		buf[numbytes] = '\0';
		int founds2=0;
		char temp[10];
		strcpy(temp,buf);
		char *KSearch;
		const char d[2]=" ";
		char *token;
		token = strtok(temp, d);
		KSearch = strtok(NULL, d);
		printf("\n\n\nThe Server 2 has received a request with key %s from Server 1 with port number %d and IP address %s.\n", KSearch, client_port,s);
		
		char s1msg[10]="POST ";
		
		/*Search for corresponding value */
		for(i=0;i<end;i++) //case 2 - making POST to s1 if there in s2
		{   
			if((strcmp(sd2[i].key,KSearch))==0)
			{ 	founds2=1;
				strcat(s1msg,sd2[i].val);
				s1msg[strlen(s1msg)]='\0';
				printf("\nThe Server 2 sends the reply %s to the Server 1 with port number %d and IP address %s.\n",s1msg,client_port,s);
				break;
			} 
		} //end of for loop searching for key
		
		if(founds2==0)
		{ 
			//Case 3 - tcp with s3
			/*************************TCP SENDING SOCKET PART***************************/
			int s2tcpsockfds3, numbytes2;  
			struct addrinfo hints2, *servinfo2, *p2;
			int rv2;
			//socklen_t addr_len2, client_size2;
			char s3[INET6_ADDRSTRLEN];
			
			
			memset(&hints2, 0, sizeof hints2);
			hints2.ai_family = AF_UNSPEC;
			hints2.ai_socktype = SOCK_STREAM;
			
			if ((rv2 = getaddrinfo("nunki.usc.edu", TCPPORT, &hints2, &servinfo2)) != 0) {
				fprintf(stderr, "getaddrinfo: %s2\n", gai_strerror(rv2));
				return 1;
			}//Source: Beej's Guide to Network Programming
			
			// loop through all the results and connect to the first we can
			for(p2 = servinfo2; p2 != NULL; p2 = p2->ai_next)
			{
				if ((s2tcpsockfds3 = socket(p2->ai_family, p2->ai_socktype,
				p2->ai_protocol)) == -1) 
				{
					perror("client: socket");
					continue;
				}
				
				if (connect(s2tcpsockfds3, p2->ai_addr, p2->ai_addrlen) == -1) {
					close(s2tcpsockfds3);
					perror("client: connect");
					continue;
				}
				
				break;
			}//Source: Beej's Guide to Network Programming
			
			if (p2 == NULL)
			{
				fprintf(stderr, "client: failed to connect\n");
				return 2;
			}//Source: Beej's Guide to Network Programming
			
			struct sockaddr_in addr2;
			socklen_t length = sizeof(addr2);
			if (getsockname(s2tcpsockfds3, (struct sockaddr *)&addr2, &length) == -1)
			{
				perror("getsockname");
				exit(1);
			}
			
			//need to append GET N SEND TO S3//
			char s3msg[20] = "GET ";
			strcat(s3msg, KSearch);
			int len2 = strlen(s3msg);
			printf("\n\nThe Server 2 sends the request %s to the Server 3.\n",s3msg);
			
			if (send(s2tcpsockfds3, s3msg, len2, 0) == -1)
			perror("send");	
			
			int client_port2 = ntohs(addr2.sin_port);
			inet_ntop(p2->ai_family, get_in_addr((struct sockaddr *)p2->ai_addr),s2, sizeof s2);
			printf("\nThe TCP port number is %d and IP address is %s.\n", client_port2, s2);
			freeaddrinfo(servinfo2); // all done with this structure
			
			
			///Receives value from s3//
			if ((numbytes2 = recv(s2tcpsockfds3, buf2, MAXDATASIZE-1, 0)) == -1) 
			{
				perror("recv");
				exit(1);
			}//Source: Beej's Guide to Network Programming
			
			buf2[numbytes2] = '\0';
			//printf("\nrecv buf2 from server 3: \n",buf2);
			
			inet_ntop(p2->ai_family, get_in_addr((struct sockaddr *)p2->ai_addr),s3, sizeof s3);
			
			///extract POST from s3's msg//	
			char *VSearch;
			char temp2[20];
			strcpy(temp2,buf2);
			token = strtok(temp2, d);
			VSearch = strtok(NULL, d);
			printf("\nThe Server 2 received the value %s from the Server 3 with port number %s and IP address %s.\n", VSearch, TCPPORT, s3);
			//putting in sd2	
			strcpy(sd2[end].key,KSearch);
			strcpy(sd2[end].val,VSearch);
			
			end++;
			founds2=1;// cos now u have got value from s3
			
			//making s1msg POST
			strcat(s1msg,VSearch);
			close(s2tcpsockfds3); 
			printf("\nThe Server 2 closed the TCP connection with the Server 3.\n");
			printf("\n\nThe Server 2, sent reply %s to the Server 1 with port number %d and IP address %s.\n", s1msg, client_port,s);
			/***********************END OF TCP SOCKET SENDING PART***************************/
		} //end if NOT FOUND AT SERVER2
		
		
		int buflen=strlen(s1msg);
		s1msg[buflen]='\0';
		if (send(new_fd, s1msg, buflen, 0) == -1)//Source: Beej's Guide to Network Programming
		perror("send");
		
		close(new_fd);
		//printf("\n------------------------------------------------------\n");
	} //end of while(1)
	close(sockfd);
	return 0;
}
