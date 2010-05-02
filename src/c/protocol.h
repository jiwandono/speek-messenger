typedef struct {
	char name[4];
	int version;
	int pkt_len;
	int service;
	int status;
	char data[32768];
} protocol;

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
 * \30 username \31 password
 * Server:
 * Server akan mengembalikan status 1 kalau login sukses, 0 kalau gagal
 *
 * SMSG_SERVICE_LOGOFF
 * Client:
 * \30 username
 *
 * SMSG_MESSAGE
 * Client:
 * \30 sender \31 receiver \31 message
 * Server:
 * Teruskan ke client receiver
 *
 * SMSG_RETFRIEND
 * Client:
 * \30 username
 * Server:
 * \30 friendname1 \31 status1 \30 friendname \31 status2
 *
 * SMSG_ADDFRIEND
 * Client:
 * \30 username \31 friendname
 * Server:
 * Status 1 kalau berhasil, 0 kalau gagal
 *
 * SMSG_REMFRIEND
 * Client:
 * \30 username \31 friendname
 * Server:
 * Status 1 kalau berhasil, 0 kalau gagal
 *
 * SMSG_STATUS_UPDATE
 * Server:
 * \30 username \31 status
 *
 * SMSG_REGISTER
 * Client:
 * \30 username \31 status
 * Server:
 * Status 1 kalau berhasil, 0 kalau gagal
 */

