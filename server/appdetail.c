#include "server.h"

int AppDetail(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
    char cmd[BUFSIZE];
    char app_id[IDSIZE];

    if(threadParam->session.is_login == 0){
        printf("[エラー] 未ログインアクセス\n");
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2002, DATA_END);
        return -1;
    }

    if(sscanf(recvBuf, "%s %s", cmd, app_id) < 2){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_1001, DATA_END);
        return -1;
    }

    char sql[SQLSIZE];
    snprintf(sql, sizeof(sql), "SELECT format_id, status, user_id FROM apps WHERE app_id = '%s';", app_id);

    PGresult *res = PQexec(threadParam->con, sql);
    if(PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0){
        PQclear(res);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_4001, DATA_END);
        return -1;
    }

    char format_id[IDSIZE];
    char status[ROLESIZE];
    char owner_id[IDSIZE];
    strcpy(format_id, PQgetvalue(res, 0, 0));
    strcpy(status, PQgetvalue(res, 0, 1));
    strcpy(owner_id, PQgetvalue(res, 0, 2));
    PQclear(res);

    if(strcmp(threadParam->session.role, "student") == 0 && strcmp(threadParam->session.user_id, owner_id) != 0){
        printf("[警告] ユーザー %s が他人の申請 %s を覗き見ようとしました\n", threadParam->session.user_id, app_id);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2003, DATA_END);
        return -1;
    }

    char fmt_filename[FILENAMESIZE];
    char data_filename[FILENAMESIZE];
    snprintf(fmt_filename, sizeof(fmt_filename), "formats/%s.txt", format_id);
    snprintf(data_filename, sizeof(data_filename), "apps_data/%s.txt", app_id);

    FILE *fp_fmt = fopen(fmt_filename, "r");
    FILE *fp_data = fopen(data_filename, "r");

    if(fp_fmt == NULL || fp_data == NULL){
        if (fp_fmt) fclose(fp_fmt);
        if (fp_data) fclose(fp_data);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_1003, DATA_END);
        return -1;
    }

    char ok_msg[BUFSIZE];
    snprintf(ok_msg, sizeof(ok_msg), "%s\n", OK_STAT);
    send(threadParam->soc, ok_msg, strlen(ok_msg), 0);

    char header[BUFSIZE];
    snprintf(header, sizeof(header), 
             "【申請ID】 %s\n【申請者】 %s\n【様式ID】 %s\n【状態】 %s\n--------------------------------------\n", 
             app_id, owner_id, format_id, status);
    send(threadParam->soc, header, strlen(header), 0);

    char fmt_line[TEXTSIZE];
    char data_line[TEXTSIZE];

    while (fgets(fmt_line, sizeof(fmt_line), fp_fmt) != NULL && fgets(data_line, sizeof(data_line), fp_data) != NULL) {
        fmt_line[strcspn(fmt_line, "\n")] = '\0';
        data_line[strcspn(data_line, "\n")] = '\0';

        char lineBuf[BUFSIZE];
        snprintf(lineBuf, sizeof(lineBuf), " %-20s : %s\n", fmt_line, data_line);
        send(threadParam->soc, lineBuf, strlen(lineBuf), 0);
    }

    fclose(fp_fmt);
    fclose(fp_data);
    
    printf("📡 申請詳細 [%s] をハイブリッド結合して送信しました\n", app_id);

    return 0;
}
