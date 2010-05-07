/* server.c */
/* Speek Messenger Server */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

/* MySQL */
#include <mysql.h>
MYSQL *dbconn;
MYSQL_RES *dbres;
MYSQL_ROW row;

/* MySQL configuration header */
#include "database.h"

/* Speek Messenger Protocol Header File */
#include "protocol.h"

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
char* itoa(int value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}

char *strdup (const char *s) {
    char *d = (char *)(malloc (strlen (s) + 1));
    if (d != NULL)
        strcpy (d,s);
    return d;
}

int measurePacket(protocol *msg) {
	int size;
	
	size = 20 + strlen((*msg).data);
	(*msg).pkt_len = ntohl(size);
	
	return size;
}

void printPacket(protocol msg) {
	printf("[DEBUG] -- Packet info START--\n");
	printf("[DEBUG] packet name: %s\n", msg.name);
	printf("[DEBUG] packet version: %d\n", ntohl(msg.version));
	printf("[DEBUG] packet pkt_len: %d\n", ntohl(msg.pkt_len));
	printf("[DEBUG] packet service: %d\n", ntohl(msg.service));
	printf("[DEBUG] packet status: %d\n", ntohl(msg.status));
	printf("[DEBUG] packet data: %s\n", msg.data);
	printf("[DEBUG] -- Packet info END --\n");
}

MYSQL_RES * db_query(MYSQL *conn, char * sql) {
	if(mysql_query(conn, sql)) {
		printf("[DATABASE] Query error: %s\n", mysql_error(conn));
		return NULL;
	}
	printf("[DATABASE] Query OK: %s\n", sql);
	
	return mysql_store_result(conn);
}

char * getUserid(char * username) {
	MYSQL_RES *tres;
	MYSQL_ROW trow;
	char tquery[256];
	
	strcpy(tquery, "SELECT `id` FROM `user` WHERE `username` = '");
	strcat(tquery, username);
	strcat(tquery, "'");
	tres = db_query(dbconn, tquery);
	
	trow = mysql_fetch_row(tres);
	
	return trow[0];
}

char * getUsername(char * userid) {
	MYSQL_RES *tres;
	MYSQL_ROW trow;
	char tquery[256];
	
	strcpy(tquery, "SELECT `username` FROM `user` WHERE `id` = '");
	strcat(tquery, userid);
	strcat(tquery, "'");
	tres = db_query(dbconn, tquery);
	
	trow = mysql_fetch_row(tres);
	
	return trow[0];
}

char * getUsernameFromSD(int sd) {
	MYSQL_RES *tres;
	MYSQL_ROW trow;
	char tquery[256];
	char tstr[256];
	
	strcpy(tquery, "SELECT `username` FROM `user` WHERE `id` = (SELECT `userid` FROM `presence` WHERE `sd` = '");
	strcat(tquery, itoa(sd, tstr, 10));
	strcat(tquery, "')");
	tres = db_query(dbconn, tquery);
	
	trow = mysql_fetch_row(tres);
	
	return trow[0];
}

