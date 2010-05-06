#ifndef _SPEEK_PROTOCOL
#define _SPEEK_PROTOCOL

#define PROTOCOL_NAME "SMSG"

#define PORT 7674

typedef struct {
	char name[4];
	int version;
	int pkt_len;
	int service;
	int status;
	char data[32768];
} protocol;

#define SPEEK_VERSION	1

#define SMSG_SERVICE_LOGON	1
#define SMSG_SERVICE_LOGOFF	2
#define SMSG_MESSAGE		3
#define SMSG_FILETRANSFER	4
#define SMSG_ADDFRIEND		5
#define SMSG_REMFRIEND		6
#define SMSG_RETFRIEND		7
#define SMSG_STATUSUPDATE	8
#define SMSG_REGISTER		9

#define SMSG_STATUS_FAILED	0
#define SMSG_STATUS_SUCCESS	1
#define SMSG_STATUS_ONLINE	2
#define SMSG_STATUS_OFFLINE	3

/*
 * SMSG_SERVICE_LOGON
 * Client:
 * \30username\31password
 * Server:
 * Server akan mengembalikan status 1 kalau login sukses, 0 kalau gagal
 *
 * SMSG_SERVICE_LOGOFF
 * Client:
 * \30username
 *
 * SMSG_MESSAGE
 * Client:
 * \30sender\31receiver\31message
 * Server:
 * Teruskan ke client receiver
 *
 * SMSG_RETFRIEND
 * Client:
 * \30username
 * Server:
 * \30friendname1\31status1\30friendname\31status2
 *
 * SMSG_ADDFRIEND
 * Client:
 * \30username\31friendname
 * Server:
 * Status 1 kalau berhasil, 0 kalau gagal
 *
 * SMSG_REMFRIEND
 * Client:
 * \30username\31friendname
 * Server:
 * Status 1 kalau berhasil, 0 kalau gagal
 *
 * SMSG_STATUS_UPDATE
 * Server:
 * \30username\31status
 *
 * SMSG_REGISTER
 * Client:
 * \30username\31status
 * Server:
 * Status 1 kalau berhasil, 0 kalau gagal
 *
 * SMSG_FILETRANSFER
 * Client:
 * \30targetusername
 * Server
 * Status: SMSG_STATUS_SUCCESS atau SMSG_STATUS_FAILED
 * \30ipaddress
 *
 */

#endif
