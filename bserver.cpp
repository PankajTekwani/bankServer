/*
g++ -o server bserver.cpp -lpthread
./server Records.txt 12340
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include "common.h"

#define MAXRECORDS 1000000
#define FILENAME "Records.txt"
#define MAXTRANSACTIONS 1000000
#define MAX_CLIENTS 200
#define INTEREST 2


static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//Server Account Records
struct acc_record
{
	int id;
	char name[50];
	float bal;
}acc_record;


//Thread Data
struct thrd_data
{
	int csock;
	struct acc_record* acc;		//Account Records at Server
}thrd_data;


// Function to read Records.txt File
struct acc_record* readServerFile(char file[])
{
	FILE *record;
	char name[50];
	float bal;
	int id,i;

	struct acc_record *rec = (struct acc_record *)malloc(MAXRECORDS * sizeof(struct acc_record));

	record = fopen(file,"r");
	if(record == NULL)
	{
		return NULL;
	}
	
	i=0;
	while ( !feof(record) )
	{
		fscanf(record,"%d %s %f\n",&id,name,&bal);
		rec[i].id = id;
		strcpy(rec[i].name, name);
		rec[i].bal = bal;
		i++;
	}
	//Dummy Record
	rec[i].id = -1;
	rec[i].name[0] = '\0';
	rec[i].bal = 0;
	
	fclose(record);
	return rec;
}



void printRecords(struct acc_record *rec)
{
	int i;
	for(i=0;rec[i].id != -1;i++)
	{
	      printf("\n%d %s %6.2f",rec[i].id,rec[i].name,rec[i].bal);
	}
	printf("\n");
}



int getRecord(struct acc_record* acc, int id)
{
	int i;
	for(i=0;acc[i].id != -1;i++)
	{
		if(acc[i].id == id)
			return i;
	}
	return -1;
}


void perform_tsn(struct acc_record* acc, struct tsn_record tsn, char *msg)
{
	
	int index = getRecord(acc, tsn.id);
	msg[0] = '\0';
	if(index == -1)
	{
		sprintf(msg,"\nA/c No:%d, doesn't Exist!!",tsn.id);
		//printf("\nAccount doesn't Exist!!\n");
		return;
	}

	pthread_mutex_lock(&lock);
	if(tsn.ttype == 'w')
	{
		if(tsn.amt < acc[index].bal)
		{
			acc[index].bal -= tsn.amt;
			sprintf(msg,"\nA/c No:%d, Withdrawn Amt:%d, New Balance:%6.2f",tsn.id,tsn.amt,acc[index].bal);
			//printf("\nAccount No: %d, Withdrawn Amt: %d, New Balance:%.62f\n",tsn.id,tsn.amt,acc[index].bal);
		}
		else
		{
			sprintf(msg,"\nA/c No:%d, No Suffient Balance! Requested:%d,Current Balance:%6.2f",tsn.id,tsn.amt,acc[index].bal);
			//printf("\nNo Suffient Balance to Withdraw!!\n");
		}
	}
	if(tsn.ttype == 'd')
	{	//No Limit On Deposits
		acc[index].bal += tsn.amt;
		sprintf(msg,"\nA/c No:%d, Deposit Amt:%d, New Balance:%6.2f",tsn.id,tsn.amt,acc[index].bal);
		//printf("\nAccount No: %d, Deposit Amt: %d, New Balance:%6.2f\n",tsn.id,tsn.amt,acc[index].bal);
	}
	pthread_mutex_unlock(&lock);
}



void * perform_cli_tsn(void *arg)
{
	struct tsn_record tsn;
	struct thrd_data *td = (struct thrd_data *)arg;
	int i = 0;
	int byte_written,byte_read,slept = 0;
	char *msg = (char *)malloc(1024 * sizeof(char));
	/* Thread Operation*/	

	while(1)
	{
		byte_read = read(td->csock, &tsn, sizeof(struct tsn_record));
		if(tsn.id == -1)
		{
			break;
		}
		perform_tsn(td->acc,tsn,msg);
		printf("%s",msg);
		byte_written = write(td->csock, (void *)msg, strlen(msg));
	}

	printf("\n*****Bye*****\n");
	strcpy(msg,"bye");
	byte_written = write(td->csock, (void *)msg, strlen(msg));

	//All Transactions from a Client are performed, so closing the connection.
	close(td->csock);
	//printRecords(td->acc);
}


void * add_interest(void *arg)
{
	struct acc_record* acc = (struct acc_record*)arg;
	int interest = INTEREST;
	int i;
	while(1)
	{
		sleep(50);
		//Acquire Mutex Lock
		pthread_mutex_lock(&lock);
		for(i=0;acc[i].id != -1;i++)
		{
			acc[i].bal =  acc[i].bal + (float)(interest * acc[i].bal)/100;
		}
		printf("\nAdded %d%% Interest to each acount",interest);
		printRecords(acc);
		pthread_mutex_unlock(&lock);
	}
}

int main(int argc,char *argv[])
{
	struct acc_record* acc;
	struct thrd_data td[MAX_CLIENTS];
	char filename[80];
	char msg[50];
	int lsock,csock[MAX_CLIENTS];
	socklen_t clilen;
	struct sockaddr_in server_addr,cli_addr;
	int port,i,no_of_clients,byte_written;

	pthread_t thrd[MAX_CLIENTS];
	pthread_t interest_thrd;
	setbuf(stdout,NULL);
	strcpy(filename,argv[1]);
	acc = readServerFile(filename);

	if(acc == NULL)
	{
		printf("\nUnable to open Server file");
		return -1;
	}

	//printRecords(acc);

	//Create Interest Thread
	pthread_create(&interest_thrd,NULL,add_interest,(void*)acc);

	if((lsock = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		printf("\nUnable to get server socket");
		return -1;
	}

	port = atoi(argv[2]);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if(bind(lsock,(struct sockaddr *)&server_addr,sizeof(server_addr)))
	{
		printf("\nUnable to Bind");
	}

	//Listen
		listen(lsock,10);
	clilen = sizeof(cli_addr);
	no_of_clients = 0;
	do
	{
	
	//Accept Connections from Client
		csock[no_of_clients] = accept(lsock,(struct sockaddr *)&cli_addr,&clilen);
		if (csock[no_of_clients] < 0) 
        	{
			printf("\nClient accept request denied!!");
			continue;
		}
		strcpy(msg,"Connection Accepted");
		byte_written = write(csock[no_of_clients], (void *)msg, strlen(msg) + 1);
		//Prepare thread data and create New Thread
		td[no_of_clients].acc = acc;
		td[no_of_clients].csock = csock[no_of_clients];
		pthread_create(&thrd[no_of_clients],NULL,perform_cli_tsn,(void*)&td[no_of_clients]);
		no_of_clients++;
	}while(1);

	close(lsock);

	return 0;
}
