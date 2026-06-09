#include "nwp.h"

int Notice(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){ 
  PGresult *res;
  char *sqlBegin = "BEGIN", *sqlCommit = "COMMIT", *sqlRollback = "ROLLBACK"; 
  char sql[SQLSIZE];
  char comm[BUFSIZE];
  int sendLen;
  char format_id[IDSIZE];
  char notice[NOTICESIZE];

  sscanf(recvBuf, "%s %s %s", comm, format_id, notice);

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

  if(strcmp(notice, "realtime") != 0 && strcmp(notice, "daily") != 0 && strcmp(notice, "weekly") != 0){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_3002, ENTER);
    printf("%s", sendBuf);
    return -1;
  }

  PQexec(threadParam -> con, sqlBegin);

  snprintf(sql, SQLSIZE,
           "UPDATE formats SET notify_freq = '%s' "
           "WHERE format_id = '%s'",
           notice, format_id);
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
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_4001, ENTER);
    printf("%s", sendBuf);
    PQexec(threadParam -> con, sqlRollback);
    PQclear(res);
    return -1;
  }

  PQexec(threadParam -> con, sqlCommit);

  sendLen = snprintf(sendBuf, BUFSIZE, "%s %s %s", OK_STAT, format_id, ENTER);
  printf("%s", sendBuf);
  send(threadParam -> soc, sendBuf, sendLen, 0);

  PQclear(res);
  return 0;
}
