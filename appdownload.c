#include "nwp.h"

int AppDownload(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
  PGresult *res;
  char sql[SQLSIZE];
  char comm[BUFSIZE];
  char app_id[IDSIZE];

  sscanf(recvBuf, "%s %s", comm, app_id);

  if(threadParam -> session.is_login != 1){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_2002, ENTER);
    return -1;
  }

  if(strcmp(threadParam -> session.role, "teacher") != 0 && strcmp(threadParam -> session.role, "admin") != 0){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_2003, ENTER);
    return -1;
  }

  snprintf(sql, SQLSIZE,
           "SELECT format_id, submitted_data FROM apps WHERE app_id = '%s'",
           app_id);
  res = PQexec(threadParam -> con, sql);
  
  if(PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) != 1){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_4001, ENTER);
    PQclear(res);
    return -1;
  }

  char format_id[IDSIZE];
  char app_data[TEXTSIZE];
  strcpy(format_id, PQgetvalue(res, 0, 0));
  strcpy(app_data, PQgetvalue(res, 0, 1));
  PQclear(res);

  char *fields[50];
  int field_count = 0;
  char *token = strtok(app_data, ",");
  while(token != NULL && field_count < 50){
    fields[field_count] = token;
    field_count++;
    token = strtok(NULL, ",");
  }

  char filename[FILENAMESIZE];
  snprintf(filename, sizeof(filename), "%s_template.tex", format_id);
  
  FILE *fp = fopen(filename, "r");
  if(fp == NULL){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_1003, ENTER);
    return -1;
  }

  snprintf(sendBuf, BUFSIZE, "%s%s", OK_STAT, ENTER);
  send(threadParam -> soc, sendBuf, strlen(sendBuf), 0);

  char line[TEXTSIZE];
  char temp_line[TEXTSIZE];
  
  while(fgets(line, sizeof(line), fp) != NULL){
    for(int i = 0; i < field_count; i++){
      char placeholder[16];
      snprintf(placeholder, sizeof(placeholder), "#DATA_%d#", i + 1);
      
      char *pos = strstr(line, placeholder);
      if(pos != NULL){
        *pos = '\0';
        snprintf(temp_line, sizeof(temp_line), "%s%s%s", line, fields[i], pos + strlen(placeholder));
        strcpy(line, temp_line);
      }
    }
    send(threadParam -> soc, line, strlen(line), 0);
  }

  fclose(fp);
  return 0;
}
