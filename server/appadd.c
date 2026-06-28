#include "server.h"

int AppAdd(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
    char cmd[BUFSIZE];
    char format_id[IDSIZE];
    char fields[TEXTSIZE];

    if(threadParam->session.is_login == 0){
        printf("[エラー] 未ログインのユーザーからの申請を弾きました\n");
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2002, DATA_END);
        return -1;
    }

    if(sscanf(recvBuf, "%s %s %[^\n]", cmd, format_id, fields) < 3){
        printf("[エラー] APPADDの引数が不足しています。\n");
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_1001, DATA_END);
        return -1;
    }

    char app_id[IDSIZE];
    snprintf(app_id, sizeof(app_id), "A%ld", (long)time(NULL));
    char sql[SQLSIZE];

    snprintf(sql, sizeof(sql),
             "INSERT INTO apps (app_id, user_id, format_id, status) VALUES ('%s', '%s', '%s', 'submitted');",
             app_id, threadParam->session.user_id, format_id);

    PGresult *res = PQexec(threadParam->con, sql);
    if(PQresultStatus(res) != PGRES_COMMAND_OK){
        printf("[DBエラー] 申請の登録に失敗しました: %s\n", PQerrorMessage(threadParam->con));
        PQclear(res);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_4004, DATA_END);
        return -1;
    }
    PQclear(res);

    char filename[FILENAMESIZE];

    snprintf(filename, sizeof(filename), "apps/%s.txt", app_id);

    FILE *fp = fopen(filename, "w");
    if(fp == NULL){
        perror("[ファイルエラー] 申請ファイルの作成に失敗しました");
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_1003, DATA_END);
        return -1;
    }

    char *token = strtok(fields, ",");
    while(token != NULL){
        fprintf(fp, "%s\n", token);
        token = strtok(NULL, ",");
    }
    fclose(fp);

    char ok_msg[BUFSIZE];
    snprintf(ok_msg, sizeof(ok_msg), "%s\n", OK_STAT);
    send(threadParam->soc, ok_msg, strlen(ok_msg), 0);
    
    printf("✨ 新規申請 [%s] (ユーザー: %s, 様式: %s) を保存しました！\n", 
           app_id, threadParam->session.user_id, format_id);

    return 0;
}
