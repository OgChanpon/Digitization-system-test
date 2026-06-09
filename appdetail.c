#include "nwp.h"

int AppDetail(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
  PGresult *res;
  char sql[SQLSIZE];
  char comm[BUFSIZE];
  int resultRows, n, sendLen;
  char app_id[IDSIZE];

  sscanf(recvBuf, "%s %s", comm, app_id);

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

  snprintf(sql, SQLSIZE,
           "SELECT submitted_data FROM apps "
           "WHERE app_id = '%s' AND user_id = '%s'",
           app_id, threadParam -> session.user_id);
  printf("[%s]\n", sql);
  res = PQexec(threadParam -> con, sql);
  if(PQresultStatus(res) != PGRES_TUPLES_OK){
    printf("%s\n", PQresultErrorMessage(res));
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_4004, ENTER);
    printf("%s", sendBuf);
    PQclear(res);
    return -1;
  }
  resultRows = PQntuples(res);

  if(resultRows == 0){
  snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_4001, ENTER); // データ不在（または権限なし）
  send(threadParam -> soc, sendBuf, strlen(sendBuf), 0);
  PQclear(res);
  return -1;
}

  sendLen = snprintf(sendBuf, BUFSIZE, "%s %d%s", OK_STAT, resultRows, ENTER);
  printf("%s", sendBuf);
  send(threadParam -> soc, sendBuf, sendLen, 0);
  for(int i = 0; i < resultRows; i++){
    sendLen = snprintf(sendBuf, BUFSIZE, "%s%s",
                       PQgetvalue(res, i, 0),
                       ENTER);
    printf("%s", sendBuf);
    send(threadParam -> soc, sendBuf, sendLen, 0);
  }

  PQclear(res);
  return 0;
}
