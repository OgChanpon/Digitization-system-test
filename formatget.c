#include "formatget.h"
#include "nwp.h"

int FormatGet(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf) {
  PGresult *res;
  char sql[SQLSIZE];
  char comm[BUFSIZE];
  int resultRows, n, sendLen;
  char format_id[IDSIZE]; 
  char role[ROLESIZE];
  char format_text[TEXTSIZE];

  sscanf(recvBuf, "%s", comm);

  if(threadParam -> session.is_login != 1){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_1056, ENTER);
    send(threadParam -> soc, sendBuf, strlen(sendBuf), 0);
    printf("%s", sendBuf);
    return -1;
  }

  if(strcmp(threadParam -> session.role, "student") != 0 && strcmp(threadParam -> session.role, "teacher") != 0 && strcmp(threadParam -> session.role, "admin") != 0){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_1055, ENTER);
    printf("%s", sendBuf);
    return -1;
  }

  snprintf(sql, SQLSIZE,
           "SELECT f.format_id, f.format_name, f.data_fields "
           "FROM formats f "
           "JOIN format_permissions p ON f.format_id = p.format_id "
           "WHERE p.role = '%s' ORDER BY f.format_id ASC",
           threadParam -> session.role);
  printf("[%s]\n", sql);
  res = PQexec(threadParam -> con, sql);
  if(PQresultStatus(res) != PGRES_TUPLES_OK){
    printf("%s\n", PQresultErrorMessage(res));
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_3, ENTER);
    printf("%s", sendBuf);
    PQclear(res);
    return -1;
  }
  resultRows = PQntuples(res);

  sendLen = snprintf(sendBuf, BUFSIZE, "%s %d%s", OK_STAT, resultRows, ENTER);
  printf("%s", sendBuf);
  send(threadParam -> soc, sendBuf, sendLen, 0);
  for(int i = 0; i < resultRows; i++){
    strcpy(format_text, PQgetvalue(res, i, 0));
    sendLen = snprintf(sendBuf, BUFSIZE, "%s, %s, %s%s",
                       PQgetvalue(res, i, 0),
                       PQgetvalue(res, i, 1),
                       PQgetvalue(res, i, 2),
                       ENTER);
    printf("%s", sendBuf);
    send(threadParam -> soc, sendBuf, sendLen, 0);
  }

  PQclear(res);
  return 0;
}
