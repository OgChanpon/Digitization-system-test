#include "register.h"
#include "nwp.h"

int Register(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
  PGresult *res;
  char *sqlBegin = "BEGIN", *sqlCommit = "COMMIT", *sqlRollback = "ROLLBACK";
  char sql[SQLSIZE];
  char comm[BUFSIZE];
  int resultRows, n, sendLen;
  char user_name[NAMESIZE], user_id[IDSIZE]; 
  char password[PASSSIZE], role[ROLESIZE];

  sscanf(recvBuf, "%s %s %s %s %[^\n]", comm, user_id, password, role, user_name);

  if(threadParam -> session.is_login != 1){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_1066, ENTER);
    send(threadParam -> soc, sendBuf, strlen(sendBuf), 0);
    printf("%s", sendBuf);
    return -1;
  }

  if(strcmp(threadParam -> session.role, "admin") != 0){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_1062, ENTER);
    printf("%s", sendBuf);
    return -1;
  }

  if(strcmp(role, "student") != 0 && strcmp(role, "teacher") != 0 && strcmp(role, "admin") != 0){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_1062, ENTER);
    printf("%s", sendBuf);
    return -1;
  }

  PQexec(threadParam -> con, sqlBegin);
  snprintf(sql, SQLSIZE,
           "INSERT INTO users (user_id, password, role, name) "
           "VALUES ('%s', '%s', '%s', '%s')",
           user_id, password, role, user_name);
  printf("[%s]\n", sql);
  res = PQexec(threadParam -> con, sql);

  if(PQresultStatus(res) != PGRES_COMMAND_OK){
    printf("%s", PQresultErrorMessage(res));
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_3, ENTER);
    printf("%s", sendBuf);
    PQexec(threadParam -> con, sqlRollback);
    PQclear(res);
    return -1;
  }
  if(atoi(PQcmdTuples(res)) != 1){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_3, ENTER);
    printf("%s", sendBuf);
    PQexec(threadParam -> con, sqlRollback);
    PQclear(res);
    return -1;
  }

  PQexec(threadParam -> con, sqlCommit);

  sendLen = snprintf(sendBuf, BUFSIZE, "%s %s%s", OK_STAT, user_name, ENTER);
  printf("%s", sendBuf);
  send(threadParam -> soc, sendBuf, sendLen, 0);

  PQclear(res);
  return 0;
}
