/*
g++ -o client bclient_basic.cpp
./client Transactions.txt 12340
*/
#include <iostream>
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

using namespace std;

#define MAXRECORDS 10000
#define SERVER_ADDR "localhost"

struct tsn_record
{
	int id;
	int time_st;
	char ttype;
	int amt;
}tsn_record;

struct tsn_record * getRecords(char file[], int *totalTransactions)
{
	FILE *client;
	char type;
	int amt,id,ts,i;

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
		fscanf(client,"%d %d %c %d\n",&ts,&id,&type,&amt);
		rec[i].id = id;
		rec[i].time_st = ts;
		rec[i].ttype = type;
		rec[i].amt = amt;
		//printf("%d %d %c %d\n",ts,id,type,amt);
		i++;
	}
	//Dummy Record
	rec[i].id = -1;
	rec[i].time_st = -1;
	rec[i].ttype = 'w';
	rec[i].amt = 0;
	*totalTransactions = i;
	//printf("%s :Total Transaction :%d,i = %d ",__FUNCTION__,*totalTransactions,i);
	fclose(client);
	return rec;
}

void printRecords(struct tsn_record *rec)
{
	int i;
	for ( i=0;rec[i].id != -1;i++ )
	{
	      cout<<rec[i].time_st <<" "<<rec[i].id<<" "<<rec[i].ttype<<" "<<rec[i].amt<< endl;
	}
}


void printRecvMsg(char *msg, int len)
{
	int i;
	if(len == 0)
		return;
	for(i=0;i<len;i++)
		printf("%c",msg[i]);
	//printf("\n");
}

int stringcmp(const char *f,const char *s)
{
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

int main(int argc,char *argv[])
{
	
	
	struct tsn_record* tsn;
	char filename[80];

	int cli_sock;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	void *msg;
	int port, totalTsn;
	int i,byte_read,byte_write = -1;

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
	server = gethostbyname(SERVER_ADDR);
	port = atoi(argv[2]);
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
	printRecvMsg((char *)msg,byte_read);

	//Send Transactions
	for(i = 0;i <= totalTsn; i++)
	{
		//printf("%d %d %c %d\n",tsn[i].time_st,tsn[i].id,tsn[i].ttype,tsn[i].amt);
		byte_write = write(cli_sock, (void *)&tsn[i], sizeof(struct tsn_record));
	}

	do
	{
		byte_read = read(cli_sock,msg,1024);
		if(byte_read > 0)
		{
			fflush(stdout);
			printRecvMsg((char *)msg,byte_read);
			if((stringcmp((char *)msg,"bye")))
			{
				close(cli_sock);
				break;
			}
		}
	}while(1);
	free(msg);
	return 0;
}
