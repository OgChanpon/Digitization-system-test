#include "client.h"

void Do_AppGet(){
    char sendBuf[BUFSIZE];
    char recv_first_line[BUFSIZE];

    printf("\n[自分の申請一覧]\n");
    snprintf(sendBuf, sizeof(sendBuf), "%s\n", APPLS);
    send(server_soc, sendBuf, strlen(sendBuf), 0);

    recv_response(server_soc, recv_first_line, NULL);
}

void Do_AppDetail(){
    char input[BUFSIZE];
    char app_id[IDSIZE];
    char sendBuf[BUFSIZE];
    char recv_first_line[BUFSIZE];

    printf("\n[申請の詳細確認]\n詳細を見たい申請IDを入力 > ");
    if(fgets(input, sizeof(input), stdin) != NULL){
        sscanf(input, "%s", app_id);

        snprintf(sendBuf, sizeof(sendBuf), "%s %s\n", APPDET, app_id);
        send(server_soc, sendBuf, strlen(sendBuf), 0);
        recv_response(server_soc, recv_first_line, NULL);
    }
}

void Do_AppAdd(){
    char input[BUFSIZE];
    char format_id[IDSIZE];
    char sendBuf[BUFSIZE];
    char recv_first_line[BUFSIZE];
    char full_body[BUFSIZE * 10];

    printf("\n[新規申請の作成]\n使用する様式IDを入力 (例: F01) > ");
    if(fgets(input, sizeof(input), stdin) == NULL) return;
    sscanf(input, "%s", format_id);

    snprintf(sendBuf, sizeof(sendBuf), "%s\n", FMTGET);
    send(server_soc, sendBuf, strlen(sendBuf), 0);

    char format_name[NAMESIZE] = "";
    char fields_str[TEXTSIZE] = "";
    int found = 0;

    if(recv_response(server_soc, recv_first_line, full_body)){
        char *line = strtok(full_body, ENTER);
        while(line != NULL){
            char tmp_id[IDSIZE], tmp_name[NAMESIZE], tmp_fields[TEXTSIZE];
            if(sscanf(line, "%s %s %[^\n]", tmp_id, tmp_name, tmp_fields) >= 3){
                if(strcmp(tmp_id, format_id) == 0){
                    strcpy(format_name, tmp_name);
                    strcpy(fields_str, tmp_fields);
                    found = 1;
                    break;
                }
            }
            line = strtok(NULL, ENTER);
        }
    }

    if(!found){
        printf("❌ 指定された様式 [%s] が見つかりませんでした\n", format_id);
        return;
    }

    char data_fields_array[MAX_FIELDS][NAMESIZE];
    char user_answers[MAX_FIELDS][TEXTSIZE];
    int field_count = 0;
    int start_index = 0;

    char *token = strtok(fields_str, ",");
    while(token != NULL && field_count < MAX_FIELDS){
        strncpy(data_fields_array[field_count], token, NAMESIZE - 1);
        memset(user_answers[field_count], 0, TEXTSIZE);
        field_count++;
        token = strtok(NULL, ",");
    }

    char draft_filename[FILENAMESIZE];
    FILE *draft_fp;
    snprintf(draft_filename, sizeof(draft_filename), "draft_%s.txt", format_id);
    draft_fp = fopen(draft_filename, "r");

    if(draft_fp != NULL){
        char l_choice[10];
        printf("\n💾 [%s] の【下書き】が見つかりました！\n", format_name);
        printf("続きから入力を再開しますか？ (y/n) > ");
        fgets(input, sizeof(input), stdin); sscanf(input, "%s", l_choice);

        if(strcmp(l_choice, "y") == 0 || strcmp(l_choice, "Y") == 0){
            fscanf(draft_fp, "%d\n", &start_index);
            for(int i = 0; i < start_index; i++){
                char line_buf[TEXTSIZE];
                if(fgets(line_buf, sizeof(line_buf), draft_fp) != NULL){
                    line_buf[strcspn(line_buf, "\n")] = '\0';
                    strcpy(user_answers[i], line_buf);
                }
            }
            printf("▶ [%d] 番目の項目から再開します。\n", start_index + 1);
        }
        fclose(draft_fp);
    }

    int save_triggered = 0;
    printf("\n--- 📝 [%s] の入力を開始します ---\n", format_name);
    for(int i = start_index; i < field_count; i++){
        printf("\n[%d/%d] %s を入力してください\n", i + 1, field_count, data_fields_array[i]);
        if(i > start_index) printf("（※中断して保存する場合は 'save'）\n");
        printf(" > ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if(strcmp(input, "save") == 0){
            draft_fp = fopen(draft_filename, "w");
            if(draft_fp != NULL){
                fprintf(draft_fp, "%d\n", i);
                for(int j = 0; j < i; j++){
                    fprintf(draft_fp, "%s\n", user_answers[j]);
                }
                fclose(draft_fp);
                printf("\n💾 下書きを保存しました．メニューに戻ります．\n");
            }
            save_triggered = 1;
            break;
        }

        strcpy(user_answers[i], input);
    }

    if(save_triggered) return;

    char final_submitted_data[TEXTSIZE] = "";
    for(int i = 0; i < field_count; i++){
        strcat(final_submitted_data, user_answers[i]);
        if(i < field_count - 1) strcat(final_submitted_data, ",");
    }

    printf("\n==============================================\n");
    printf(" 全項目の入力が完了しました！\n");
    printf(" 送信データ: [%s]\n", final_submitted_data);
    printf("==============================================\n");
    printf("サーバーに申請を提出しますか？ (y/n) > ");
    fgets(input, sizeof(input), stdin);
    
    if(input[0] == 'y' || input[0] == 'Y'){
        snprintf(sendBuf, sizeof(sendBuf), "%s %s %s\n", APPADD, format_id, final_submitted_data);
        send(server_soc, sendBuf, strlen(sendBuf), 0);
        
        if(recv_response(server_soc, recv_first_line, NULL)){
            printf("✨ 申請が正常に提出されました！\n");
            remove(draft_filename);
        }
    }
    else{
        printf("❌ 送信をキャンセルしました\n");
    }
}

