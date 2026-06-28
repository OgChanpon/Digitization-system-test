#include "server.h"

int Notice(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
    if (threadParam->session.is_login == 0) {
        printf("[エラー] 未ログインアクセス\n");
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2002, DATA_END);
        return -1;
    }

    char sql[SQLSIZE];
    char message[TEXTSIZE] = "";

    if(strcmp(threadParam->session.role, "student") == 0){
        snprintf(sql, sizeof(sql),
                 "SELECT count(*) FROM apps WHERE user_id = '%s' AND status IN ('approved', 'rejected');",
                 threadParam->session.user_id);
                 
        PGresult *res = PQexec(threadParam->con, sql);
        if(PQresultStatus(res) == PGRES_TUPLES_OK){
            int count = atoi(PQgetvalue(res, 0, 0));
            if (count > 0){
                snprintf(message, sizeof(message), 
                         "🔔 【通知】あなたの申請のうち、%d件の審査が完了しています！\n「2. 自分の申請一覧」から状況を確認してください\n", count);
            }
            else{
                strcpy(message, "📭 現在、あなた宛の新しい通知はありません\n");
            }
        }
        else{
            strcpy(message, "通知の取得に失敗しました\n");
        }
        PQclear(res);

    }
    else{
        snprintf(sql, sizeof(sql),
                 "SELECT count(*) FROM apps WHERE status = 'submitted';");
                 
        PGresult *res = PQexec(threadParam->con, sql);
        if(PQresultStatus(res) == PGRES_TUPLES_OK){
            int count = atoi(PQgetvalue(res, 0, 0));
            if(count > 0){
                snprintf(message, sizeof(message), 
                         "🔔 【タスク】現在，承認待ち(submitted)の新規申請が %d 件あります\n「6. 申請の承認・差し戻し」から処理を行ってください。\n", count);
            }
            else{
                strcpy(message, "📭 現在，未処理の申請はありません．お疲れ様です！\n");
            }
        }
        else{
            strcpy(message, "通知の取得に失敗しました\n");
        }
        PQclear(res);
    }

    char ok_msg[BUFSIZE];
    snprintf(ok_msg, sizeof(ok_msg), "%s\n", OK_STAT);
    send(threadParam->soc, ok_msg, strlen(ok_msg), 0);
    
    printf("📡 ユーザー [%s] に通知情報を送信しました\n", threadParam->session.user_id);

    return 0;
}
