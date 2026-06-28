#include "server.h"

int UserRegister(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
    char cmd[BUFSIZE];
    char user_id[IDSIZE];
    char password[64];
    char user_name[NAMESIZE];
    char role[ROLESIZE];

    if(sscanf(recvBuf, "%s %s %s %s %s", cmd, user_id, password, user_name, role) < 5){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_1001, DATA_END);
        return -1;
    }

    char sql[SQLSIZE];
    snprintf(sql, sizeof(sql),
             "INSERT INTO users (user_id, password, user_name, role) VALUES ('%s', '%s', '%s', '%s');",
             user_id, password, user_name, role);

    PGresult *res = PQexec(threadParam->con, sql);
    if(PQresultStatus(res) != PGRES_COMMAND_OK){
        printf("[DBエラー] ユーザー登録に失敗しました: %s\n", PQerrorMessage(threadParam->con));
        PQclear(res);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_4004, DATA_END);
        return -1;
    }
    PQclear(res);

    char ok_msg[BUFSIZE];
    snprintf(ok_msg, sizeof(ok_msg), "%s\n", OK_STAT);
    send(threadParam->soc, ok_msg, strlen(ok_msg), 0);
    
    printf("✨ 新規ユーザー [%s: %s] (権限: %s) を登録しました\n", user_id, user_name, role);

    return 0;
}
