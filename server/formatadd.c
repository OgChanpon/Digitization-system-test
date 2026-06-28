#include "server.h"

int FormatAdd(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
    char cmd[BUFSIZE];
    char format_id[IDSIZE];
    char format_name[NAMESIZE];
    char role[ROLESIZE];
    char notice[NOTICESIZE];
    char fields[TEXTSIZE];

    if(sscanf(recvBuf, "%s %s %s %s %s %[^\n]", cmd, format_id, format_name, role, notice, fields) < 6){
        printf("[エラー] FMTADDの引数が不足しています\n");
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_1001, DATA_END);
        return -1;
    }

    char sql[SQLSIZE];
    snprintf(sql, sizeof(sql),
             "INSERT INTO formats (format_id, format_name, role, notify_freq) VALUES ('%s', '%s', '%s', '%s');",
             format_id, format_name, role, notice);

    PGresult *res = PQexec(threadParam->con, sql);
    if(PQresultStatus(res) != PGRES_COMMAND_OK){
        printf("[DBエラー] 様式の登録に失敗しました: %s\n", PQerrorMessage(threadParam->con));
        PQclear(res);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_4004, DATA_END);
        return -1;
    }
    PQclear(res);

    char filename[FILENAMESIZE];
    snprintf(filename, sizeof(filename), "formats/%s.txt", format_id);

    FILE *fp = fopen(filename, "w");
    if(fp == NULL){
        perror("[ファイルエラー] 様式ファイルの作成に失敗しました");
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
    
    printf("✨ 新様式 [%s: %s] を %s に保存しました！\n", format_id, format_name, filename);

    return 0;
}
