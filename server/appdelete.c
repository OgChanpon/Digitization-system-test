#include "server.h"

int AppDelete(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
    char cmd[BUFSIZE];
    char app_id[IDSIZE];

    if(threadParam->session.is_login == 0){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2002, DATA_END);
        return -1;
    }

    if(sscanf(recvBuf, "%s %s", cmd, app_id) < 2){
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
            printf("[警告] ユーザー %s が他人の申請 %s を削除しようとしました\n", threadParam->session.user_id, app_id);
            snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2003, DATA_END);
            return -1;
        }
        if(strcmp(status, "submitted") != 0){
            printf("[警告] 審査済みの申請 %s は取り下げできません\n", app_id);
            snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2003, DATA_END);
            return -1;
        }
    }

    snprintf(sql, sizeof(sql), "DELETE FROM apps WHERE app_id = '%s';", app_id);
    res = PQexec(threadParam->con, sql);
    if(PQresultStatus(res) != PGRES_COMMAND_OK){
        PQclear(res);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_4004, DATA_END);
        return -1;
    }
    PQclear(res);

    char filename[FILENAMESIZE];
    snprintf(filename, sizeof(filename), "apps_data/%s.txt", app_id);
    if(remove(filename) == 0){
        printf("🗑️ 申請データファイル %s を削除しました\n", filename);
    }
    else{
        perror("[警告] ファイルの削除に失敗しましたが、DBからは削除済です");
    }

    char ok_msg[BUFSIZE];
    snprintf(ok_msg, sizeof(ok_msg), "%s\n", OK_STAT);
    send(threadParam->soc, ok_msg, strlen(ok_msg), 0);
    
    printf("✨ 申請 [%s] を完全に取り下げました\n", app_id);

    return 0;
}
