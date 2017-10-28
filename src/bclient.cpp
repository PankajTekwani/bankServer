/*
g++ -o client bclient_basic.cpp
./client Transactions.txt localhost 12340
*/

#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
<<<<<<< HEAD:src/bclient.cpp
#include <time.h>
=======
>>>>>>> bd8528b1b346c2d9e9212319f74dc45bd692648b:bclient_basic.cpp
#include "common.h"


#define MAXRECORDS 1000000
#define SERVER_ADDR "localhost"

<<<<<<< HEAD:src/bclient.cpp
/*
This function reads the transaction file and prepares the array of structures to pass it to the server in order to perform transactions
*/
=======
>>>>>>> bd8528b1b346c2d9e9212319f74dc45bd692648b:bclient_basic.cpp
struct tsn_record * getRecords(char file[], int *totalTransactions)
{
	FILE *client;
	char type;
	int id,ts,i;
	long amt;
	struct tsn_record *rec = (struct tsn_record *)malloc(MAXRECORDS * sizeof(struct tsn_record));

	client = fopen(file,"r");
	if(client == NULL)
	{
		printf("unable to open file\n");
		return 0;
	}
	
	i=0;
	while ( !feof(client) )
	{
		fscanf(client,"%d %d %c %ld\n",&ts,&id,&type,&amt);
		rec[i].id = id;
		rec[i].time_st = ts;
		rec[i].ttype = type;
		rec[i].amt = amt;
		//printf("%d %d %c %d\n",ts,id,type,amt);
		i++;
	}
	//Dummy Record
	rec[i].id = -1;
	rec[i].time_st = 0;
	rec[i].ttype = 'w';
	rec[i].amt = 0;
	*totalTransactions = i;
	//printf("%s :Total Transaction :%d,i = %d ",__FUNCTION__,*totalTransactions,i);
	fclose(client);
	return rec;
}

/*
This function prints the records which the client read from the transaction file
*/
void printRecords(struct tsn_record *rec)
{
	int i;
	for ( i=0;rec[i].id != -1;i++ )
	{
<<<<<<< HEAD:src/bclient.cpp
		printf("\n%d %d %c %ld",rec[i].time_st,rec[i].id,rec[i].ttype,rec[i].amt);
=======
		printf("\n%d %d %c %d",rec[i].time_st,rec[i].id,rec[i].ttype,rec[i].amt);
>>>>>>> bd8528b1b346c2d9e9212319f74dc45bd692648b:bclient_basic.cpp
	}
}

/*
It prints the message recieved from the server on the client terminal
*/
void printRecvMsg(char *msg, int len, double time)
{
	int i;
	if(len == 0)
		return;
	printf("\n[%lf]secs\t",time);
	for(i=0;i<len;i++)
		printf("%c",msg[i]);
	//printf("\n");
}


int stringcmp(const char *f,const char *s)
{//Unable to work
	int i,len;
	len = strlen(f);
	if(len !=strlen(s))
		return 0;
	for(i = 0;i<len;i++)
	{
		if(f[i]!=s[i])
			return 0;
	}
	return 1;
}
/*
Main function to start a client
*/
int main(int argc,char *argv[])
{
	
	
	struct tsn_record* tsn;
	char filename[80];
	int slept = 0;
	int cli_sock;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	void *msg;
	int port, totalTsn;
	int i,byte_read,byte_write = -1;
<<<<<<< HEAD:src/bclient.cpp
	clock_t begin, end;
	double time_spent,ttime;

=======
	
>>>>>>> bd8528b1b346c2d9e9212319f74dc45bd692648b:bclient_basic.cpp
	setbuf(stdout,NULL);
	strcpy(filename,argv[1]);
	
	tsn = getRecords(filename,&totalTsn);
	if(tsn == NULL)
	{
		printf("\nunable to open client file");
		return -1;
	}

	//printRecords(tsn);	

	//Make Clients
	cli_sock = socket(AF_INET,SOCK_STREAM,0);
	if(cli_sock == -1)
	{
		printf("\nUnable to create Socket Descriptor");
		return -1;
	}

	//Prepare Server Structure to Connect
	server = gethostbyname(argv[2]);
	port = atoi(argv[3]);
	memset(&serv_addr,'0',sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	//inet_aton((char *)server->h_addr, /*(struct in_addr *)*/(char *)&serv_addr.sin_addr.s_addr);
	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);

	//Connecct to Server
	if(connect(cli_sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
	{
		printf("\nError in Client Connection");
		return -1;
	}

	msg = malloc(1024 * sizeof(char));
	byte_read = read(cli_sock,msg,21);
	printf("\n%s",(char *)msg);

<<<<<<< HEAD:src/bclient.cpp
	ttime = 0;
	//Send Transactions to SERVER
	for(i = 0;i <= totalTsn; i++)
	{
		//printf("%d %d %c %ld\n",tsn[i].time_st,tsn[i].id,tsn[i].ttype,tsn[i].amt);
		//SLEEP till the specified Time Stamp
		if((tsn[i].time_st - slept) > 0)
			{
				usleep((tsn[i].time_st - slept)*1000);
				slept = tsn[i].time_st;
			}
		begin = clock();
		byte_write = write(cli_sock, (void *)&tsn[i], sizeof(struct tsn_record));
		byte_read = read(cli_sock,msg,1024);
		end = clock();
		if(byte_read > 0)
		{
			time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
			printRecvMsg((char *)msg,byte_read,time_spent);
			ttime += time_spent;
		}
	}
	printf("\n\nTotal Time for All Transactions: [%lf] secs\n",(ttime));
=======
	//printf("Total Transn: %d",totalTsn);
	//Send Transactions
	for(i = 0;i <= totalTsn; i++)
	{
		//printf("%d %d %c %d\n",tsn[i].time_st,tsn[i].id,tsn[i].ttype,tsn[i].amt);
		if((tsn[i].time_st - slept) > 0)
			{
				sleep(tsn[i].time_st - slept);
				slept = tsn[i].time_st;
			}
		byte_write = write(cli_sock, (void *)&tsn[i], sizeof(struct tsn_record));
		byte_read = read(cli_sock,msg,1024);

		if(byte_read > 0)
		{
			if((stringcmp((char *)msg,"bye")))
			{
				printf("\n****Bye****\n");
				close(cli_sock);
				break;
			}
			printRecvMsg((char *)msg,byte_read);
		}
	}

>>>>>>> bd8528b1b346c2d9e9212319f74dc45bd692648b:bclient_basic.cpp
	free(msg);
	close(cli_sock);
	return 0;
}