void Do_AppCheck(){
    char input[BUFSIZE];
    char app_id[IDSIZE];
    char status[ROLESIZE];
    char sendBuf[BUFSIZE];
    char recv_first_line[BUFSIZE];

    printf("\n[申請の承認・差し戻し]\n対象の申請IDを入力 > ");
    if(fgets(input, sizeof(input), stdin) != NULL) sscanf(input, "%s", app_id);

    printf("判定を入力 (approved / rejected) > ");
    if(fgets(input, sizeof(input), stdin) != NULL) sscanf(input, "%s", status);

    snprintf(sendBuf, sizeof(sendBuf), "%s %s %s\n", APPCHK, app_id, status);
    send(server_soc, sendBuf, strlen(sendBuf), 0);

    if(recv_response(server_soc, recv_first_line, NULL)){
        printf("✨ 判定 [%s] を記録しました\n", status);
    }
}

void Do_Notice(){
    char sendBuf[BUFSIZE];
    char recv_first_line[BUFSIZE];

    printf("\n[新着通知の確認]\n");
    snprintf(sendBuf, sizeof(sendBuf), "%s\n", NOTICE);
    send(server_soc, sendBuf, strlen(sendBuf), 0);
    recv_response(server_soc, recv_first_line, NULL);
}

void Do_AppUpdate(){
    char input[BUFSIZE], app_id[IDSIZE], sendBuf[BUFSIZE], recv_first_line[BUFSIZE];
    
    printf("\n--- あなたの提出済み申請一覧 ---\n");
    snprintf(sendBuf, sizeof(sendBuf), "%s\n", APPLS);
    send(server_soc, sendBuf, strlen(sendBuf), 0);
    recv_response(server_soc, recv_first_line, NULL);
    
    printf("\n修正する申請IDをコピーして貼り付けてください > ");
    if(fgets(input, sizeof(input), stdin) == NULL) return;
    sscanf(input, "%s", app_id);

    printf("\n新しい入力データをカンマ区切りで入力してください\n");
    printf("(例: 宮崎太郎,情報科学科,4年)\n> ");
    
    char new_data[TEXTSIZE];
    if(fgets(new_data, sizeof(new_data), stdin) != NULL){
        new_data[strcspn(new_data, "\n")] = '\0';

        snprintf(sendBuf, sizeof(sendBuf), "%s %s %s\n", APPUPD, app_id, new_data);
        send(server_soc, sendBuf, strlen(sendBuf), 0);

        if(recv_response(server_soc, recv_first_line, NULL)){
            printf("✨ 申請 [%s] のデータを修正しました！\n", app_id);
        }
        else{
            printf("❌ 修正に失敗しました（審査済み，または他人の申請の可能性があります）\n");
        }
    }
}

