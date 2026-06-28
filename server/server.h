#ifndef _SERVER_H_
#define _SERVER_H_

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
#include<time.h>
#include<unistd.h>

#define BUFSIZE 4096
#define SERVER_IP "127.0.0.1"
#define PORT 10000
#define ENTER "\n"
#define DATA_END ".\n"
#define DELIMITER '\n'

//共通サイズ定義
#define SQLSIZE 2048
#define TEXTSIZE 2048
#define IDSIZE 20
#define PASSSIZE 20
#define NAMESIZE 64
#define ROLESIZE 20
#define NOTICESIZE 20
#define FILENAMESIZE 128
#define DATESIZE 16
#define DATASIZE 1024

//コマンド定義
#define LOGIN "LOGIN"
#define FMTGET "FMTGET"
#define APPADD "APPADD"
#define APPLS "APPLS"
#define APPDET "APPDET"
#define APPUPD "APPUPD"
#define APPDEL "APPDEL"
#define USRREG "USRREG"
#define LOGOUT "LOGOUT"
#define FMTADD "FMTADD"
#define FMTUPD "FMTUPD"
#define APPCHK "APPCHK"
#define NOTICE "NOTICE"
#define USRDEL "USRDEL"
#define USRUPD "USRUPD"
#define APPDWN "APPDWN"

#define OK_STAT "+OK"
#define ER_STAT "-ERR"


//エラーコード
//1000番台：システム・通信エラー
#define E_CODE_1001 1001 //リクエストコマンドの引数エラー
#define E_CODE_1002 1002 //リクエストコマンドが存在しない
#define E_CODE_1003 1003 //ファイル操作・テンプレート作成失敗

//2000番台：認証・権限エラー
#define E_CODE_2001 2001 //ログイン失敗（IDまたはパスワードが違う）
#define E_CODE_2002 2002 //未ログイン
#define E_CODE_2003 2003 //権限エラー

//3000番台：データ入力・形式エラー
#define E_CODE_3001 3001 //入力項目数不足
#define E_CODE_3002 3002 //データ型・形式エラー
#define E_CODE_3003 3003 // 必須項目が空欄

//4000番台：データベースエラー
#define E_CODE_4001 4001 //データ不在（指定されたIDがない等）
#define E_CODE_4002 4002 //データの重複
#define E_CODE_4003 4003 //状態遷移エラー
#define E_CODE_4004 4004 // データベース内部・SQL実行エラー


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
extern int Logout(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int UserRegister(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int UserUpdate(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int UserDelete(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int AppGet(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int AppDetail(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int AppAdd(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int AppUpdate(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int AppDelete(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int AppDownload(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int AppCheck(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int Notice(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int FormatGet(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int FormatAdd(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);
extern int FormatUpdate(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf);

#endif