int processMessage(int sd, protocol imsg) {
	protocol omsg;
	protocol tmsg;
	
	int valid = 1;
	int send_ok = 1;
	int size;
	
	int tsd;
	
	char query[512];
	char * tstr1;
	char * tstr2;
	char tstr3[128];
	
	int n;
	
	/* Service variables */
	// GENERIC
	char * username;
	char * password;
	
	// LOGON
	char * logon_username;
	char * logon_password;
	int logon_userid;
	int	logon_status = 0;
	
	// LOGOFF
	// -- N/A
	
	// MESSAGE
	char * msg_sender;
	char * msg_receiver;
	char * msg_content;
	
	// ADD/REM FRIEND
	char * buddy;
	
	if(strcmp(imsg.name, PROTOCOL_NAME) != 0) {
		return 0;
	}
	
	imsg.version = ntohl(imsg.version);
	imsg.service = ntohl(imsg.service);
	imsg.pkt_len = ntohl(imsg.pkt_len);
	imsg.status = ntohl(imsg.status);
	
	memset(&omsg, '\0', sizeof(protocol));
	
	switch(imsg.service) {
		case SMSG_SERVICE_LOGON:
			tstr1 = strtok(imsg.data, "\30");
			logon_username = strtok(tstr1, "\31");
			username = strdup(logon_username);
			logon_password = strtok(NULL, "\31");
			
			printf("[CLIENT-%d] SERVICE_LOGON: username=%s, password=%s\n", sd, logon_username, logon_password);
			
			strcpy(query, "SELECT `id`, `username`, `password` FROM `user` WHERE `username` = '");
			strcat(query, logon_username);
			strcat(query, "' AND `password` = '");
			strcat(query, logon_password);
			strcat(query, "'");

			/* Check username & password */
			dbres = db_query(dbconn, query);
			if(mysql_num_rows(dbres) > 0) {
				/* Get user id number */
				row = mysql_fetch_row(dbres);
				logon_userid = atoi((char *)row[0]);
				
				/* Update presence table */
				strcpy(query, "UPDATE `presence` SET `userid` = '");
				strcat(query, itoa(logon_userid, tstr1, 10));
				strcat(query, "' WHERE `sd` = '");
				strcat(query, itoa(sd, tstr1, 10));
				strcat(query, "'");
				db_query(dbconn, query);
				
				logon_status = 1;
			}
			
			if(logon_status == 1) {
				/* Compose reply packet */
				omsg.status = htonl(SMSG_STATUS_SUCCESS);
				strcpy(omsg.data, "OK");
				
				printf("[CLIENT-%d] Logon success.\n", sd);
				
				/* Broadcast online status to online friends */
				strcpy(query, "SELECT DISTINCT `user`.`username`, `presence`.`sd` FROM `user`, `friend` LEFT JOIN `presence` ON (`friend`.`buddy2` = `presence`.`userid`) WHERE `friend`.`buddy2` = `user`.`id` AND `friend`.`buddy1` = '");
				strcat(query, itoa(logon_userid, tstr1, 10));
				strcat(query, "'");
				dbres = db_query(dbconn, query);
				
				while((row = mysql_fetch_row(dbres)) != NULL) {
					if(row[1] != NULL) {
						tsd = atoi(row[1]);
						printf("[SERVER] Sending status update to %s @ sd=%d\n", row[0], tsd);
						
						memset(&tmsg, '\0', sizeof(protocol));
						
						strcpy(tmsg.name, PROTOCOL_NAME);
						tmsg.version = htonl(SPEEK_VERSION);
						tmsg.service = htonl(SMSG_STATUSUPDATE);
						tmsg.status = htonl(SMSG_STATUS_ONLINE);
						strcpy(tmsg.data, "\30");
						strcat(tmsg.data, username);
						
						size = measurePacket(&tmsg);
						
						printPacket(tmsg);
						
						send(tsd, (void *)& tmsg, size, 0);
					}
				}
			} else {
				omsg.status = htonl(SMSG_STATUS_FAILED);
				strcpy(omsg.data, "ERR");
				
				printf("[CLIENT-%d] Logon failed.\n", sd);
			}
			break;
		
		case SMSG_SERVICE_LOGOFF:
			printf("[CLIENT-%d] Logged off.\n", sd);
			omsg.status = htonl(SMSG_STATUS_SUCCESS);
			strcpy(omsg.data, "OK");
			break;
		
		case SMSG_MESSAGE:
			tstr1 = strtok(imsg.data, "\30");
			msg_sender = strdup(strtok(tstr1, "\31"));
			msg_receiver = strdup(strtok(NULL, "\31"));
			msg_content = strdup(strtok(NULL, "\31"));
			
			/* Look for receiver sd number */
			strcpy(query, "SELECT `sd` FROM `presence` WHERE `userid` = (SELECT `id` AS `userid` FROM `user` WHERE `username` = '");
			strcat(query, msg_receiver);
			strcat(query, "')");
			dbres = db_query(dbconn, query);
			
			printf("[SERVER] Forwarding message from %s to %s\n", msg_sender, msg_receiver);
			if(mysql_num_rows(dbres) > 0) {
				row = mysql_fetch_row(dbres);
				sd = atoi(row[0]);
				
				/* Composing forward message */
				memcpy(&omsg, &imsg, sizeof(protocol));
				strcpy(omsg.data, "\30");
				strcat(omsg.data, msg_sender);
				strcat(omsg.data, "\31");
				strcat(omsg.data, msg_receiver);
				strcat(omsg.data, "\31");
				strcat(omsg.data, msg_content);
			} else {
				printf("[SERVER] User %s is offline. Message discarded.\n", msg_receiver);
				send_ok = 0;
			}
			
			break;
		
		case SMSG_FILETRANSFER:
			username = strtok(imsg.data, "\30");
			
			printf("[CLIENT-%d] File transfer request to %s\n", sd, username);
			
			strcpy(query, "SELECT `user`.`username`, `presence`.`ip` FROM `user`, `presence` WHERE `user`.`id` = `presence`.`userid` AND `presence`.`userid` = (SELECT `id` AS `userid` FROM `user` WHERE `username` = '");
			strcat(query, username);
			strcat(query, "')");
			dbres = db_query(dbconn, query);
			
			if(mysql_num_rows(dbres) > 0) {
				row = mysql_fetch_row(dbres);
				
				printf("[CLIENT-%d] User %s connected from %s\n", sd, username, row[0]);
				
				omsg.status = htonl(SMSG_STATUS_SUCCESS);
				strcpy(omsg.data, "\30");
				strcat(omsg.data, row[0]);
				strcat(omsg.data, "\31");
				strcat(omsg.data, row[1]);
			} else {
				printf("[CLIENT-%d] User %s is offline. Filetransfer not possible.\n", sd, username);
				
				omsg.status = htonl(SMSG_STATUS_FAILED);
			}
			
			break;
		
		case SMSG_ADDFRIEND:
			tstr1 = strtok(imsg.data, "\30");
			username = strtok(tstr1, "\31");
			buddy = strtok(NULL, "\31");
			
			printf("[CLIENT-%d] User %s adds %s as friend.\n", sd, username, buddy);
			
			strcpy(query, "SELECT * FROM `user` WHERE `username` = '");
			strcat(query, buddy);
			strcat(query, "'");
			dbres = db_query(dbconn, query);
			
			if(mysql_num_rows(dbres) > 0) {
				strcpy(query, "INSERT INTO `friend` (`buddy1`, `buddy2`) SELECT `id` AS `buddy1`, (SELECT `id` FROM `user` WHERE `username` = '");
				strcat(query, buddy);
				strcat(query, "') AS `buddy2` FROM `user` WHERE `username` = '");
				strcat(query, username);
				strcat(query, "'");
				dbres = db_query(dbconn, query);
				
				omsg.status = htonl(SMSG_STATUS_SUCCESS);
				strcpy(omsg.data, "OK");
			} else {
				printf("[CLIENT-%d] User %s does not exist.\n", sd, buddy);
				
				omsg.status = htonl(SMSG_STATUS_FAILED);
				strcpy(omsg.data, "ERR");
			}
			
			break;
		
		case SMSG_REMFRIEND:
			tstr1 = strtok(imsg.data, "\30");
			username = strtok(tstr1, "\31");
			buddy = strtok(NULL, "\31");
			
			printf("[CLIENT-%d] User %s removes %s from friendlist.\n", sd, username, buddy);
			
			strcpy(query, "DELETE FROM `friend` WHERE `buddy1` = (SELECT `id` FROM `user` WHERE `username` = '");
			strcat(query, username);
			strcat(query, "') AND `buddy2` = (SELECT `id` FROM `user` WHERE `username` = '");
			strcat(query, buddy);
			strcat(query, "')");
			dbres = db_query(dbconn, query);
			
			omsg.status = htonl(SMSG_STATUS_SUCCESS);
			strcpy(omsg.data, "OK");
			
			break;
		
		case SMSG_RETFRIEND:
			username = strtok(imsg.data, "\30");
			printf("[CLIENT-%d] Retrieving friend list for user %s\n", sd, username);
			
			strcpy(query, "SELECT DISTINCT `user`.`username`, `presence`.`sd` FROM `user`, `friend` LEFT JOIN `presence` ON (`friend`.`buddy2` = `presence`.`userid`) WHERE `friend`.`buddy2` = `user`.`id` AND `friend`.`buddy1` = (SELECT `id` AS `buddy1` FROM `user` WHERE `username` = '");
			strcat(query, username);
			strcat(query, "')");
			dbres = db_query(dbconn, query);
			
			if(mysql_num_rows(dbres) > 0) {
				/* Compose reply packet */
				while((row = mysql_fetch_row(dbres)) != NULL) {
					strcat(omsg.data, "\30");
					strcat(omsg.data, row[0]);
					strcat(omsg.data, "\31");
					
					if(row[1] == NULL) {
						/* User offline */
						strcat(omsg.data, itoa(SMSG_STATUS_OFFLINE, tstr3, 10));
					} else {
						/* User online */
						strcat(omsg.data, itoa(SMSG_STATUS_ONLINE, tstr3, 10));
					}
				}
			}
			
			break;
		
		case SMSG_REGISTER:
			tstr1 = strtok(imsg.data, "\30");
			username = strtok(tstr1, "\31");
			password = strtok(NULL, "\31");
			
			printf("[CLIENT-%d] Register username=%s, password=%s\n", sd, username, password);
			
			strcpy(query, "SELECT `id` FROM `user` WHERE `username` = '");
			strcat(query, username);
			strcat(query, "'");
			dbres = db_query(dbconn, query);
			
			if(mysql_num_rows(dbres) > 0) {
				omsg.status = htonl(SMSG_STATUS_FAILED);
				strcpy(omsg.data, "ERR");
				printf("[CLIENT-%d] User %s already exists.\n", sd, username);
			} else {
				strcpy(query, "INSERT INTO `user` (`username`, `password`) VALUES('");
				strcat(query, username);
				strcat(query, "', '");
				strcat(query, password);
				strcat(query, "')");
				dbres = db_query(dbconn, query);
			
				omsg.status = htonl(SMSG_STATUS_SUCCESS);
				strcpy(omsg.data, "OK");
			}
			
			break;
		
		default:
			valid = 0;
			break;
	}
	
	if(valid) {
		if(send_ok == 1) {
			strcpy(omsg.name, PROTOCOL_NAME);
			omsg.version = htonl(SPEEK_VERSION);
			omsg.service = htonl(imsg.service);
			
			size = measurePacket(&omsg);
			
			printf("[CLIENT-%d] Sending reply\n", sd);
			printPacket(omsg);
			send(sd, (void *)& omsg, size, 0);
		}
	} else {
		size = -1;
	}
	
	return size;
}

