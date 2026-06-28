#ifndef _CLIENT_H_
#define _CLIENT_H_

#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
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
#define MAX_FIELDS 50

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

extern int is_login;
extern char current_user_id[IDSIZE];
extern int server_soc;

int recv_response(int sock, char *first_line_buf, char *full_body_buf);

void Do_Login();
void Do_Logout();
void Do_UserRegister();
void Do_UserUpdate();
void Do_UserDelete();
void Do_AppGet();
void Do_AppDetail();
void Do_AppAdd();
void Do_AppUpdate();
void Do_AppDelete();
void Do_AppDownload();
void Do_AppCheck();
void Do_Notice();
void Do_FormatGet();
void Do_FormatAdd();
void Do_FormatUpdate();

#endif
