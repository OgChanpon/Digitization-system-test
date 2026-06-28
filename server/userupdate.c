#include "server.h"

int UserUpdate(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
    char cmd[BUFSIZE], user_id[IDSIZE], password[64], user_name[NAMESIZE], role[ROLESIZE];

    if(threadParam->session.is_login == 0){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2002, DATA_END);
        return -1;
    }

    if(sscanf(recvBuf, "%s %s %s %s %s", cmd, user_id, password, user_name, role) < 5){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_1001, DATA_END);
        return -1;
    }

    if(strcmp(threadParam->session.role, "admin") != 0 && strcmp(threadParam->session.user_id, user_id) != 0){
        printf("[警告] ユーザー %s が他人のアカウント情報を更新しようとしました\n", threadParam->session.user_id);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2003, DATA_END);
        return -1;
    }

    char sql[SQLSIZE];
    snprintf(sql, sizeof(sql),
             "UPDATE users SET password = '%s', user_name = '%s', role = '%s' WHERE user_id = '%s';",
             password, user_name, role, user_id);

    PGresult *res = PQexec(threadParam->con, sql);
    if(PQresultStatus(res) != PGRES_COMMAND_OK){
        PQclear(res);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_4004, DATA_END);
        return -1;
    }

    if(atoi(PQcmdTuples(res)) == 0){
        PQclear(res);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_4001, DATA_END);
        return -1;
    }
    PQclear(res);

    char ok_msg[BUFSIZE];
    snprintf(ok_msg, sizeof(ok_msg), "%s\n", OK_STAT);
    send(threadParam->soc, ok_msg, strlen(ok_msg), 0);
    
    printf("✨ ユーザー [%s] の情報を更新しました\n", user_id);

    return 0;
}