void Do_AppDelete(){
    char input[BUFSIZE];
    char app_id[IDSIZE];
    char sendBuf[BUFSIZE];
    char recv_first_line[BUFSIZE];

    printf("\n[申請の取り下げ]\n取り下げる申請IDを入力 > ");
    if(fgets(input, sizeof(input), stdin) != NULL){
        sscanf(input, "%s", app_id);

        printf("⚠️ 本当に申請 [%s] を取り下げますか？ (y/n) > ", app_id);
        fgets(input, sizeof(input), stdin);
        
        if(input[0] == 'y' || input[0] == 'Y'){
            snprintf(sendBuf, sizeof(sendBuf), "%s %s\n", APPDEL, app_id);
            send(server_soc, sendBuf, strlen(sendBuf), 0);
            
            if(recv_response(server_soc, recv_first_line, NULL)){
                printf("🗑️ 申請 [%s] を完全に取り下げました\n", app_id);
            }
            else{
                printf("❌ 取り下げに失敗しました（審査済み，または他人の申請の可能性があります）\n");
            }
        }
        else{
            printf("❌ 取り下げをキャンセルしました\n");
        }
    }
}

void Do_AppDownload(){
    char input[BUFSIZE];
    char app_id[IDSIZE];
    char file_type[10];
    char sendBuf[BUFSIZE];
    char recv_first_line[BUFSIZE];
    char full_body[BUFSIZE * 10] = "";

    printf("\n[申請書のダウンロード]\n出力したい申請IDを入力 > ");
    if(fgets(input, sizeof(input), stdin) == NULL) return;
    sscanf(input, "%s", app_id);

    printf("出力形式を選択 (1: LuaLaTeX / 2: CSV) > ");
    if(fgets(input, sizeof(input), stdin) == NULL) return;
    
    int choice = atoi(input);
    if(choice == 1) strcpy(file_type, "tex");
    else if(choice == 2) strcpy(file_type, "csv");
    else{
        printf("[エラー] 正しい形式を選択してください\n");
        return;
    }

    snprintf(sendBuf, sizeof(sendBuf), "%s %s %s\n", APPDWN, app_id, file_type);
    send(server_soc, sendBuf, strlen(sendBuf), 0);

    if(recv_response(server_soc, recv_first_line, full_body)){
        char filename[FILENAMESIZE];
        snprintf(filename, sizeof(filename), "download_%s.%s", app_id, file_type);

        FILE *fp = fopen(filename, "w");
        if(fp != NULL){
            char *start_ptr = strchr(full_body, '\n');
            if(start_ptr != NULL){
                start_ptr++;
                fprintf(fp, "%s", start_ptr);
            }
            fclose(fp);

            printf("\n✨ ダウンロード成功！ [%s] として保存しました\n", filename);
            if(choice == 1){
                printf("💡 PDF化するには、ターミナルで以下のコマンドを実行してください：\n");
                printf("   $ lualatex %s\n", filename);
            }
        }
        else{
            printf("❌ ローカルへのファイル保存に失敗しました\n");
        }
    }
    else{
        printf("❌ ダウンロードに失敗しました（IDが存在しない、または権限がありません）\n");
    }
}
