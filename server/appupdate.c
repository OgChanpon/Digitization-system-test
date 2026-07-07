#include "server.h"

int AppUpdate(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
    char cmd[BUFSIZE];
    char app_id[IDSIZE];
    char new_fields[TEXTSIZE];

    if(threadParam->session.is_login == 0){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2002, DATA_END);
        return -1;
    }

    if(sscanf(recvBuf, "%s %s %[^\n]", cmd, app_id, new_fields) < 3){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_1001, DATA_END);
        return -1;
    }

    char sql[SQLSIZE];
    snprintf(sql, sizeof(sql), "SELECT user_id, status FROM apps WHERE app_id = '%s';", app_id);
    PGresult *res = PQexec(threadParam->con, sql);
    
    if(PQntuples(res) == 0){
        PQclear(res);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_4001, DATA_END);
        return -1;
    }
    
    char owner_id[IDSIZE];
    char status[ROLESIZE];
    strcpy(owner_id, PQgetvalue(res, 0, 0));
    strcpy(status, PQgetvalue(res, 0, 1));
    PQclear(res);

    if(strcmp(threadParam->session.role, "student") == 0){
        if(strcmp(threadParam->session.user_id, owner_id) != 0){
            printf("[警告] ユーザー %s が他人の申請 %s を修正しようとしました\n", threadParam->session.user_id, app_id);
            snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2003, DATA_END);
            return -1;
        }
        if(strcmp(status, "submitted") != 0){
            printf("[警告] 審査済みの申請 %s は修正できません\n", app_id);
            snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2003, DATA_END);
            return -1;
        }
    }

    char filename[FILENAMESIZE];
    snprintf(filename, sizeof(filename), "apps_data/%s.txt", app_id);

    FILE *fp = fopen(filename, "w");
    if(fp == NULL){
        perror("[ファイルエラー] 申請ファイルの更新に失敗しました");
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_1003, DATA_END);
        return -1;
    }

    char *token = strtok(new_fields, ",");
    while(token != NULL){
        fprintf(fp, "%s\n", token);
        token = strtok(NULL, ",");
    }
    fclose(fp);
    
    printf("✨ 申請 [%s] のデータを上書き修正しました\n", app_id);

    return 0;
}
