#include<arpa/inet.h>
#include<netinet/in.h>
#include<pthread.h>
#include<postgresql/libpq-fe.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<time.h>
#include<unistd.h>

#define BUFSIZE 4096
#define PORT 10000
#define ENTER "\n"
#define DATA_END ".\n"
#define DELIMITER '\n'

#define LOGIN "LOGIN"
#define FMTGT "FMTGT"
#define APPAD "APPAD"
#define APLGT "APLGT"
#define APDET "APDET"
#define APUPD "APUPD"
#define APDLE "APDLE"
#define REGST "REGST"
#define LOGOT "LOGOT"

#define OK_STAT "+OK"
#define ER_STAT "-ERR"

#define E_CODE_1 100 // リクエストコマンドの引数エラー
#define E_CODE_2 200 // リクエストコマンドが存在しない
#define E_CODE_3 300 //データベースエラー

typedef struct _UserSession{
  int is_login;
  char user_id[20];
  char role[20];
}UserSession;

typedef struct _ThreadParameter{
  struct in_addr c_ipaddr;
  in_port_t c_port;
  int soc;
  PGconn *con;
  UserSession session;
}ThreadParameter;

extern int receive_message(int __s, char *__buf, int __maxlen);
extern int setup_listen(u_short __port);
extern void *controller(void *arg);

extern int Login(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int FormatGet(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int AppAdd(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int AppGet(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int AppDetail(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int AppUpdate(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int AppDelete(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int Register(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int Logout(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
