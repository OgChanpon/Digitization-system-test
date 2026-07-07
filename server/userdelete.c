#include "server.h"

int UserDelete(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
    char cmd[BUFSIZE], user_id[IDSIZE];

    if(threadParam->session.is_login == 0){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2002, DATA_END);
        return -1;
    }

    if(sscanf(recvBuf, "%s %s", cmd, user_id) < 2){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_1001, DATA_END);
        return -1;
    }

    if(strcmp(threadParam->session.role, "admin") != 0 && strcmp(threadParam->session.user_id, user_id) != 0){
        printf("[警告] ユーザー %s が他人のアカウントを削除しようとしました\n", threadParam->session.user_id);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2003, DATA_END);
        return -1;
    }

    char sql[SQLSIZE];
    snprintf(sql, sizeof(sql), "DELETE FROM users WHERE user_id = '%s';", user_id);

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
    
    printf("🗑️ ユーザー [%s] をシステムから削除しました\n", user_id);

    return 0;
}
