#include "client.h"

void Do_FormatGet(){
    char sendBuf[BUFSIZE];
    char recv_first_line[BUFSIZE];

    printf("\n[様式の一覧]\n");
    snprintf(sendBuf, sizeof(sendBuf), "%s\n", FMTGET);
    send(server_soc, sendBuf, strlen(sendBuf), 0);

    recv_response(server_soc, recv_first_line, NULL);
}

void Do_FormatAdd(){
    char input_mode[BUFSIZE];
    int mode = 0;
    
    printf("\n==============================================\n");
    printf("        【新規様式の作成ウィザード】\n");
    printf("==============================================\n");
    printf(" 1. 一から新規作成する\n");
    printf(" 2. 既存の様式をコピーして作成する\n");
    printf(" > ");
    if(fgets(input_mode, sizeof(input_mode), stdin) != NULL){
        sscanf(input_mode, "%d", &mode);
    }

    if(mode != 1 && mode != 2){
        printf("[エラー] 正しい番号を選択してください\n");
        return;
    }

    char fields_array[MAX_FIELDS][NAMESIZE];
    int field_count = 0;

    if(mode == 2){
        char base_id[IDSIZE];
        char full_body[BUFSIZE * 10];
        
        printf("\nベースにする様式IDを入力してください (例: F01) > ");
        if(fgets(input_mode, sizeof(input_mode), stdin) == NULL) return;
        sscanf(input_mode, "%s", base_id);

        char sendBuf[BUFSIZE];
        char recv_first_line[BUFSIZE];
        snprintf(sendBuf, sizeof(sendBuf), "%s\n", FMTGET);
        send(server_soc, sendBuf, strlen(sendBuf), 0);
        
        if(recv_response(server_soc, recv_first_line, full_body)){
            char *line = strtok(full_body, ENTER);
            int found = 0;
            
            while(line != NULL){
                char tmp_id[IDSIZE], tmp_name[NAMESIZE], tmp_fields[TEXTSIZE];
                if(sscanf(line, "%s %s %[^\n]", tmp_id, tmp_name, tmp_fields) >= 3){
                    if(strcmp(tmp_id, base_id) == 0){
                        printf("\n📡 サーバーから [%s: %s] の設計図を取得しました！\n", tmp_id, tmp_name);
                        
                        char *token = strtok(tmp_fields, ",");
                        while(token != NULL && field_count < MAX_FIELDS){
                            strncpy(fields_array[field_count], token, NAMESIZE - 1);
                            field_count++;
                            token = strtok(NULL, ",");
                        }
                        found = 1;
                        break;
                    }
                }
                line = strtok(NULL, ENTER);
            }
            
            if(!found){
                printf("❌ 指定された様式が見つかりませんでした\n");
                return;
            }
        }
        else{
            return;
        }
    }

    printf("\n--- 📝 項目の編集モード ---\n");
    while(1){
        printf("\n[現在の項目一覧]\n");
        for(int i = 0; i < field_count; i++){
            printf(" %2d: %s\n", i + 1, fields_array[i]);
        }
        
        printf("\n操作を入力してください\n");
        printf("(追加: 'add 項目名' / 挿入: 'ins 番号 項目名' / 修正: 'mod 番号 新項目名' / 削除: 'del 番号' / 完了: 'end') > ");
        
        char edit_cmd[BUFSIZE];
        if(fgets(edit_cmd, sizeof(edit_cmd), stdin) == NULL) break;
        edit_cmd[strcspn(edit_cmd, "\n")] = '\0';

        if(strcmp(edit_cmd, "end") == 0){
            if(field_count == 0){
                printf("[警告] 最低1つは項目を入力してください\n");
                continue;
            }
            break;
        } 
        else if(strncmp(edit_cmd, "del ", 4) == 0){
            int del_idx;
            if(sscanf(edit_cmd, "del %d", &del_idx) == 1){
                del_idx--;
                if(del_idx >= 0 && del_idx < field_count){
                    printf("🗑️ [%s] を削除しました\n", fields_array[del_idx]);
                    for(int i = del_idx; i < field_count - 1; i++){
                        strcpy(fields_array[i], fields_array[i + 1]);
                    }
                    field_count--;
                }
                else{
                    printf("[エラー] 存在しない番号です\n");
                }
            }
        }
        else if(strncmp(edit_cmd, "add ", 4) == 0){
            char *add_str = edit_cmd + 4;
            if(strlen(add_str) > 0 && field_count < MAX_FIELDS){
                strncpy(fields_array[field_count], add_str, NAMESIZE - 1);
                printf("✨ [%s] を追加しました\n", fields_array[field_count]);
                field_count++;
            }
            else if(field_count >= MAX_FIELDS){
                printf("[エラー] 項目数が上限 (%d) に達しています\n", MAX_FIELDS);
            }
        }
        else if(strncmp(edit_cmd, "mod ", 4) == 0){
            int mod_idx;
            char new_name[NAMESIZE];
            if(sscanf(edit_cmd, "mod %d %s", &mod_idx, new_name) == 2){
                mod_idx--;
                if(mod_idx >= 0 && mod_idx < field_count){
                    printf("🔄 [%s] を [%s] に変更しました\n", fields_array[mod_idx], new_name);
                    strncpy(fields_array[mod_idx], new_name, NAMESIZE - 1);
                }
                else{
                    printf("[エラー] 存在しない番号です\n");
                }
            }
            else{
                printf("[エラー] 形式が違います．例: mod 3 新項目名\n");
            }
        }
        else if(strncmp(edit_cmd, "ins ", 4) == 0){
            int ins_idx;
            char ins_name[NAMESIZE];
            if(sscanf(edit_cmd, "ins %d %s", &ins_idx, ins_name) == 2){
                ins_idx--;
                if(ins_idx >= 0 && ins_idx <= field_count && field_count < MAX_FIELDS){
                    for(int i = field_count; i > ins_idx; i--){
                        strcpy(fields_array[i], fields_array[i - 1]);
                    }
                    strncpy(fields_array[ins_idx], ins_name, NAMESIZE - 1);
                    field_count++;
                    printf("✨ %d番目に [%s] を挿入しました\n", ins_idx + 1, ins_name);
                }
                else{
                    printf("[エラー] 番号が範囲外か，項目数が上限です\n");
                }
            }
            else{
                printf("[エラー] 形式が違います．例: ins 3 割り込み項目名\n");
            }
        }
        else{
            printf("[エラー] コマンドの形式が間違っています\n");
        }
    }

    char new_format_id[IDSIZE];
    char new_format_name[NAMESIZE];
    char role[ROLESIZE];
    char notice[NOTICESIZE];

    printf("\n==============================================\n");
    printf("登録する新しい様式の情報を入力してください\n");
    
    printf("様式ID (例: F02) > ");
    if(fgets(input_mode, sizeof(input_mode), stdin) != NULL) sscanf(input_mode, "%s", new_format_id);
    
    printf("様式名 (例: 備品購入届) > ");
    if(fgets(input_mode, sizeof(input_mode), stdin) != NULL){
        input_mode[strcspn(input_mode, "\n")] = '\0';
        strncpy(new_format_name, input_mode, NAMESIZE - 1);
    }

    printf("利用権限 (student/teacher/admin) > ");
    if(fgets(input_mode, sizeof(input_mode), stdin) != NULL) sscanf(input_mode, "%s", role);

    printf("通知頻度 (realtime/daily/weekly) > ");
    if(fgets(input_mode, sizeof(input_mode), stdin) != NULL) sscanf(input_mode, "%s", notice);

    char final_format_text[TEXTSIZE] = "";
    for(int i = 0; i < field_count; i++){
        strcat(final_format_text, fields_array[i]);
        if(i < field_count - 1) strcat(final_format_text, ",");
    }

    printf("\n最終確認: [%s] をサーバーに登録しますか？ (y/n) > ", new_format_name);
    if(fgets(input_mode, sizeof(input_mode), stdin) != NULL){
        if(input_mode[0] == 'y' || input_mode[0] == 'Y'){
            char sendBuf[BUFSIZE];
            char recv_first_line[BUFSIZE];
            snprintf(sendBuf, sizeof(sendBuf), "%s %s %s %s %s %s\n", 
                     FMTADD, new_format_id, new_format_name, role, notice, final_format_text);
            send(server_soc, sendBuf, strlen(sendBuf), 0);
            
            if(recv_response(server_soc, recv_first_line, NULL)){
                printf("✨ 新様式 [%s] の登録とLaTeXテンプレートの生成が完了しました！\n", new_format_name);
            }
        }
        else{
            printf("❌ 登録をキャンセルしました．\n");
        }
    }
}

