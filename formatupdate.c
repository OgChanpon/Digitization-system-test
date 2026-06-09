#include "nwp.h"

int FormatUpdate(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){ 
  PGresult *res;
  char *sqlBegin = "BEGIN", *sqlCommit = "COMMIT", *sqlRollback = "ROLLBACK"; 
  char sql[SQLSIZE];
  char comm[BUFSIZE];
  int resultRows, n, sendLen;
  char format_id[IDSIZE];
  char format_text[TEXTSIZE]; 

  sscanf(recvBuf, "%s %s %[^\n]", comm, format_id, format_text);

  if(threadParam -> session.is_login != 1){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_2002, ENTER);
    send(threadParam -> soc, sendBuf, strlen(sendBuf), 0);
    printf("%s", sendBuf);
    return -1;
  }

  if(strcmp(threadParam -> session.role, "student") != 0 && strcmp(threadParam -> session.role, "teacher") != 0 && strcmp(threadParam -> session.role, "admin") != 0){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_2003, ENTER);
    printf("%s", sendBuf);
    return -1;
  }

  PQexec(threadParam -> con, sqlBegin);

  snprintf(sql, SQLSIZE,
           "UPDATE formats SET data_fields = '%s' "
           "WHERE format_id = '%s'",
           format_text, format_id);
  printf("[%s]\n", sql);
  res = PQexec(threadParam -> con, sql);
  
  if(PQresultStatus(res) != PGRES_COMMAND_OK){
    printf("%s\n", PQresultErrorMessage(res));
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_4004, ENTER);
    printf("%s", sendBuf);
    PQexec(threadParam -> con, sqlRollback);
    PQclear(res);
    return -1;
  }

  if(atoi(PQcmdTuples(res)) != 1){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_4001, ENTER); // データ不在エラー
    printf("%s", sendBuf);
    PQexec(threadParam -> con, sqlRollback);
    PQclear(res);
    return -1;
  }

  PQexec(threadParam -> con, sqlCommit);
  PQclear(res);

  char format_name[50] = "Unknown_Format";
  
  snprintf(sql, SQLSIZE, "SELECT format_name FROM formats WHERE format_id = '%s'", format_id);
  res = PQexec(threadParam -> con, sql);
  if(PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) == 1){
    strcpy(format_name, PQgetvalue(res, 0, 0));
  }
  PQclear(res);

  char filename[FILENAMESIZE];
  snprintf(filename, sizeof(filename), "%s_template.tex", format_id);
  
  FILE *fp = fopen(filename, "w");
  if(fp != NULL){
    fprintf(fp, "%% --- 更新された %s のテンプレート ---\n", format_id);
    fprintf(fp, "\\documentclass[a4paper,11pt]{ltjsarticle}\n");
    fprintf(fp, "\\begin{document}\n");
    fprintf(fp, "\\begin{center}\n");
    fprintf(fp, "  {\\LARGE %s}\n", format_name);
    fprintf(fp, "\\end{center}\n");
    fprintf(fp, "\\vspace{2em}\n");

    char temp_text[TEXTSIZE];
    strcpy(temp_text, format_text);
    
    char *token = strtok(temp_text, ",");
    int count = 1;
    while(token != NULL){
      fprintf(fp, "\\noindent\n");
      fprintf(fp, "{\\bf %s:} #DATA_%d# \\\\\n", token, count);
      fprintf(fp, "\\vspace{1em}\n");
      token = strtok(NULL, ",");
      count++;
    }

    fprintf(fp, "\\end{document}\n");
    fclose(fp);
    printf("[TEMPLATE UPDATED] %s\n", filename);
  }

  sendLen = snprintf(sendBuf, BUFSIZE, "%s %s%s", OK_STAT, format_id, ENTER);
  send(threadParam -> soc, sendBuf, sendLen, 0);

  PQclear(res);
  return 0;
}
