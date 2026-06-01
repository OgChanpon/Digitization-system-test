#include "login.h"
#include "nwp.h"

int Login(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
  PGresult *res;
  char sql[SQLSIZE];
  char comm[BUFSIZE];
  int resultRows, n, sendLen;
  char user_id[IDSIZE], password[PASSSIZE];
  char notice_text[TEXTSIZE];

  n = sscanf(recvBuf, "%s %s %s", comm, user_id, password);

  if(n != 3){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_1, ENTER);
    printf("%s", sendBuf);
    return -1;
  }

  //snprintf(sql, SQLSIZE, "SET search_path to test");
  //PQexec(threadParam -> con, sql);

  snprintf(sql, SQLSIZE,
           "SELECT role FROM users WHERE user_id = '%s' AND password = '%s'",
           user_id, password);
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
  if(resultRows != 1){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_3, ENTER);
    printf("%s", sendBuf);
    return -1;
  }

  threadParam -> session.is_login = 1;
  strcpy(threadParam -> session.user_id, user_id);
  strcpy(threadParam -> session.role, PQgetvalue(res, 0, 0));

  /*snprintf(sql, SQLSIZE, "SELECT notice_text FROM notice WHERE user_id = '%s' ORDER BY notice_date ASC", id);
  printf("[%s]\n", sql);
  PQclear(res);
  res = PQexec(threadParam -> con, sql);
  if(PQresultStatus(res) != PGRES_TUPLES_OK){
    printf("%s\n", PQresultErrorMessage(res));
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_3, ENTER);
    printf("%s", sendBuf);
    PQclear(res);
    return -1;
  }
  resultRows = PQntuples(res);*/

  sendLen = snprintf(sendBuf, BUFSIZE, "%s %s %d%s", OK_STAT, user_id, resultRows, ENTER);
  printf("%s", sendBuf);
  send(threadParam -> soc, sendBuf, sendLen, 0);
  for(int i = 0; i < resultRows; i++){
    strcpy(notice_text, PQgetvalue(res, i, 0));
    sendLen = snprintf(sendBuf, BUFSIZE, "%s%s", notice_text, ENTER);
    printf("%s", sendBuf);
    send(threadParam -> soc, sendBuf, sendLen, 0);
  }

  PQclear(res);
  return 0;
}
