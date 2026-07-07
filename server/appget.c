#include "server.h"

int AppGet(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
    if(threadParam->session.is_login == 0){
        printf("[エラー] 未ログインのユーザーからのアクセスを弾きました\n");
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2002, DATA_END);
        return -1;
    }

    char sql[SQLSIZE];
    snprintf(sql, sizeof(sql),
             "SELECT app_id, format_id, status, created_at FROM apps WHERE user_id = '%s' ORDER BY created_at DESC;",
             threadParam->session.user_id);

    PGresult *res = PQexec(threadParam->con, sql);
    if(PQresultStatus(res) != PGRES_TUPLES_OK){
        printf("[DBエラー] 申請一覧の取得に失敗しました: %s\n", PQerrorMessage(threadParam->con));
        PQclear(res);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_4004, DATA_END);
        return -1;
    }

    int rows = PQntuples(res);

    char ok_msg[BUFSIZE];
    snprintf(ok_msg, sizeof(ok_msg), "%s %d\n", OK_STAT, rows);
    send(threadParam->soc, ok_msg, strlen(ok_msg), 0);

    for(int i = 0; i < rows; i++){
        char *app_id = PQgetvalue(res, i, 0);
        char *format_id = PQgetvalue(res, i, 1);
        char *status = PQgetvalue(res, i, 2);
        char *created_at = PQgetvalue(res, i, 3);
        char lineBuf[BUFSIZE];

        snprintf(lineBuf, sizeof(lineBuf), "%s %s %s %s\n", app_id, format_id, status, created_at);
        send(threadParam->soc, lineBuf, strlen(lineBuf), 0);
    }

    PQclear(res);
    
    printf("📡 ユーザー [%s] に申請一覧 (全 %d 件) を送信しました\n", threadParam->session.user_id, rows);

    return 0;
}
