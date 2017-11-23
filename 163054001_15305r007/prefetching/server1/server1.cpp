/*
FILE NAME: server1.cpp
CREATED BY:163054001 AMIT PATHANIA
			15305r007 Nivia Jatain
This files creates a central server. Server takes one commandline input arguments ie own listning port.
Clients will use Server IP and port to connect this server for fetching files.

*/
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <thread>
#include <cstdlib>
#include <string>
#include <fcntl.h> 
#include <unistd.h> 
#include <future>
#include <sys/stat.h>
#include <sys/mman.h>
#include <bits/stdc++.h>
using namespace std;


#define MAX_BUFFER   512 //used to set max size of buffer for send recieve data 

int MAX_FILES;
char testfile[20];
int start_index;
int file_size;
// Defining Constant Variables for database connections
//#define AUTH_SERVER_IP "127.0.0.1"
//#define AUTH_SERVER_PORT 20000

char AUTH_SERVER_PORT[MAX_BUFFER]; // authentication server port
char AUTH_SERVER_IP[MAX_BUFFER]; // auth server IP address
//char AUTH_SERVER_PORT[]="20000"; // authentication server port
//char AUTH_SERVER_IP[]="10.130.5.172"; // auth server IP address


time_t current_time; // variable to store current time

void *client_handler(void *);


int connect_to_auth_server(char*); // function to connect to authentication server
int upload_file(int , char*);
int prefetch_files();

int pfile_count;
char *m_file[100000];
struct request {
	char uname[20],upassword[20],type[20];
};



