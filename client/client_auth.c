#include "client.h"

void Do_Login(){
    char input_id[IDSIZE];
    char input_pass[PASSSIZE];
    char input[BUFSIZE];
    char sendBuf[BUFSIZE];
    char recv_first_line[BUFSIZE];

    printf("\n[ログイン]\n");

    printf("ユーザーID > ");
    if(fgets(input, sizeof(input), stdin) != NULL){
        sscanf(input, "%s", input_id);
    }

    printf("パスワード > ");
    if(fgets(input, sizeof(input), stdin) != NULL){
        sscanf(input, "%s", input_pass);
    }

    snprintf(sendBuf, sizeof(sendBuf), "%s %s %s\n", LOGIN, input_id, input_pass);
    send(server_soc, sendBuf, strlen(sendBuf), 0);

    if(recv_response(server_soc, recv_first_line, NULL)){
        printf("✨ ログインに成功しました！\n");
        is_login = 1;
        strncpy(current_user_id, input_id, IDSIZE - 1);
    }
    else{
        printf("❌ ログインに失敗しました。\n");
    }
}

void Do_Logout(){
    char sendBuf[BUFSIZE];
    char recv_first_line[BUFSIZE];

    printf("\n[ログアウト処理を開始します...]\n");

    snprintf(sendBuf, sizeof(sendBuf), "%s\n", LOGOUT);
    send(server_soc, sendBuf, strlen(sendBuf), 0);

    if(recv_response(server_soc, recv_first_line, NULL)){
        printf("👋 ログアウトしました\n");
        is_login = 0;
        memset(current_user_id, 0, sizeof(current_user_id));
    }
    else{
        printf("❌ ログアウトに失敗しました\n");
    }
}

void Do_UserRegister(){
    char input[BUFSIZE];
    char user_id[IDSIZE];
    char password[64];
    char user_name[NAMESIZE];
    char role[ROLESIZE];
    char sendBuf[BUFSIZE];
    char recv_first_line[BUFSIZE];

    printf("\n==============================================\n");
    printf("        【新規ユーザー登録】\n");
    printf("==============================================\n");
    
    printf("ユーザーID (学籍番号など) > ");
    if(fgets(input, sizeof(input), stdin) != NULL) sscanf(input, "%s", user_id);

    printf("パスワード > ");
    if(fgets(input, sizeof(input), stdin) != NULL) sscanf(input, "%s", password);

    printf("氏名 (スペースなしで入力) > ");
    if(fgets(input, sizeof(input), stdin) != NULL) sscanf(input, "%s", user_name);

    printf("権限 (student / teacher / admin) > ");
    if(fgets(input, sizeof(input), stdin) != NULL) sscanf(input, "%s", role);

    snprintf(sendBuf, sizeof(sendBuf), "%s %s %s %s %s\n", USRREG, user_id, password, user_name, role);
    send(server_soc, sendBuf, strlen(sendBuf), 0);

    if(recv_response(server_soc, recv_first_line, NULL)) {
        printf("✨ ユーザー [%s] の登録が完了しました！これでログイン可能です\n", user_name);
    }
    else{
        printf("❌ 登録に失敗しました（すでに存在するIDなどの可能性があります）\n");
    }
}

void Do_UserUpdate(){
    char input[BUFSIZE];
    char user_id[IDSIZE], password[64], user_name[NAMESIZE], role[ROLESIZE];
    char sendBuf[BUFSIZE], recv_first_line[BUFSIZE];

    printf("\n[ユーザー情報の更新]\n対象のユーザーIDを入力 > ");
    if(fgets(input, sizeof(input), stdin) != NULL) sscanf(input, "%s", user_id);

    printf("新しいパスワード > ");
    if(fgets(input, sizeof(input), stdin) != NULL) sscanf(input, "%s", password);

    printf("新しい氏名 (スペースなし) > ");
    if(fgets(input, sizeof(input), stdin) != NULL) sscanf(input, "%s", user_name);

    printf("新しい権限 (student / teacher / admin) > ");
    if(fgets(input, sizeof(input), stdin) != NULL) sscanf(input, "%s", role);

    snprintf(sendBuf, sizeof(sendBuf), "%s %s %s %s %s\n", USRUPD, user_id, password, user_name, role);
    send(server_soc, sendBuf, strlen(sendBuf), 0);

    if(recv_response(server_soc, recv_first_line, NULL)){
        printf("✨ ユーザー [%s] の情報を更新しました！\n", user_id);
    }
    else{
        printf("❌ 更新に失敗しました（権限がない、またはIDが存在しません）\n");
    }
}

void Do_UserDelete(){
    char input[BUFSIZE];
    char user_id[IDSIZE];
    char sendBuf[BUFSIZE], recv_first_line[BUFSIZE];

    printf("\n[ユーザーの削除]\n削除するユーザーIDを入力 > ");
    if(fgets(input, sizeof(input), stdin) != NULL){
        sscanf(input, "%s", user_id);

        printf("⚠️ 本当にユーザー [%s] を削除しますか？ (y/n) > ", user_id);
        fgets(input, sizeof(input), stdin);
        
        if(input[0] == 'y' || input[0] == 'Y'){
            snprintf(sendBuf, sizeof(sendBuf), "%s %s\n", USRDEL, user_id);
            send(server_soc, sendBuf, strlen(sendBuf), 0);
            
            if(recv_response(server_soc, recv_first_line, NULL)){
                printf("🗑️ ユーザー [%s] を削除しました\n", user_id);
            }
            else{
                printf("❌ 削除に失敗しました（権限がない，またはIDが存在しません）\n");
            }
        }
        else{
            printf("❌ 削除をキャンセルしました\n");
        }
    }
}
