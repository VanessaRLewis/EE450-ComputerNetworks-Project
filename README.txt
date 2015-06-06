Socket Programming Project for EE 450

I coded in C for this project, which simulates a distributed hash system. I wrote 5 programs – one each for the 3 servers and 2 clients. I have completed all the three phases.
For the first phase, I made 2 types of structures – serverData and clientData for the servers and the clients respectively. 
Each serverData structure has a string “key” and a string “value”. I made an array of serverData structures for each Server. The array of serverData structures is loaded by reading the input files Server#.txt for the respective server.
Each clientData structure has a string “term” and “key”. I made an array of clientData for each client, which are filled in by reading from the input files Client#.txt. 

CODE FILES:
-client1.c: A UDP client that initially prompts the user for a search term and maps the entered search term to the corresponding key. Client 1, then sends this key to Server 1, so that the Server can search for the corresponding value. Once Client 1 receives the value corresponding to the search term from Server 1, it displays the value to the user.  
-client2.c: A UDP client that initially prompts the user for a search term and maps the entered search term to the corresponding key. Client 2, then sends this key to Server 1, so that the Server can search for the corresponding value. Once Client 2 receives the value corresponding to the search term from Server 1, it displays the value to the user. 
-dhtserver1.c: This is the designated server that the clients contact for the required value over a bidirectional UDP connection. When the Server 1 receives the key from the Client, it first searches whether it has the required value within its own data structure. If it does, it sends this value over the bidirectional UDP connection. If Server 1 doesn't have the value, it acts as a TCP Client and queries Server 2 for the required value over a TCP connection. Upon receipt of the required value from Server 2, Server 1 caches this value in its own data structure & sends the required value to the client over the bidirectional UDP connection.
-dhtserver2.c: Once this server is contacted by Server 1 for a value it searches its own data structure for the required value. If it has the value, Server 2 sends it to Server 1 over the TCP connection. Else, Server 2 opens a TCP connection with Server 3 and queries it with the key received from Server 1 over this TCP connection. Upon receipt of the required value from Server 3, Server 2 caches this value in its own data structure & sends the required value to Server 1 over the initial TCP connection.
-dhtserver3.c: This is a TCP server that is contacted by Server 2 for the required value. When Server 3 receives a key from Server 2, it searches this data structure for the corresponding value and sends this value to Server 2 over the TCP connection. 

TO COMPILE THE SOURCE CODE FILES, please enter the following commands:
gcc -o s3 dhtserver3.c -lsocket -lnsl -lresolv
gcc -o s2 dhtserver2.c -lsocket -lnsl -lresolv
gcc -o s1 dhtserver1.c -lsocket -lnsl -lresolv
gcc -o c1 client1.c -lsocket -lnsl -lresolv
gcc -o c2 client2.c -lsocket -lnsl -lresolv


TO RUN THESE FILES, open 5 nunki sessions, navigate to the folder and execute the following commands in 1 session each:
s1
s2
s3
c1
Once done with Client 1, the grader should execute Client 2 in the fifth nunki session by typing the following: 
c2

The grader will need to kill the Server 2 & Server 3 processes manually (by pressing Ctrl - C in the nunki sessions where Server 2 & Server 3 are running) once the Server 1 process terminates. Server 1 is programmed to run for only two iterations and will stop after it sends the value to Client 2. 

The format of all the messages exchanged: No other format is used.

Server 1 is programmed to run for only two iterations and will stop after it sends the required value to Client 2. However, the grader will need to kill the Server 2 & Server 3 processes manually (by pressing Ctrl - C in the nunki sessions where Server 2 & Server 3 are running) once the Server 1 process terminates; else the two processes will run infinitely.
The program also assumes that the graders would test with the right input, so I haven't handled cases where the user enters the search term in lowercase or enters search terms not in client1.txt ot client2.txt .

A lot of socket programing code has been reused from Beej's Guide to Network Programming, which has been marked in my source code files.
