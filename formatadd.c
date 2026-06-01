#include "formatadd.h"
#include "nwp.h"

int FormatAdd(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
  PGresult *res;
  char *sqlBegin = "BEGIN", *sqlCommit = "COMMIT", *sqlRollback = "ROLLBACK";
  char sql[SQLSIZE];
  char comm[BUFSIZE];
  int resultRows, n, sendLen;
  char format_id[IDSIZE], format_name[NAMESIZE]; 
  char format_text[TEXTSIZE], role[ROLESIZE];

  sscanf(recvBuf, "%s %s %s %[^\n]", comm, format_id, format_name, role, format_text);

  if(threadParam -> session.is_login != 1){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_1066, ENTER);
    send(threadParam -> soc, sendBuf, strlen(sendBuf), 0);
    printf("%s", sendBuf);
    return -1;
  }

  if(strcmp(threadParam -> session.role, "student") != 0 && strcmp(threadParam -> session.role, "teacher") != 0 && strcmp(threadParam -> session.role, "admin") != 0){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_1062, ENTER);
    printf("%s", sendBuf);
    return -1;
  }

  PQexec(threadParam -> con, sqlBegin);

  snprintf(sql, SQLSIZE,
           "INSERT INTO formats (format_id, format_name, data_fields) "
           "VALUES ('%s', '%s', '%s'); "
           "INSERT INTO format_permissions (format_id, role) "
           "VALUES ('%s' '%s')",
           format_id, format_name, format_text, format_id, role);
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

  sendLen = snprintf(sendBuf, BUFSIZE, "%s %s%s", OK_STAT, format_id, ENTER);
  printf("%s", sendBuf);
  send(threadParam -> soc, sendBuf, sendLen, 0);

  PQclear(res);
  return 0;
}
