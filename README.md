# web-server

This is the work of the spring semester System Programming course in 2020 

# Introduction

In this project there are implemented a distributed and optimized database with a multi-threaded web server and client that respond to queries about coronavirus cases all written in C language.

Specifically, three have been implemented programs:
1)    a master program that creates a series of Worker processes.
2)    a multi-threaded whoServer that collects summary statistics from Worker over a network.
processes and queries from client 
3)    a multicreaded client whoClient program that creates a lot
threads, where each thread plays the role of a client sending queries to whoServer.

## The master program
The master program is used as follows:
`./master –w numWorkers -b bufferSize –s serverIP –p serverPort -i input_dir`
where:
Num The numWorkers parameter is the Worker number of processes that the application will generate.
Buff The bufferSize parameter: is the size of the buffer for reading over pipes.
Server The serverIP parameter: is the IP address of the whoServer to which the Worker processes will connect to
send him summary statistics.
Server The serverPort parameter: is the port number where the whoServer listens.
Input The input_dir parameter: is a directory that contains subdirectories with the files to be processed
the Workers.
Each subdirectory will have the name of a country and will contain files with
names that are DD-MM-YYYY form dates. Each DD-MM-YYYY file will contain a series of patient records where each line will describe a patient who was admitted / discharged to / from a hospital that day and contains the recordID, his name,
the virus, and its age.


Starting, the master program starts numWorkers Workers child processes and distributes
uniformly the subdirectories with the countries in input_dir in Workers. Starts the Workers and informs each Worker via named pipe about the subdirectories that will
undertaken by the Worker. The parent process sends via named pipe both the IP address and the port number of whoServer.
When the creation of the Worker processes is finished, the parent process will remain (ie will not end) to
forks a new Worker process in case an existing Worker suddenly terminates.
Each Worker process, for each directory assigned to it, reads all its files in chronological order
of filenames and fills in a number of data structures that it will use to respond to
questions posed by whoServer. Logs in to whoServer and sends him the following information:
1)    a port number where the Worker process listens for questions that will be forwarded by whoServer 
2)    the summaries statistics (same as in the second work). When the Worker process finishes transferring information to whoServer,
listens to the port number it has selected and wait for connections from whoServer for requests regarding
countries it manages.


## The whoServer program 
WhoServer will be used as follows:
`./whoServer –q queryPortNum -s statisticsPortNum –w numThreads –b bufferSize`
where:
Qu The queryPortNum parameter is a port number where whoServer will listen for query links
from the whoClient client
Statistics The statisticsPortNum parameter is a port number where whoServer will listen for connections to
summary statistics by Worker processes
Num The numThreads parameter: is the number of threads that whoServer will generate to serve
incoming connections from the network. Threads should be created once at the beginning when the
whoServer.
Buff The bufferSize parameter is the size of a ** circular ** buffer to be shared between the threads
are created by the whoServer process. The bufferSize represents the number of file / socket descriptors that
can be stored in it (eg 10, means 10 descriptors).

In the implementation, when whoServer starts, the original thread creates numThreads threads. The
main (main process) thread listens on the ports queryPortNum and statisticsPortNum, accepts connections
with the accept() system call and place the file / socket descriptors that
correspond to the connections in a circular buffer of size defined by bufferSize. The original thread is NOT
read from the links it receives. Simply, whenever it accepts a connection it places the file descriptor
which accept() returns to the buffer and continues to accept subsequent connections. The work of numThreads
are to serve the links whose corresponding file descriptors are placed in
buffer. Each of the numThreads threads wakes up when there is at least one descriptor in the buffer.
More specifically, the original thread listens to statisticsPortNum for links from Worker processes to
receive the summary statistics and the port number where each Worker process listens, and listens to
queryPortNum for connections from whoClient to receive queries about cases reported in
distributed process system.


## The queries
WhoServer will accept and serve the following requests coming from whoClient.

*    `/diseaseFrequency virusName date1 date2 [country]`
If no country argument is given, whoServer will find the number of cases for virusName
recorded in the system within [date1 ... date2]. If a country argument is given, ο
whoServer will find out about the virusName disease, the number of cases in the country that have
recorded in space [date1 ... date2]. The date1 date2 arguments will take the form
DD-MM-YYYY. The communication protocol between the client and whoServer should
manages in some way the fact that [country] is optional in this query.


*   `/topk-AgeRanges k country disease date1 date`
WhoServer will find, for the country and the virus disease the top k age categories that have
cases of the virus in that country and their incidence. The
date1 date2 arguments will be in DD-MM-YYYY format.

*   `/searchPatientRecord recordID`
WhoServer forwards the request to all Workers and waits for a response from the Worker with the record
recordID.

*   `/numPatientAdmissions disease date1 date2 [country]`
If a country argument is given, whoServer should forward the request to the workers in order to find the
total number of patients admitted to hospital with the disease in that country within
in space [date1 date2]. If no country argument is given, he will find the number of patients with the disease
disease that entered the hospital in space [date1, date2]. The date1 date2 arguments will
have DD-MM-YYYY format.

*    `/numPatientDischarges disease date1 date2 [country]`
If given the country argument, whoServer will find the total number of patients with the disease
who have been discharged from a hospital in that country within [date1, date2]. If not given
country argument, whoServer will find the number of patients with the disease who have been discharged from
hospital in space [date1, date2]. The date1, date2 arguments will be in DD-MM-YYYY format. 


## The whoClient program
The whoClient program is used as follows:
`./whoClient –q queryFile -numThreads –sp servPort –sip servIP`
The queryFile parameter is the file that contains the queries to be sent to whoServer.
Num The numThreads parameter: is the number of threads that whoClient will generate for sending
queries in whoServer
Serv The servPort parameter is the port number where the whoServer listens to which the whoClient will connect.
Serv The servIP parameter is the IP address of the whoServer to which the whoClient will connect.

The function of multithreaded whoClient is as follows. The queryFile file starts and opens, which it reads line by line. On each line there is one command that the whoServer can accept. For each command creates a thread that undertakes to send a command (ie a line) to whoServer. Thread
is created but does not connect directly to whoServer. When all the threads are created, that is, we have one
thread for each command in the file, then the threads should all start together to try to connect
to whoServer and send their command. When the command is sent, each thread prints the response received
from whoServer to stdout and can quit. When all threads are terminated, the whoClient terminates as well. 


