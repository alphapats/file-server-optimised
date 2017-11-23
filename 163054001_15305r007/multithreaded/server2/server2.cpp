/*
FILE NAME: server2.cpp
CREATED BY:163054001 AMIT PATHANIA
			15305r007 Nivia Jatain
This files creates a authentication server. Server takes one commandline input arguments ie own listning port.
Server1 will use Server IP and port to connect this server for authentication users.

*/
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
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
#include <cstdlib>
#include <string>
#include <fcntl.h>
#include <unistd.h> 
#include <sys/wait.h> 
// For MySQL Connection
#include <mysql/mysql.h>
using namespace std;

#define ERROR     -1  // defines error message
#define MAX_BUFFER   512 //used to set max size of buffer for send recieve data 
#define THREADSTACK  65536
// Defining Constant Variables for database connections
#define SERVER "localhost"
#define USER "root"
#define PASSWORD "root"
#define DATABASE "ems_data"




//funtions for database handling

int add_newuser(string, string);
int authenticate_user(string, string);
void *client_handler(void *);
int test_connection();

struct request {
	char uname[20],upassword[20],type[10];
};


int main(int argc, char **argv)  
{
	int sock; // sock is socket desriptor for server 
	int new1; // socket descriptor for new client
	struct sockaddr_in server; //server structure 
	struct sockaddr_in client; //structure for server to bind to particular machine
	socklen_t sockaddr_len=sizeof (struct sockaddr_in);	//stores length of socket address
	int *new_sock;

	// Receiver buffer; 

	int success=0;

	//variable for authenticating existinguser operation
	
	int len;// variable to measure MAX_BUFFER of incoming stream from user
	int rc,c;
     
	

	if (argc < 2)    // check whether port number provided or not
	{ 
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}


	/*get socket descriptor */
	if ((sock= socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
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
	if((bind(sock, (struct sockaddr *)&server, sockaddr_len)) == ERROR) //pointer casted to sockaddr*
	{
		perror("bind");
		exit(-1);
	}

	printf ("SERVER STARTED........................\n");
	test_connection();
	/*listen the incoming connections */
	if((listen(sock, 60000)) == ERROR) // listen for max connections
	{
		perror("listen");
		exit(-1);
	}

	c = sizeof(struct sockaddr_in);

	pthread_t sniffer_thread;
	pthread_attr_t attr;
		// Initialize 
	rc=pthread_attr_init(&attr);
	if (rc) {
	        perror("Error:unable to initilaise thread attribute" );
	   	 	exit(-1);
	     }

	while((new1 = accept(sock, (struct sockaddr *)&client, (socklen_t*)&c)))
	{
		//printf("New client connection accepted \n");
		new_sock = new int(1);
        *new_sock = new1;

       
		
	   //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
       pthread_attr_setstacksize(&attr, THREADSTACK);
         
        if( pthread_create( &sniffer_thread , &attr ,  client_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
       // pthread_join( sniffer_thread , NULL);
       // printf("New client connected from port no %d and IP %s\n", ntohs(client.sin_port), inet_ntoa(client.sin_addr));
	 	
       // printf("Handler assigned to new client");
        

    }
    rc=pthread_attr_destroy(&attr);
		   if (rc != 0)
		   {
		      perror("pthread_attr_destroy");
		   }
		  
     

    if ((new1 < 0)) // accept takes pointer to variable containing len of struct
		{
			perror("ACCEPT.Error accepting new connection");
			exit(-1);
		}

	

     return 0;
   }


void *client_handler(void *socket_cl)
{
	int new1 = *(int*)socket_cl;
    char *temp;
    

    //variables for add new user operation
	char buffer[MAX_BUFFER]; // Receiver buffer; 
	//char file_name[MAX_BUFFER];//Buffer to store filename,path and port recieved from client
	//char *client_ip;//variable to store IP address of client
	
	int success=0;
	
	int len;// variable to measure MAX_BUFFER of incoming stream from user
     
    struct sockaddr_in client;
    socklen_t sin_size = sizeof(client);
    int res = getpeername(new1, (struct sockaddr *)&client, &sin_size);
    char *client_ip ;
    client_ip=inet_ntoa(client.sin_addr);
    
    while(1)
		{
			bzero(buffer,MAX_BUFFER);
			len=recv(new1, buffer , MAX_BUFFER, 0);
			buffer[len] = '\0';
			

		//conenctionerror checking
			if(len<=0)//connection closed by client or error
				{
				if(len==0)//connection closed
				{
					printf("Peer %s hung up\n",inet_ntoa(client.sin_addr));
				}
				else //error
				{
					perror("ERROR IN RECIEVE");
				}
			close(new1);//closing this connection
			exit (0);
				}


		int i,j;
		char str[256];
			//ADD NEW USER OPERATION
		if(buffer[0]=='n' && buffer[1]=='e' && buffer[2]=='w') // check if user wants to publish a file
		{
	
			struct request newreq1;
		success=0;
		strcpy(newreq1.type, "new");
		
		bzero(str,128);
		j = 0;	
		for( i = 4;	buffer[i] != ':' ; i++ )
				str[j++]=buffer[i];
		buffer[j]=0;
		strcpy(newreq1.uname, str);
			//cout<<newreq.uname;

		for( i = j;	buffer[i] != ':' ; i++ )
			str[j++]=buffer[i];
		buffer[j]=0;
			
		strcpy(newreq1.upassword, str);

		
		//get_user_details(new1, user_name, user_password);
		//cout<<user_name<<user_password;
        
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//INSERT CODE TO ADD USERS TO DATABASE

        success=add_newuser(newreq1.uname,newreq1.upassword);
        //cout<<"Success "<<success<<endl;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		bzero(buffer,MAX_BUFFER);
		if (success)
		{
		char Report3[] = "Success"; 
		send(new1,Report3,sizeof(Report3),0);	
		
	
		}
		else
		{
		char Report3[] = "Unsuccessfull "; 
		send(new1,Report3,sizeof(Report3),0);
		//printf("%s\n",Report3);	
		
		}
		//printf("Closing connection\n");
	   // printf("Client disconnected from port no %d and IP %s\n", ntohs(client.sin_port), 
	//inet_ntoa(client.sin_addr));
		//client_ip = inet_ntoa(client.sin_addr); // return the IP
		
		//printf("Closing connection");
		bzero(buffer,MAX_BUFFER);
		close(new1); /* close connected socket*/
	    return 0;
	   
		}


		

		//AUTHENTICATE USER AND GET EVENTS
		else if(buffer[0]=='g' && buffer[1]=='e' && buffer[2]=='t') //check keyword 
		{
			
			struct request newreq;
		success=0;
		
		bzero(str,128);
		    	
		j = 0;	
		for( i = 4;	buffer[i] != ':' ; i++ )
			{
				
				str[j++]=buffer[i];
				
			}

		buffer[j]=0;
		strcpy(newreq.uname, str);
		//cout<<newreq.uname<<endl;
		
		for( i = j;	buffer[i] != ':' ; i++ )
			str[j++]=buffer[i];
		buffer[j]=0;
			
		strcpy(newreq.upassword, str);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		
		//INSERT CODE TO AUTHENTICATE USER FROM DATABASE
		success=authenticate_user(newreq.uname,newreq.upassword);
		//cout<<"Success "<<success<<endl;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		bzero(buffer,MAX_BUFFER);
		if (success)
			{
				char Report3[] = "Successfull "; 
				send(new1,Report3,sizeof(Report3),0);
				
			}
		else
		{
			char Report3[] = "Unsuccessfull ";
			send(new1,Report3,sizeof(Report3),0);
			cout<<Report3 << newreq.uname<<endl;
		}
		
		//printf("Closing connection\n");
		bzero(buffer,MAX_BUFFER);
		close(new1); /* close connected socket*/
	    return 0;

		}
		
	} /* CLOSE WHILE LOOP1*/
     close(new1);   
    //Free the socket pointer
    free(socket_cl);
     
    return 0;
}

int test_connection()
{

	MYSQL *connect;
    connect = mysql_init(NULL);

    if (!connect)
    {
        cout << "Mysql Initialization Failed";
        return 0;
    }

    connect = mysql_real_connect(connect, SERVER, USER, PASSWORD, DATABASE, 0,NULL,0);
    if (connect)
    {
       cout << "Test Connection to database Succeeded\n";
    }
    else
    {
        cout << "Connection Failed...Unable to connect to Mysql DATABASE\n";
        return 0; 
    }

    mysql_close (connect);
    return 1;


}

int add_newuser(string username,string password)
{
	MYSQL *connect;
    connect = mysql_init(NULL);

    if (!connect)
    {
        cout << "Mysql Initialization Failed";
        return 0;
    }

    connect = mysql_real_connect(connect, SERVER, USER, PASSWORD, DATABASE, 0,NULL,0);

    if (connect)
    {
       // cout << "Connection Succeeded\n";
    }
    else
    {
        cout << "Connection Failed...Unable to connect to Mysql DATABASE\n";
        return 0; 
    }

    MYSQL_RES *res_set;
    MYSQL_ROW row;
    //------------------------------------------------------------------------------------------------------
    /*Signing up at the server*/

    
    string query="INSERT INTO user(user_name,passwd) values('"+username+"','"+password+"');";

    if(mysql_query(connect,query.c_str()))
    {
    	mysql_close (connect);
    	return 0;
    }
    else
	{
		mysql_close (connect);
		return 1;
	}

    
}

int authenticate_user(string username,string password)
{
	MYSQL *connect;
    connect = mysql_init(NULL);
    
    if (!connect)
    {
        cout << "Mysql Initialization Failed";
        return 0;
    }

    connect = mysql_real_connect(connect, SERVER, USER, PASSWORD, DATABASE, 0,NULL,0);

    if (connect)
    {
       // cout << "Connection Succeeded\n";

    }
    else
    {
        cout << "Connection Failed...Unable to connect to Mysql DATABASE\n";
        return 0; 
    }

    MYSQL_RES *res_set;
    MYSQL_ROW row;
    //------------------------------------------------------------------------------------------------------
    /*Login in  at the server*/

        
    string query1="select * from user where user_name='"+username+"' and passwd='"+password+"'";

    if(mysql_query(connect,query1.c_str()))
    	{
        cout<<"query not executed"<<endl;
    	return 0;
		}

    res_set=mysql_store_result(connect);
    unsigned int numrows = mysql_num_rows(res_set);

    mysql_free_result(res_set);
    //cout<<numrows<<endl;
    if(numrows==1)
       {
       	mysql_close (connect);
       	return 1;
       }
    else
      {
      	mysql_close (connect);
      	return 0;
      }

    //----------------------------------------------------------------------------------------------------------
    
}


