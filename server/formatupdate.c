#include "server.h"

int FormatUpdate(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
    char cmd[BUFSIZE];
    char format_id[IDSIZE];
    char format_name[NAMESIZE];
    char role[ROLESIZE];
    char notice[NOTICESIZE];
    char fields[TEXTSIZE];

    if(threadParam->session.is_login == 0){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2002, DATA_END);
        return -1;
    }

    if(sscanf(recvBuf, "%s %s %s %s %s %[^\n]", cmd, format_id, format_name, role, notice, fields) < 6){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_1001, DATA_END);
        return -1;
    }

    char sql[SQLSIZE];
    snprintf(sql, sizeof(sql),
             "UPDATE formats SET format_name = '%s', role = '%s', notify_freq = '%s' WHERE format_id = '%s';",
             format_name, role, notice, format_id);

    PGresult *res = PQexec(threadParam->con, sql);
    if(PQresultStatus(res) != PGRES_COMMAND_OK){
        printf("[DBエラー] 様式の更新に失敗しました: %s\n", PQerrorMessage(threadParam->con));
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

    char filename[FILENAMESIZE];
    snprintf(filename, sizeof(filename), "formats/%s.txt", format_id);

    FILE *fp = fopen(filename, "w");
    if(fp == NULL){
        perror("[ファイルエラー] 様式ファイルの更新に失敗しました");
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
    
    printf("✨ 様式 [%s: %s] の定義を更新しました\n", format_id, format_name);

    return 0;
}