int main(int argc, char **argv)  
{
	int sock,new1; // socket descriptor for new client
	struct sockaddr_in server; //server structure 
	struct sockaddr_in client; //structure for server to bind to particular machine
	socklen_t sockaddr_len=sizeof (struct sockaddr_in);	//stores length of socket address
	int *new_sock;
	int rc,c;

	if (argc < 2)    // check whether port number provided or not
	{ 
		fprintf(stderr, "ERROR, no listening port provided\n");
		exit(1);
	}
	

	printf("Enter Authentication server IP   ");
	scanf(" %[^\t\n]s",AUTH_SERVER_IP);
	printf("Enter Authentication server listening port  ");
	scanf(" %[^\t\n]s",AUTH_SERVER_PORT);
	printf("Enter testfile name PREFIX(1KB/100KB/1MB....)   ");
	scanf(" %[^\t\n]s",testfile);
	printf("Enter maximum number of files(<100000)  ");
	cin>>MAX_FILES;
	
	if (strcmp(testfile,"1KB")==0 || strcmp(testfile,"1MB")==0 || strcmp(testfile,"1GB")==0)    // check whether port number provided or not
	{ 
		start_index=3;
		
	}
	if (strcmp(testfile,"10KB")==0 || strcmp(testfile,"10MB")==0 || strcmp(testfile,"10GB")==0)    // check whether port number provided or not
	{ 
		start_index=4;
	}
	if (strcmp(testfile,"100KB")==0 || strcmp(testfile,"100MB")==0 || strcmp(testfile,"100GB")==0)    // check whether port number provided or not
	{ 
		start_index=5;
	}
	//scanf(" %d\n",MAX_FILES);

	//scanf(" %[^\t\n]s",MAX_FILES);
	//char* m_file[MAX_FILES];
	


	/*get socket descriptor */
	if ((sock= socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{ 
		perror("server socket error: ");  // error checking the socket
		exit(-1);  
	} 
	 
	/*server structure */ 
	server.sin_family = AF_INET; // protocol family
	server.sin_port =htons(atoi(argv[1])); // Port No and htons to convert from host to network byte order. atoi to convert asci to integer
	server.sin_addr.s_addr = INADDR_ANY;//INADDR_ANY means server will bind to all netwrok interfaces on machine for given port no
	bzero(&server.sin_zero, 8); //padding zeros


	// set SO_REUSEADDR on a socket to true (1):
	// kill "Address already in use" error message
	int opt;
	if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
	}
	
	
	/*binding the socket */
	if((bind(sock, (struct sockaddr *)&server, sockaddr_len)) == -1) //pointer casted to sockaddr*
	{
		perror("bind");
		exit(-1);
	}

	printf("SERVER STARTED...... \n ");
	/*listen the incoming connections */
	if((listen(sock, -1)) == -1) // listen for max connections
	{
		perror("listen");
		exit(-1);
	}

	c = sizeof(struct sockaddr_in);
	pthread_t sniffer_thread;
	pthread_attr_t attr;
	
	prefetch_files();
	
	//int cnt=0;
	while((new1 = accept(sock, (struct sockaddr *)&client, (socklen_t*)&c)))
	{
		//printf("New client connection accepted \n");
		new_sock = new int(1);
        *new_sock = new1;

		
		
		// Initialize 
	   rc=pthread_attr_init(&attr);
	   if (rc) {
	        perror("Error:unable to initilaise thread attribute" );
	   	 	exit(-1);
	     }
	  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	   
         
        if( pthread_create( &sniffer_thread , &attr,  client_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
       //cnt++;
       //cout<<"Handler assigned to new client " <<cnt<<endl;
        //Now join the thread , so that we dont terminate before the thread
       //pthread_join( sniffer_thread , NULL);
       // printf("New client connected from port no %d and IP %s\n", ntohs(client.sin_port), inet_ntoa(client.sin_addr));
	 	
       // printf("Handler assigned to new client");
            rc=pthread_attr_destroy(&attr);
		   if (rc != 0)
		   {
		      perror("pthread_attr_destroy");
		   }   

    }

		       
    
   

    if ((new1 < 0)) // accept takes pointer to variable containing len of struct
		{
			perror("ACCEPT.Error accepting new connection");
			exit(-1);
		}

	

     return 0;
   }


/*
 * This will handle connection for each client
 * 
 */
void *client_handler(void *socket_cl)
{
    //Get the socket descriptor
    int new1 = *(int*)socket_cl;
    struct sockaddr_in client;
    socklen_t sin_size = sizeof(client);

    char buffer[MAX_BUFFER]; // Receiver buffer; 
	//char file_name[MAX_BUFFER];//Buffer to store filename,path and port recieved from client
	//char *client_ip;//variable to store IP address of client
	
	int success=0;

	//variable for authenticating existinguser operation
	char file_name[MAX_BUFFER];//file keyword for search by user
	int len;// variable to measure MAX_BUFFER of incoming stream from user
	
   
    
    //int res = getpeername(new1, (struct sockaddr *)&client, &sin_size);
    //char *client_ip ;
    //client_ip=inet_ntoa(client.sin_addr);
    //printf(client_ip);
    //strcpy(client_ip, inet_ntoa(client.sin_addr));
		
	
	 	
		while(1)
		{
			//bzero(buffer,MAX_BUFFER);
		len=recv(new1, buffer , MAX_BUFFER, 0);
		buffer[len] = '\0';
		//printf("%s\n",buffer);

		//conenctionerror checking
			if(len<=0)//connection closed by client or error
				{
				if(len==0)//connection closed
				{
					printf("client %s hung up\n",inet_ntoa(client.sin_addr));
					
					return 0;
					
				}
				else //error
				{
					perror("ERROR IN RECIEVE");
					return 0;
				}
			
			}//clos if loop

			int i,j;
			char str[256];

			//ADD NEW USER OPERATION
		if(buffer[0]=='n' && buffer[1]=='e' && buffer[2]=='w') // check if user wants to publish a file
		{
			
        success = connect_to_auth_server(buffer);
        bzero(buffer,MAX_BUFFER);


		if (success)
		{
			strcpy(buffer,"Successfull");
			send(new1,buffer,MAX_BUFFER,0);	
		//printf("%s\n",Report3);
		
		}
		else
		{
			strcpy(buffer,"Unsuccessfull ");
			send(new1,buffer,MAX_BUFFER,0);	
		
		
		}
		//printf("Closing connection\n");
	    //printf("Client disconnected from port no %d and IP %s\n", ntohs(client.sin_port), 	inet_ntoa(client.sin_addr));
		//client_ip = inet_ntoa(client.sin_addr); // return the IP
			
		close(new1); /* close connected socket*/
	    return 0;

		}


		

		//AUTHENTICATE USER AND GET EVENTS
		else if(buffer[0]=='g' && buffer[1]=='e' && buffer[2]=='t') //check keyword for search sent by client
		{
			
			success = connect_to_auth_server(buffer);
			

			bzero(buffer,MAX_BUFFER);
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
		if (success)
			{
				
				strcpy(buffer,"Successfull");
				send(new1,buffer,MAX_BUFFER,0);
				bzero(buffer,MAX_BUFFER);
				
			
				//char rec_buff[MAX_BUFFER]; /* to store received string */
				//ssize_t len; /* bytes received from socket */
				/* read name of requested file from socket */
				if ( (len = recv(new1, buffer, MAX_BUFFER, 0)) < 0) {
				perror("Error in recieve");
					}
				sscanf (buffer, "%s\n", file_name); /* discard CR/LF */
				bzero(buffer,MAX_BUFFER);
				//puts(file_name);
			    upload_file(new1, file_name);
			    		 

			}
		else
		{
			strcpy(buffer,"Unsuccessfull");
			send(new1,buffer,MAX_BUFFER,0);
			bzero(buffer,MAX_BUFFER);

		}
					
		close(new1); /* close connected socket*/
	    return 0;
		}// close get condition


		
		
			//TERMINATE OPERATION:when user want to disconnect from server
		else if(buffer[0]=='t' && buffer[1]=='e' && buffer[2]=='r')
		{

			char ack[]="Request_received";
			send(new1, ack, sizeof(ack),0); // recieve confirmation message from server
			//printf("Client disconnected from port no %d and IP %s\n", ntohs(client.sin_port), 
	//inet_ntoa(client.sin_addr));
			//client_ip = inet_ntoa(client.sin_addr); // return the IP
			
			//printf("Closing connection\n");
			close(new1); /* close connected socket*/
        
	    
			return 0;
			
		} //close terminate loop
	
	}// close while loop inside fork.server will keep listening client till disconnected
	
	
       close(new1);   
    //Free the socket pointer
    free(socket_cl);
     
    return 0;
}

int prefetch_files(){

int r=0;
char file_name[20];
strcpy(file_name,testfile);
char count[32];
int fd;
char file_name_full[MAX_BUFFER];
char *send_buf[MAX_BUFFER]; 
ssize_t read_bytes;
struct stat sb;

while(r<MAX_FILES)
	{
		//cout<<r;
		r++;
		sprintf(count, "%d", r);
		strcpy(file_name_full,"uploads/");
		strcat(file_name_full,file_name);
		strcat(file_name_full,count);
		strcat(file_name_full,".txt");
		

if( (fd = open(file_name_full, O_RDONLY|O_DIRECT)) < 0) 
	{
		
		perror(file_name_full );
		return 0;

	}
	else 
	{
		if (fstat(fd, &sb) == -1)           // To obtain file size 
		{
			perror("fstat");
		}
		file_size=sb.st_size;
		
		char *source = NULL;
		 // Go to the end of the file. 
		
		//lseek(fd, 0, SEEK_END);
		//long file_size = ftell(fd);
		//lseek(fd, 0, SEEK_SET);  //same as rewind(f);

		//posix_memalign(&source,512,file_size);
		source = (char*) malloc(file_size);
		if (source == 0)
		{
			printf("ERROR: Out of memory\n");
			return 0;
		}

    	    
        size_t read_bytes= read(fd,source, file_size);
        if (read_bytes== 0) {
            fputs("Error reading file", stderr);
        } else {
            //source[++read_bytes] = '\0'; 
            //cout<<"bytes read" <<read_bytes<<endl;
        }
       
		close(fd);
		m_file[r-1]=source;
		pfile_count++;
		

          }

       
     
	//printf("File sent to client: %s\n", file_name_full);
	} 

puts("Files prefetched");
return 1;
}



int upload_file(int sock, char *file_name)
{

	ssize_t read_bytes, sent_bytes, sent_file_size; 
	off_t offset=0;
    size_t length=MAX_BUFFER;
	//int sent_counter; // to count number of packets transfered
    
	char send_buf[MAX_BUFFER]; //buffer to store the data read from file
	//char * errmsg_notfound = "File not found\n";
	int fp; //file descriptor
	int r,j = 0;	
	char str[10];
	
	char file_name_full[MAX_BUFFER];
	strcpy(file_name_full,"uploads/");
	strcat(file_name_full,file_name);
	sent_file_size = 0;
	//int file_size;
	//struct stat sb;
	char* addr=NULL;

/*
	if (fstat(fd, &sb) == -1)           // To obtain file size 
		{
			perror("fstat");
		}
	file_size=sb.st_size;
	addr = (char*) malloc(file_size);
	if (addr == 0)
		{
			printf("ERROR: Out of memory\n");
			return 1;
		}

    */
	
	//bzero(str,10);


	
	for( int i = start_index;file_name[i] != '.' ; i++ )
		str[j++]=file_name[i];
		//puts(str);
	//sscanf(str, "%d", &r);
	r=atoi(str);
   
    if (r>pfile_count)
    {
    	perror("File not prefetched");
    	 return -1;
    }

   //strncpy(addr,m_file[r-1],file_size);
   	addr=m_file[r-1];
    while(sent_file_size<=file_size)
      {

      	if (offset + length > file_size)
      		length = file_size - offset;                      //Can't display bytes past end of file 

      	
         bzero(send_buf,MAX_BUFFER);
         /*
         for(int i=offset;i<(offset+length);i++)
			 {
              	send_buf[j++]=addr[i];

             }
          */
          memcpy(send_buf,addr+offset,length);    
          sent_bytes = send(sock, send_buf, sizeof(send_buf),0);
              
       	sent_file_size +=sent_bytes;
     	offset+=length;
        //cout<<"Sent" <<sent_bytes <<" bytes " <<sent_file_size <<endl;
      	//bzero(send_buf,MAX_BUFFER);
      }
      //free(addr);
      //cout<<"Sent" <<sent_bytes <<" bytes " <<sent_file_size <<endl;
	return sent_bytes;
}





int connect_to_auth_server(char* buffer)
{

	int auth_sock; // auth_sock is socket desriptor for connecting to remote server 
	struct sockaddr_in auth_server; // contains IP and port no of remote server
	char send_buff[MAX_BUFFER];  //user input stored 
	char receive_buff[MAX_BUFFER]; //recd from remote server
	int len;//to measure length of recieved input stram on TCP
	
	if ((auth_sock= socket(AF_INET, SOCK_STREAM, 0)) <0)
	{ 
		perror("Problem in creating socket");  // error checking the socket
		exit(-1);  
	} 
	  
	memset(&auth_server,0,sizeof(auth_server));
	auth_server.sin_family = AF_INET; // family
	auth_server.sin_port =htons(atoi(AUTH_SERVER_PORT)); // Port No and htons to convert from host to network byte order. 
	auth_server.sin_addr.s_addr = inet_addr((AUTH_SERVER_IP));//IP addr in ACSI form to network byte order converted using inet
	bzero(&auth_server.sin_zero, 8); //padding zeros
	

	if((connect(auth_sock, (struct sockaddr *)&auth_server,sizeof(auth_server)))  == -1) //pointer casted to sockaddr*
	{
		perror("Problem in connect");
		exit(-1);
	}
	//printf("%s","\n Connected to Authentication server\t \n");

	bzero(send_buff,MAX_BUFFER);
	strcpy(send_buff, buffer);
	//strcat(send_buff,":");
	//strcat(send_buff,user_name);
	//strcat(send_buff,":");
	//strcat(send_buff,user_key);

	
	send(auth_sock, send_buff, sizeof(send_buff),0); // send input to server
	len = recv(auth_sock, receive_buff, MAX_BUFFER, 0); // recieve confirmation message from server
	receive_buff[len] = '\0';
	//printf("%s\n" , receive_buff); // display confirmation message
	 // pad MAX_BUFFER with zeros


	//printf("Connection terminated with AUTHENTICATION server.\n");
	close(auth_sock);
	//cout<<numrows<<endl;
    if(receive_buff[0]=='S' && receive_buff[1]=='u' && receive_buff[2]=='c') 

       {
       	      	return 1;
       }
    else
      {
      	      	return 0;
      }
      bzero(receive_buff,MAX_BUFFER);
     // bzero(receive_buff,MAX_BUFFER); // pad MAX_BUFFER with zeros




}

