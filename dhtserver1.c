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

#define MYUDPPORT "21348" //port that clients will be connecting to
#define MAXBUFLEN 100
#define TCPPORT "22348" // the TCPPORT server1 will be connecting to ,ie, server2
#define MAXDATASIZE 100 // max number of bytes we can get at once 

struct serverData
{
	char key[7];
	char val[15];
}sd1[12];

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
	///////PHASE 1///////
	/*read from server1.txt & storage in a struct*/
	FILE *fp;
	fp = fopen("server1.txt","r");
	if(fp == NULL)
	{
		perror("Error opening file server1.txt");
		return(-1);
	}
	
	char k[7];
	char v[8];
	int i=0;
	
	while(fscanf(fp,"%s %s",k,v)!=EOF)
	{
		strcpy(sd1[i].key,k);
		strcpy(sd1[i].val,v);
		i++;
	}
	
	fclose(fp);
	
	int end = 4;
	//printf("Last entry in server is %d\n",end);
	
	/************* UDP SERVER CODE *******************/
	int s1udpsockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len, client_size;
	char s[INET6_ADDRSTRLEN];
	char c[INET6_ADDRSTRLEN];
	
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	
	/*getting nunki's address */
	if ((rv = getaddrinfo("nunki.usc.edu", MYUDPPORT, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}//Source: Beej's Guide to Network Programming
	
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((s1udpsockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("listener: socket");
			continue;
		}
		
		if (bind(s1udpsockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(s1udpsockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}//Source: Beej's Guide to Network Programming
	
	if (p == NULL)
	{
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
	
	void *addr;
	struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
	addr = &(ipv4->sin_addr);
	inet_ntop(p->ai_family, addr, s, sizeof s);
	printf("\nThe Server 1 has UDP port number %s and IP address %s.\n", MYUDPPORT, s);
	freeaddrinfo(servinfo);
	
	addr_len = sizeof their_addr;
	int z=1;
	while(z<=2)
	{
		//printf("listener: waiting to recvfrom client %d ...\n",z);
		
		if ((numbytes = recvfrom(s1udpsockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
		{
			perror("recvfrom");
			exit(1);
		}//Source: Beej's Guide to Network Programming
		
		int client_port;
		client_size = sizeof their_addr;
		getpeername(s1udpsockfd, (struct sockaddr*)&their_addr, &client_size);
		client_port =(int)ntohs((struct sockaddr_in *)(&their_addr))->sin_port;
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), c, sizeof c);
		
		buf[numbytes] = '\0';
		
		/*Search for corresponding value */
		char c1msg[20]="POST ";
		char temp[10];
		char *KSearch;
		const char d[2]=" ";
		char *token;
		
		strcpy(temp,buf);
		token = strtok(temp, d);
		KSearch = strtok(NULL, d);
		int founds1=0;
		printf("\n\n\nThe Server 1 has received a request with key %s from Client %d with port number %d and IP address %s.\n", KSearch, z, client_port,c);
		
		//case 1
		for(i=0;i<end;i++)
		{
			if((strcmp(sd1[i].key,KSearch))==0)
			{   founds1=1;
				strcat(c1msg,sd1[i].val);
				printf("\nThe Server 1 sends the reply %s to Client %d with port number %d and IP address %s.\n", c1msg, z, client_port,c);
				break;
			}
		} //end of for loop searching for key
		
		if(founds1==0)
		{ 
			//case 2
			//needs to sends msg to server2 over dynamic tcp
			/*************************TCP SENDING SOCKET PART***************************/
			
			int s1tcpsockfds2, numbytes2;  
			char buf2[MAXDATASIZE];
			struct addrinfo hints2, *servinfo2, *p2;
			int rv2;
			socklen_t addr_len2, client_size2;
			char s2[INET6_ADDRSTRLEN];
			
			
			memset(&hints2, 0, sizeof hints2);
			hints2.ai_family = AF_UNSPEC;
			hints2.ai_socktype = SOCK_STREAM;
			
			if ((rv2 = getaddrinfo("nunki.usc.edu", TCPPORT, &hints2, &servinfo2)) != 0) {
				fprintf(stderr, "getaddrinfo: %s2\n", gai_strerror(rv2));
				return 1;
			}//Source: Beej's Guide to Network Programming
			
			// loop through all the results and connect to the first we can
			for(p2 = servinfo2; p2 != NULL; p2 = p2->ai_next) {
				if ((s1tcpsockfds2 = socket(p2->ai_family, p2->ai_socktype,
				p2->ai_protocol)) == -1) {
					perror("client: socket");
					continue;
				}
				
				if (connect(s1tcpsockfds2, p2->ai_addr, p2->ai_addrlen) == -1) {
					close(s1tcpsockfds2);
					perror("client: connect");
					continue;
				}
				
				break;
			}//Source: Beej's Guide to Network Programming
			
			if (p2 == NULL) {
				fprintf(stderr, "client: failed to connect\n");
				return 2;
			}//Source: Beej's Guide to Network Programming
			
			struct sockaddr_in addr2;
			socklen_t length = sizeof(addr2);
			if (getsockname(s1tcpsockfds2, (struct sockaddr *)&addr2, &length) == -1)
			{
				perror("getsockname");
				exit(1);
			}
			
			
			char s2msg[20] = "GET ";
			strcat(s2msg, KSearch);
			int len = strlen(s2msg);
			printf("\n\nThe Server 1 sends the request %s to the Server 2.\n",s2msg);
			
			
			int client_port2 = ntohs(addr2.sin_port);
			//inet_ntop(p2->ai_family, get_in_addr((struct sockaddr *)p2->ai_addr),s2, sizeof s2);
			printf("\nThe TCP port number is %d and IP address is %s.\n", client_port2, s);
			freeaddrinfo(servinfo2); // all done with this structure	
			
			if (send(s1tcpsockfds2, s2msg, len, 0) == -1)
			perror("send");	
			//Source: Beej's Guide to Network Programming
			
			/////reply from s2/////
			
			if ((numbytes2 = recv(s1tcpsockfds2, buf2, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
				exit(1);
			}//Source: Beej's Guide to Network Programming
			
			buf2[numbytes2] = '\0';
			inet_ntop(p2->ai_family, get_in_addr((struct sockaddr *)p2->ai_addr),s2, sizeof s2);
			
			///extract POST from s2's msg//	
			char *VSearch;
			char temp2[20];
			strcpy(temp2,buf2);
			token = strtok(temp2, d);
			VSearch = strtok(NULL, d);
			printf("\nThe Server 1 received the value %s from the Server 2 with port number %s and IP address %s.\n", VSearch, TCPPORT, s2);
			char va2[10];
			//putting in sd1	
			strcpy(sd1[end].key,KSearch);
			strcpy(va2,VSearch);
			strcpy(sd1[end].val,va2);
			
			end++;
			founds1=1;// cos now u have got value from s3
			//making c1msg POST
			strcat(c1msg,VSearch);
			c1msg[strlen(c1msg)]='\0';
			close(s1tcpsockfds2);  
			
			printf("\nThe Server 1 closed the TCP connection with the Server 2.\n");
			
			printf("\n\nThe Server 1 sent the reply %s to the Client %d with port number %d and IP address %s.\n", c1msg, z, client_port,c);
			
			/***********************END OF TCP SOCKET SENDING PART***************************/
			
		} //end if NOT FOUND AT SERVER1
		
		numbytes=0;
		
		if ((numbytes = sendto(s1udpsockfd, c1msg, strlen(c1msg), 0, (struct sockaddr *)&their_addr, addr_len)) == -1) 
		{
			perror("talker: sendto");
			exit(1);
		}//Source: Beej's Guide to Network Programming
		
		z++;
	}//end of while 1 for UDP
	
	close(s1udpsockfd);
	
	return 0;
}
