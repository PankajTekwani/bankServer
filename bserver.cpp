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
#include <vector>

#define MAXRECORDS 100
#define FILENAME "Records.txt"
#define MAXTRANSACTIONS 10000
#define MAX_CLIENTS 10

using namespace std;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//Server Account Records
struct acc_record
{
	int id;
	char name[50];
	int bal;
}acc_record;



//Transaction Record
struct tsn_record
{
	int id;
	int time_st;
	char ttype;
	int amt;
}tsn_record;


//Thread Data
struct thrd_data
{
	int csock;
	struct acc_record* acc;		//Account Records at Server
	struct tsn_record* tsn_table;	//Client Transaction
}thrd_data;


// Function to read Records.txt File
struct acc_record* readServerFile(char file[])
{
	FILE *record;
	char name[50];
	int bal,id,i;

	struct acc_record *rec = (struct acc_record *)malloc(MAXRECORDS * sizeof(struct acc_record));

	record = fopen(file,"r");
	if(record == NULL)
	{
		printf("\nunable to open Server file");
		return 0;
	}
	
	i=0;
	while ( !feof(record) )
	{
		fscanf(record,"%d %s %d\n",&id,name,&bal);
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
	      printf("%d %s %d\n",rec[i].id,rec[i].name,rec[i].bal);
	}
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
	//pthread_mutex_lock(&lock);
	int index = getRecord(acc, tsn.id);
	msg[0] = '\0';
	if(index == -1)
	{
		sprintf(msg,"\nAccount No:%d doesn't Exist!!",tsn.id);
		//printf("Account doesn't Exist!!\n");
		return;
	}
	if(tsn.ttype == 'w')
	{
		if(tsn.amt < acc[index].bal)
		{
			acc[index].bal -= tsn.amt;
			sprintf(msg,"\nAccount No:%d, Withdrawn Amt:%d, New Balance:%d",tsn.id,tsn.amt,acc[index].bal);
			//printf("Account No: %d, Withdrawn Amt: %d, New Balance:%d\n",tsn.id,tsn.amt,acc[index].bal);
		}
		else
		{
			sprintf(msg,"\nAccount No:%d, No Suffient Balance! Requested:%d,Current Balance:%d",tsn.id,tsn.amt,acc[index].bal);
			//printf("No Suffient Balance to Withdraw!!\n");
		}
	}
	if(tsn.ttype == 'd')
	{	//No Limit On Deposits
		acc[index].bal += tsn.amt;
		sprintf(msg,"\nAccount No:%d, Deposit Amt:%d, New Balance:%d",tsn.id,tsn.amt,acc[index].bal);
		//printf("Account No: %d, Deposit Amt: %d, New Balance:%d\n",tsn.id,tsn.amt,acc[index].bal);
	}
	//pthread_mutex_unlock(&lock);
}



void * perform_cli_tsn(void *arg)
{
	struct thrd_data *td = (struct thrd_data *)arg;
	int i = 0;
	int byte_written,slept = 0;
	char *msg = (char *)malloc(1024 * sizeof(char));
	/* Thread Operation*/	

	while(td->tsn_table[i].id != -1)
	{
		msg[0] = '\0';
		//printf("\nTsn:%d Sleeping for %d",td->tsn_table[i].id,td->tsn_table[i].time_st - slept);
		sleep(td->tsn_table[i].time_st - slept);
		slept = td->tsn_table[i].time_st;
		//printf("\n%d %d %c %d",td->tsn_table[i].time_st,td->tsn_table[i].id,td->tsn_table[i].ttype,td->tsn_table[i].amt);
		perform_tsn(td->acc,td->tsn_table[i],msg);
		printf("%s",msg);
		byte_written = write(td->csock, (void *)msg, strlen(msg));
		//printf("byte_wrriten : %d sting len:%lu\n", byte_written,strlen(msg));
		i++;
	}
	byte_written = write(td->csock, (void *)"bye", 4);
	close(td->csock);
	free(td->tsn_table);
	//printRecords(td->acc);
}



struct tsn_record * read_from_client(int csock)
{
	struct tsn_record tsn;
	struct tsn_record *tsn_table = (struct tsn_record *)malloc(MAXTRANSACTIONS * sizeof(struct tsn_record));;
	int byte_read,no_of_tsn = 0;
     	do 
	{
		byte_read = read(csock, &tsn, sizeof(struct tsn_record));
		if(tsn.id == -1)
		{
			//Last Dummy Record
			tsn_table[no_of_tsn].id = -1;
			break;
		}
		tsn_table[no_of_tsn].id = tsn.id;
		tsn_table[no_of_tsn].ttype = tsn.ttype;
		tsn_table[no_of_tsn].amt = tsn.amt;
		tsn_table[no_of_tsn].time_st = tsn.time_st;
		//printf("\n%d %d %c %d",tsn.time_st,tsn.id,tsn.ttype,tsn.amt);
		no_of_tsn++;
	}while(1);
	return tsn_table;	
}



int main(int argc,char *argv[])
{
	struct acc_record* acc;
	struct thrd_data td[MAX_CLIENTS];
	struct tsn_record *tsn_table;
	char filename[80];

	int lsock,csock[MAX_CLIENTS];
	socklen_t clilen;
	struct sockaddr_in server_addr,cli_addr;
	int port,i,no_of_clients,byte_written;

	pthread_t thrd[MAX_CLIENTS];

	strcpy(filename,argv[1]);
	acc = readServerFile(filename);
	//printRecords(acc);
	if((lsock = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		printf("\nUnable to get server socket");
		return 0;
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
		byte_written = write(csock[no_of_clients], (void *)"\nConnection Accepted", 21);
		tsn_table = read_from_client(csock[no_of_clients]);
		td[no_of_clients].acc = acc;		/*Prepare thread data*/
		td[no_of_clients].csock = csock[no_of_clients];
		td[no_of_clients].tsn_table = tsn_table;
		pthread_create(&thrd[no_of_clients],NULL,perform_cli_tsn,(void*)&td[no_of_clients]);
		no_of_clients++;
	}while(1);

	for(i = 0;i<no_of_clients;i++)
	{
		pthread_join(thrd[i],NULL);
	}

	close(lsock);

	return 0;
}
