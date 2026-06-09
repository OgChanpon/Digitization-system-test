#include "nwp.h"

int AppUpdate(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){ 
  PGresult *res;
  char *sqlBegin = "BEGIN", *sqlCommit = "COMMIT", *sqlRollback = "ROLLBACK"; 
  char sql[SQLSIZE];
  char comm[BUFSIZE];
  int sendLen;
  char app_id[IDSIZE];
  char format_text[TEXTSIZE]; 

  sscanf(recvBuf, "%s %s %[^\n]", comm, app_id, format_text);

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
           "UPDATE apps SET submitted_data = '%s' "
           "WHERE app_id = '%s' AND user_id = '%s'",
           format_text, app_id, threadParam -> session.user_id);
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

  sendLen = snprintf(sendBuf, BUFSIZE, "%s %s%s", OK_STAT, app_id, ENTER);
  printf("%s", sendBuf);
  send(threadParam -> soc, sendBuf, sendLen, 0);

  PQclear(res);
  return 0;
}