int main(int argc, char *argv[]) {
	int fdmax;			/* maximum file descriptor number */
	fd_set master;		/* master file descriptor list */
	fd_set read_fds;	/* temp file descriptor list for select() */
	
	struct sockaddr_in serveraddr;	/* server address */
	struct sockaddr_in clientaddr;	/* client address */
	
	int listener;	/* listening socket descriptor */	
	int newfd;		/* newly accept()ed socket descriptor */
	int tsd;		/* temporary socket descriptor */
	
	int nbytes;		/* buffer for client data */
	
	char * tstr1;
	char * tstr2;
	char query[512];
	
	protocol imsg;
	protocol omsg;
	
	/* for setsockopt() SO_REUSEADDR, below */
	int yes = 1;
	int addrlen;
	
	int i, j;
	int connid = 1;
	
	/* -- Variable declarations end */
	
	/* Print welcome message */
	printf("Speek Messenger Server\n");
	printf("----------------------\n");
	
	/* Database init */
	dbconn = mysql_init(NULL);
	if(!mysql_real_connect(dbconn, hostname, username, password, database, 0, NULL, 0)) {
		printf("Database initialization failed: %s\n", mysql_error(dbconn));
		exit(1);
	}
	printf("[SERVER] Initializing database...OK\n");
	db_query(dbconn, "TRUNCATE TABLE `presence`");
	
	/* clear the master and temp sets */
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	/* get the listener */
	if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Server-socket() error lol!");
		/*just exit lol!*/
		exit(1);
	}
	printf("[SERVER] Opening socket...OK\n");

	/*"address already in use" error message */
	if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("Server-setsockopt() error lol!");
		exit(1);
	}
	printf("[SERVER] Setting socket options...OK\n");

	/* bind */
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(PORT);
	memset(&(serveraddr.sin_zero), '\0', 8);

	if(bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
		perror("Server-bind() error lol!");
		exit(1);
	}
	printf("[SERVER] Binding address...OK\n");

	/* listen */
	if(listen(listener, 10) == -1) {
		perror("Server-listen() error lol!");
		exit(1);
	}
	printf("[SERVER] Listening on port %d...OK\n", PORT);
	printf("[SERVER] Server is ready to accept connections\n\n");

	/* add the listener to the master set */
	FD_SET(listener, &master);
	/* keep track of the biggest file descriptor */
	fdmax = listener; /* so far, it's this one*/

	/* loop */
	while(1) {
		/* copy it */
		read_fds = master;

		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("Server-select() error lol!");
			exit(1);
		}
		//printf("Server-select() is OK...\n");

		/*run through the existing connections looking for data to be read*/
		for(i = 0; i <= fdmax; i++) {
			if(FD_ISSET(i, &read_fds)) {
				/* we got one... */
				if(i == listener) {
					/* handle new connections */
					addrlen = sizeof(clientaddr);
					if((newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1) {
						perror("Server-accept() error lol!");
					} else {
						//printf("[SERVER] Server-accept() is OK...\n");

						FD_SET(newfd, &master); /* add to master set */
						if(newfd > fdmax) { /* keep track of the maximum */
							fdmax = newfd;
						}
						printf("[SERVER] New connection from %s on socket %d\n", inet_ntoa(clientaddr.sin_addr), newfd);
						
						/* Add to table presence */
						
						strcpy(query, "INSERT INTO `presence` (`userid`, `sd`, `ip`) VALUES('0', '");
						strcat(query, itoa(newfd, tstr1, 10));
						strcat(query, "', '");
						strcat(query, inet_ntoa(clientaddr.sin_addr));
						strcat(query, "')");
						
						db_query(dbconn, query);
						
						connid++;
					}
				} else {
					/* handle data from a client */
					memset(&imsg, '\0', sizeof(imsg));
					if((nbytes = recv(i, (void *)& imsg, sizeof(imsg), 0)) <= 0) {
						/* got error or connection closed by client */
						if(nbytes == 0) {
							/* connection closed */
							printf("[CLIENT-%d] ", i);
							printf("Client disconnected, connection closed.\n");
						} else {
							printf("[CLIENT-%d] ", i);
							printf("Client disconnected.\n");
						}
						
						/* Broadcast off status to online friends */
						strcpy(query, "SELECT DISTINCT `user`.`username`, `presence`.`sd` FROM `user`, `friend` LEFT JOIN `presence` ON (`friend`.`buddy2` = `presence`.`userid`) WHERE `friend`.`buddy2` = `user`.`id` AND `friend`.`buddy1` = (SELECT `userid` AS `buddy1` FROM `presence` WHERE `sd` = '");
						strcat(query, itoa(i, tstr1, 10));
						strcat(query, "')");
						dbres = db_query(dbconn, query);
						
						while((row = mysql_fetch_row(dbres)) != NULL) {
							if(row[1] != NULL) {
								tsd = atoi(row[1]);
								printf("[SERVER] Sending status update to %s @ sd=%d\n", row[0], tsd);
								
								memset(&omsg, '\0', sizeof(protocol));
								
								strcpy(omsg.name, PROTOCOL_NAME);
								omsg.version = htonl(SPEEK_VERSION);
								omsg.service = htonl(SMSG_STATUSUPDATE);
								omsg.status = htonl(SMSG_STATUS_OFFLINE);
								strcpy(omsg.data, "\30");
								strcat(omsg.data, getUsernameFromSD(i));
								
								nbytes = measurePacket(&omsg);
								
								printPacket(omsg);
								
								send(tsd, (void *)& omsg, nbytes, 0);
							}
						}
						
						/* Update presence table */
						strcpy(query, "DELETE FROM `presence` WHERE `sd` = '");
						strcat(query, itoa(i, tstr1, 10));
						strcat(query, "'");
						db_query(dbconn, query);

						/* close it... */
						close(i);
						/* remove from master set */
						FD_CLR(i, &master);
					} else {
						/* we got some data from a client*/
						printf("[CLIENT-%d] Receiving data\n", i);
						
						printPacket(imsg);
						
						if(nbytes = processMessage(i, imsg) > 0) {
							
						} else {
							printf("[CLIENT-%d] Invalid packet.\n", i);
						}
					}
				}
			}
		}
	}
	return 0;
}
