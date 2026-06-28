#include "server.h"

int AppCheck(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
    char cmd[BUFSIZE];
    char app_id[IDSIZE];
    char status[ROLESIZE];

    if(threadParam->session.is_login == 0){
        printf("[エラー] 未ログインアクセス\n");
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2002, DATA_END);
        return -1;
    }

    if(strcmp(threadParam->session.role, "student") == 0){
        printf("[警告] 学生ユーザー %s が承認処理を行おうとしました (アクセス拒否)\n", threadParam->session.user_id);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2003, DATA_END);
        return -1;
    }

    if(sscanf(recvBuf, "%s %s %s", cmd, app_id, status) < 3){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_1001, DATA_END);
        return -1;
    }

    char sql[SQLSIZE];
    snprintf(sql, sizeof(sql),
             "UPDATE apps SET status = '%s' WHERE app_id = '%s';",
             status, app_id);

    PGresult *res = PQexec(threadParam->con, sql);
    if(PQresultStatus(res) != PGRES_COMMAND_OK){
        printf("[DBエラー] 申請の更新に失敗しました: %s\n", PQerrorMessage(threadParam->con));
        PQclear(res);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_4004, DATA_END);
        return -1;
    }

    if(atoi(PQcmdTuples(res)) == 0){
        printf("[エラー] 存在しない申請ID (%s) の更新が試みられました\n", app_id);
        PQclear(res);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_4001, DATA_END);
        return -1;
    }
    PQclear(res);

    char ok_msg[BUFSIZE];
    snprintf(ok_msg, sizeof(ok_msg), "%s\n", OK_STAT);
    send(threadParam->soc, ok_msg, strlen(ok_msg), 0);
    
    printf("✨ 申請 [%s] のステータスを [%s] に更新しました！(処理者: %s)\n", 
           app_id, status, threadParam->session.user_id);

    return 0;
}