void Do_FormatUpdate(){
    char input_mode[BUFSIZE];
    char target_id[IDSIZE];
    char sendBuf[BUFSIZE];
    char recv_first_line[BUFSIZE];
    char full_body[BUFSIZE * 10];
    
    printf("\n==============================================\n");
    printf("        【既存様式の編集・更新】\n");
    printf("==============================================\n");
    printf("更新したい様式IDを入力してください (例: F01) > ");
    if(fgets(input_mode, sizeof(input_mode), stdin) == NULL) return;
    sscanf(input_mode, "%s", target_id);

    snprintf(sendBuf, sizeof(sendBuf), "%s\n", FMTGET);
    send(server_soc, sendBuf, strlen(sendBuf), 0);
    
    char fields_array[MAX_FIELDS][NAMESIZE];
    int field_count = 0;
    int found = 0;
    char current_name[NAMESIZE] = "";

    if(recv_response(server_soc, recv_first_line, full_body)){
        char *line = strtok(full_body, ENTER);
        while(line != NULL){
            char tmp_id[IDSIZE], tmp_name[NAMESIZE], tmp_fields[TEXTSIZE];
            if(sscanf(line, "%s %s %[^\n]", tmp_id, tmp_name, tmp_fields) >= 3){
                if(strcmp(tmp_id, target_id) == 0){
                    strcpy(current_name, tmp_name);
                    printf("\n📡 [%s: %s] の設計図を読み込みました！\n", tmp_id, tmp_name);
                    
                    char *token = strtok(tmp_fields, ",");
                    while(token != NULL && field_count < MAX_FIELDS){
                        strncpy(fields_array[field_count], token, NAMESIZE - 1);
                        field_count++;
                        token = strtok(NULL, ",");
                    }
                    found = 1;
                    break;
                }
            }
            line = strtok(NULL, ENTER);
        }
    }

    if(!found){
        printf("❌ 指定された様式が見つかりませんでした\n");
        return;
    }

    printf("\n--- 📝 項目の編集モード ---\n");
    while(1){
        printf("\n[現在の項目一覧]\n");
        for(int i = 0; i < field_count; i++){
            printf(" %2d: %s\n", i + 1, fields_array[i]);
        }
        
        printf("\n操作を入力してください\n");
        printf("(追加: 'add 項目名' / 挿入: 'ins 番号 項目名' / 修正: 'mod 番号 新項目名' / 削除: 'del 番号' / 完了: 'end') > ");
        
        char edit_cmd[BUFSIZE];
        if(fgets(edit_cmd, sizeof(edit_cmd), stdin) == NULL) break;
        edit_cmd[strcspn(edit_cmd, "\n")] = '\0';

        if(strcmp(edit_cmd, "end") == 0){
            if(field_count == 0){
                printf("[警告] 最低1つは項目を入力してください\n");
                continue;
            }
            break;
        } 
        else if(strncmp(edit_cmd, "del ", 4) == 0){
            int del_idx;
            if(sscanf(edit_cmd, "del %d", &del_idx) == 1){
                del_idx--;
                if(del_idx >= 0 && del_idx < field_count){
                    printf("🗑️ [%s] を削除しました\n", fields_array[del_idx]);
                    for(int i = del_idx; i < field_count - 1; i++){
                        strcpy(fields_array[i], fields_array[i + 1]);
                    }
                    field_count--;
                }
            }
        }
        else if(strncmp(edit_cmd, "add ", 4) == 0){
            char *add_str = edit_cmd + 4;
            if(strlen(add_str) > 0 && field_count < MAX_FIELDS){
                strncpy(fields_array[field_count], add_str, NAMESIZE - 1);
                field_count++;
            }
        } 
    }

    char role[ROLESIZE] = "student";
    char notice[NOTICESIZE] = "realtime";
    
    char final_format_text[TEXTSIZE] = "";
    for(int i = 0; i < field_count; i++){
        strcat(final_format_text, fields_array[i]);
        if(i < field_count - 1) strcat(final_format_text, ",");
    }

    printf("\n最終確認: 様式 [%s] をサーバー上で上書き更新しますか？ (y/n) > ", current_name);
    if(fgets(input_mode, sizeof(input_mode), stdin) != NULL){
        if(input_mode[0] == 'y' || input_mode[0] == 'Y'){
            snprintf(sendBuf, sizeof(sendBuf), "%s %s %s %s %s %s\n", 
                     FMTUPD, target_id, current_name, role, notice, final_format_text);
            send(server_soc, sendBuf, strlen(sendBuf), 0);
            
            if(recv_response(server_soc, recv_first_line, NULL)){
                printf("✨ 様式 [%s] の更新が完了しました！\n", current_name);
            }
        }
        else{
            printf("❌ 更新をキャンセルしました\n");
        }
    }
}
