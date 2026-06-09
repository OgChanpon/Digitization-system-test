#include "nwp.h"

int UserDelete(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
  PGresult *res;
  char sql[SQLSIZE];
  char comm[BUFSIZE];
  int sendLen;
  char target_id[IDSIZE];

  sscanf(recvBuf, "%s %s", comm, target_id);

  if(threadParam -> session.is_login != 1){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_2002, ENTER);
    send(threadParam -> soc, sendBuf, strlen(sendBuf), 0);
    printf("%s", sendBuf);
    return -1;
  }

  if(strcmp(threadParam -> session.role, "admin") != 0){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_2003, ENTER);
    printf("%s", sendBuf);
    return -1;
  }

  snprintf(sql, SQLSIZE,
           "DELETE FROM users "
           "WHERE user_id = '%s'",
           target_id);
  printf("[%s]\n", sql);
  res = PQexec(threadParam -> con, sql);
  if(PQresultStatus(res) != PGRES_COMMAND_OK){
    printf("%s\n", PQresultErrorMessage(res));
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_4004, ENTER);
    printf("%s", sendBuf);
    PQclear(res);
    return -1;
  }

  int deleted_count = atoi(PQcmdTuples(res));
  if(deleted_count == 0){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_4001, ENTER);
    printf("%s", sendBuf);
    send(threadParam -> soc, sendBuf, strlen(sendBuf), 0);
    PQclear(res);
    return -1;
  }
  sendLen = snprintf(sendBuf, BUFSIZE, "%s %d records deleted.%s", OK_STAT, target_id, ENTER);
  printf("%s", sendBuf);
  send(threadParam -> soc, sendBuf, sendLen, 0);

  PQclear(res);
  return 0;
}
