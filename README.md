# file-server-optimised
file-server-optimised

There are five folders inside : loadgen, ph2, multithreaded ,prefetching and redis.The ph2, multithreaded, prefetching and redis folder contains server1 and server2 subfolders for respective stages of optimsations.The folder/directory structure and files are listed below. #The loadgen folder contains:

� Makefile

� client.cpp : Load generator (with Number of request as input)

� client timer.cpp : Load generator (with run time as input)

� "downloads" folder : to store received files.

To run client/ load generator:

./client Server1 IP Server1 Listening-port request type

#Each server1 sub-folder contains:

� Makefile

� server1.cpp : File server code.

� "uploads" folder : to store files available for sharing/download.

� filecreate.sh : To create files with random content of specified size.

� clearbuffer.sh : To clear contents of cache buffer.

� tunesocket.sh : To tune the socket parameters.

#Server2 sub-folder of ph2, multithreaded and prefetching folder contains:

� Makefile

� server2.cpp : Authentication server code.

� installmysql.sh : script to install MySQL server and required libraries for header files.

� importdatabase.sh : script to import database and user table from cs744.sql file.

� cs744.sql : contains MySQL database and table for import.

Each server2 sub-folder of redis folder contains:

� Makefile

� server2.cpp : Authentication server code.

� installredis.sh : script to install Redis server and required libraries for header files.
