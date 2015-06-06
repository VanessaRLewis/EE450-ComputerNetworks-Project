/************* UDP CLIENT CODE *******************/
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

#define UDPPORT "21348" //UDP port of s1 that client connects to
#define MAXBUFLEN 100 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{return &(((struct sockaddr_in*)sa)->sin_addr);}
	
	return &(((struct sockaddr_in6*)sa)->sin6_addr); 
}//Source: Beej's Guide to Network Programming

struct clientData
{
	char term[5];
	char key[7];
}cd2[12];

int main()
{
    ////////PHASE 1////////
    /*PHASE 1.1 - Read from file client1.txt and store in a clientData struct cd2*/
    FILE *fp;
    char t[5];
	char k[7];
	
	fp = fopen("client2.txt" , "r"); //opening file for reading
	if(fp == NULL)
	{
		perror("Error opening file");
		return(-1);
	}
	
	int i=0;
	while(fscanf(fp,"%s %s",t,k)!=EOF) //reading file
	{
		strcpy(cd2[i].term,t);
		strcpy(cd2[i].key,k);
		i++;
	}
	fclose(fp);
	
	
	/////* PHASE 1.2- MAKING UDP SOCKET */////
	int c2udpsockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	char s2[INET6_ADDRSTRLEN];
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if ((rv = getaddrinfo("nunki.usc.edu", UDPPORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}//Source: Beej's Guide to Network Programming
	
	//loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((c2udpsockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
		{
			perror("talker: socket");
			continue;
		}
		break;
	}//Source: Beej's Guide to Network Programming
	
	if (p == NULL)
	{
		fprintf(stderr, "talker: failed to bind socket\n");
		return 2;
	}
	
	////PHASE 2 : SEARCH MAPPING//
	char str[7];
	char skey[10];
	char s1msg[20] = "GET ";
	// while(1){
	printf("\nPlease Enter Your Search (USC, UCLA etc.): ");
	scanf("%s",str);
	for(i=0;i<12;i++) //Searching for corresponding key
	{
		if((strcmp(cd2[i].term,str))==0)
		{
			strcpy(skey,cd2[i].key);
			printf("\nThe Client 2 has received a request with search word %s, which maps to key %s.\n",str, skey);
		}
	}
	
	strcat(s1msg, skey);
	s1msg[strlen(s1msg)]='\0';
	
	/*Send message to server*/
	if ((numbytes = sendto(c2udpsockfd, s1msg, strlen(s1msg), 0, p->ai_addr, p->ai_addrlen)) == -1) 
	{
		perror("talker: sendto");
		exit(1);
	}//Source: Beej's Guide to Network Programming
	
	struct sockaddr_in udpaddr;
	socklen_t length2 = sizeof(udpaddr);
	if (getsockname(c2udpsockfd, (struct sockaddr *)&udpaddr, &length2) == -1)
	{
		perror("getsockname");
		exit(1);
	}
	
	//IP address of Server 1
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("\nThe Client 2 sends the request %s to the  Server 1 with port number %s and IP address %s.", s1msg, UDPPORT, s) ;
	freeaddrinfo(servinfo);
    
    //display c2 address
	int client_udpport = ntohs(udpaddr.sin_port);
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s2, sizeof s2);
	printf("\nThe Client 2's port number is %d and IP address is %s.\n", client_udpport, s2);
	
	numbytes = strlen(s1msg) + 2;
	
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(c2udpsockfd, s1msg, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) ==-1)
	{
		perror("recvfrom");
		exit(1);
	}
	s1msg[numbytes] = '\0';
	
	///extract POST from s1's msg//	
	char *VSearch;
	const char d[2]=" ";
	char *token;
	char temp2[20];
	strcpy(temp2,s1msg);
	token = strtok(temp2, d);
	VSearch = strtok(NULL, d);
	VSearch[strlen(VSearch)]='\0';
	
	printf("\nThe Client 2 received the value %s from the Server 1 with port number %s & IP address %s.", VSearch , UDPPORT, s);
	printf("\nThe Client 2's port number is %d and IP address is %s.\n", client_udpport, s2);
	
	close(c2udpsockfd);
	
	// } //end of while(1)
	return 0;
}
