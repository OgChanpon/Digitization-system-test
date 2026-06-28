#include "client.h"

int is_login = 0;
char current_user_id[IDSIZE] = "";
int server_soc = -1;

int recv_response(int soc, char *first_line_buf, char *full_body_buf){
    char buf[BUFSIZE];
    char full_response[BUFSIZE * 10] = ""; 
    
    while(1){
        memset(buf, 0, sizeof(buf));
        int len = recv(soc, buf, sizeof(buf) - 1, 0);
        if(len <= 0){
            printf("\n[通信エラー] サーバーとの接続が切れました\n");
            return -1;
        }
        strcat(full_response, buf);
        if(strstr(full_response, DATA_END) != NULL) break;
    }

    char *end_ptr = strstr(full_response, DATA_END);
    if(end_ptr != NULL) *end_ptr = '\0';

    char *first_line_end = strchr(full_response, '\n');
    if(first_line_end != NULL){
        int first_len = first_line_end - full_response;
        strncpy(first_line_buf, full_response, first_len);
        first_line_buf[first_len] = '\0';
    }
    else{
        strcpy(first_line_buf, full_response);
    }

    if(full_body_buf != NULL){
        strcpy(full_body_buf, full_response);
    }

    printf("\n--- サーバー応答 ---\n%s\n--------------------\n", full_response);
    
    return (strncmp(first_line_buf, OK_STAT, 3) == 0) ? 1 : 0;
}

int main(){
    struct sockaddr_in server_addr;
    char input[BUFSIZE];
    int choice;

    server_soc = socket(AF_INET, SOCK_STREAM, 0);
    if(server_soc < 0){
        perror("ソケットの作成に失敗しました");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if(connect(server_soc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("サーバーへの接続に失敗しました");
        close(server_soc);
        return 1;
    }

    printf("\n>>> サーバーへの接続に成功しました！ <<<\n");

    while(1){
        printf("\n==============================================\n");
        printf("       【各種申請システム】\n");
        printf("==============================================\n");
        
        if(is_login){
            printf(" 👤 ログイン中: %s\n", current_user_id);
            printf("----------------------------------------------\n");
            printf("  1. 様式の一覧を見る (FMTGET)\n");
            printf("  2. 新規様式の作成・編集 (FMTADD)\n");
            printf("  3. 既存様式の更新 (FMTUPD)\n");
            printf("  4. 自分の申請一覧を見る (APPLS)\n");
            printf("  5. 申請の詳細を見る (APPDET)\n");
            printf("  6. 新規申請を作成する (APPADD)\n");
            printf("  7. 申請の修正 (APPUPD)\n");
            printf("  8. 申請の取り下げ (APPDEL)\n");
            printf("  9. 申請書のダウンロード (APPDWN)\n");
            printf(" 10. 新着通知を確認する (NOTICE)\n");
            printf(" 11. 申請の承認・差し戻し (APPCHK)\n");
            printf(" 12. ユーザー登録 (USRREG)\n");
            printf(" 13. ユーザー情報の更新 (USRUPD)\n");
            printf(" 14. ユーザーの削除 (USRDEL)\n");
            printf("----------------------------------------------\n");
            printf(" 99. ログアウト (LOGOUT)\n");
        }
        else{
            printf(" 👤 現在ゲスト状態です\n");
            printf("----------------------------------------------\n");
            printf("  1. ログイン (LOGIN)\n");
        }
        printf("  0. アプリケーションを終了\n");
        printf("==============================================\n");
        printf("番号を入力してください > ");

        if(fgets(input, sizeof(input), stdin) == NULL) break;
        if(sscanf(input, "%d", &choice) != 1) continue;

        if(choice == 0){
            break;
        }

        if(!is_login){
            if(choice == 1){
                Do_Login();
            }
            else{
                printf("[エラー] 正しい番号を選択してください\n");
            }
        }
        else{
            switch(choice){
                case 1: Do_FormatGet(); break;
                case 2: Do_FormatAdd(); break;
                case 3: Do_FormatUpdate(); break;
                case 4: Do_AppGet(); break;
                case 5: Do_AppDetail(); break;
                case 6: Do_AppAdd(); break;
                case 7: Do_AppUpdate(); break;
                case 8: Do_AppDelete(); break;
                case 9: Do_AppDownload(); break;
                case 10: Do_Notice(); break;
                case 11: Do_AppCheck(); break;
                case 12: Do_UserRegister(); break;
                case 13: Do_UserUpdate(); break;
                case 14: Do_UserDelete(); break;
                case 99: Do_Logout(); break;
                default:
                    printf("[エラー] その機能は準備中か，番号が間違っています\n");
            }
        }
  }

    printf("システムを終了します！お疲れ様でした！！！\n");
    close(server_soc);
    return 0;
}
