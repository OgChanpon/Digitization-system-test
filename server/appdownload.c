#include "server.h"

int AppDownload(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
    char cmd[BUFSIZE];
    char app_id[IDSIZE];
    char file_type[10];

    if(threadParam->session.is_login == 0){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2002, DATA_END);
        return -1;
    }

    if(sscanf(recvBuf, "%s %s %s", cmd, app_id, file_type) < 3){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_1001, DATA_END);
        return -1;
    }

    char sql[SQLSIZE];
    snprintf(sql, sizeof(sql), 
             "SELECT format_id, user_id, status FROM apps WHERE app_id = '%s';", app_id);
    PGresult *res = PQexec(threadParam->con, sql);
    
    if(PQntuples(res) == 0){
        PQclear(res);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_4001, DATA_END);
        return -1;
    }
    
    char format_id[IDSIZE], owner_id[IDSIZE], status[ROLESIZE];
    strcpy(format_id, PQgetvalue(res, 0, 0));
    strcpy(owner_id, PQgetvalue(res, 0, 1));
    strcpy(status, PQgetvalue(res, 0, 2));
    PQclear(res);

    if(strcmp(threadParam->session.role, "student") == 0 && strcmp(threadParam->session.user_id, owner_id) != 0){
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_2003, DATA_END);
        return -1;
    }

    char format_name[NAMESIZE] = "申請書";
    snprintf(sql, sizeof(sql), "SELECT format_name FROM formats WHERE format_id = '%s';", format_id);
    res = PQexec(threadParam->con, sql);
    if(PQntuples(res) > 0) strcpy(format_name, PQgetvalue(res, 0, 0));
    PQclear(res);

    char fmt_filename[FILENAMESIZE], data_filename[FILENAMESIZE];
    snprintf(fmt_filename, sizeof(fmt_filename), "formats/%s.txt", format_id);
    snprintf(data_filename, sizeof(data_filename), "apps_data/%s.txt", app_id);

    FILE *fp_fmt = fopen(fmt_filename, "r");
    FILE *fp_data = fopen(data_filename, "r");

    if(fp_fmt == NULL || fp_data == NULL){
        if(fp_fmt) fclose(fp_fmt);
        if(fp_data) fclose(fp_data);
        snprintf(sendBuf, BUFSIZE, "%s %d\n%s", ER_STAT, E_CODE_1003, DATA_END);
        return -1;
    }

    char ok_msg[BUFSIZE];
    snprintf(ok_msg, sizeof(ok_msg), "%s\n", OK_STAT);
    send(threadParam->soc, ok_msg, strlen(ok_msg), 0);

    char fmt_line[TEXTSIZE], data_line[TEXTSIZE];

    if(strcmp(file_type, "tex") == 0){
        char tex_header[BUFSIZE * 2];
        snprintf(tex_header, sizeof(tex_header),
            "\\documentclass[a4paper,11pt]{ltjsarticle}\n"
            "\\usepackage{luatexja}\n"
            "\\usepackage[margin=2.5cm]{geometry}\n"
            "\\usepackage{longtable}\n"
            "\\begin{document}\n"
            "\\begin{center}\n"
            "  {\\LARGE \\textbf{%s}}\\\\[1em]\n"
            "  {\\large 申請ID: %s \\quad 状態: %s}\n"
            "\\end{center}\n"
            "\\vspace{1em}\n"
            "\\begin{longtable}{|p{6cm}|p{10cm}|}\n"
            "\\hline\n"
            "\\textbf{項目} & \\textbf{入力内容} \\\\ \\hline\\hline\n"
            "\\endfirsthead\n", format_name, app_id, status);
        send(threadParam->soc, tex_header, strlen(tex_header), 0);

        while(fgets(fmt_line, sizeof(fmt_line), fp_fmt) != NULL && fgets(data_line, sizeof(data_line), fp_data) != NULL){
            fmt_line[strcspn(fmt_line, "\n")] = '\0';
            data_line[strcspn(data_line, "\n")] = '\0';
            
            char lineBuf[BUFSIZE];
            snprintf(lineBuf, sizeof(lineBuf), "%s & %s \\\\ \\hline\n", fmt_line, data_line);
            send(threadParam->soc, lineBuf, strlen(lineBuf), 0);
        }

        char tex_footer[] = "\\end{longtable}\n\\end{document}\n";
        send(threadParam->soc, tex_footer, strlen(tex_footer), 0);

    }
    else if(strcmp(file_type, "csv") == 0){
        char csv_header[] = "項目名,入力データ\n";
        send(threadParam->soc, csv_header, strlen(csv_header), 0);

        while(fgets(fmt_line, sizeof(fmt_line), fp_fmt) != NULL && fgets(data_line, sizeof(data_line), fp_data) != NULL){
            fmt_line[strcspn(fmt_line, "\n")] = '\0';
            data_line[strcspn(data_line, "\n")] = '\0';
            
            char lineBuf[BUFSIZE];
            snprintf(lineBuf, sizeof(lineBuf), "\"%s\",\"%s\"\n", fmt_line, data_line);
            send(threadParam->soc, lineBuf, strlen(lineBuf), 0);
        }
    }

    fclose(fp_fmt);
    fclose(fp_data);
    
    printf("📡 申請 [%s] を %s 形式で生成・送信しました\n", app_id, file_type);

    return 0;
}
