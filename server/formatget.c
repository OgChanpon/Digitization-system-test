#include "server.h"

int FormatGet(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
    char sql[SQLSIZE];
    snprintf(sql, sizeof(sql), "SELECT format_id, format_name FROM formats ORDER BY format_id;");

    PGresult *res = PQexec(threadParam->con, sql);
    if(PQresultStatus(res) != PGRES_TUPLES_OK){
        printf("[DBエラー] 様式一覧の取得に失敗しました: %s\n", PQerrorMessage(threadParam->con));
        PQclear(res);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_4004, DATA_END);
        return -1;
    }

    int rows = PQntuples(res);

    char ok_msg[BUFSIZE];
    snprintf(ok_msg, sizeof(ok_msg), "%s %d\n", OK_STAT, rows);
    send(threadParam->soc, ok_msg, strlen(ok_msg), 0);

    for(int i = 0; i < rows; i++){
        char *format_id = PQgetvalue(res, i, 0);
        char *format_name = PQgetvalue(res, i, 1);

        char filename[FILENAMESIZE];
        snprintf(filename, sizeof(filename), "formats/%s.txt", format_id);

        FILE *fp = fopen(filename, "r");
        char fields[TEXTSIZE] = "";

        if(fp != NULL){
            char line[256];
            int is_first = 1;

            while(fgets(line, sizeof(line), fp) != NULL){
                line[strcspn(line, "\n")] = '\0';
                if(strlen(line) == 0) continue;

                if(!is_first){
                    strcat(fields, ",");
                }
                strcat(fields, line);
                is_first = 0;
            }
            fclose(fp);
        }
        else{
            strcpy(fields, "項目データなし");
        }

        char lineBuf[BUFSIZE];
        snprintf(lineBuf, sizeof(lineBuf), "%s %s %s\n", format_id, format_name, fields);
        send(threadParam->soc, lineBuf, strlen(lineBuf), 0);
    }

    PQclear(res);

    printf("📡 様式データ (全 %d 件) を結合して送信しました\n", rows);

    return 0;
}
