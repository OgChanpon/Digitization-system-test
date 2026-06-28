#include "server.h"

int Logout(pthread_t selfId, ThreadParameter *threadParam, char *recvBuf, char *sendBuf){
  char comm[BUFSIZE];
  int sendLen;

  sscanf(recvBuf, "%s", comm);

  if(threadParam -> session.is_login != 1){
    snprintf(sendBuf, BUFSIZE, "%s %d%s", ER_STAT, E_CODE_2002, ENTER);
    send(threadParam -> soc, sendBuf, strlen(sendBuf), 0);
    printf("%s", sendBuf);
    return -1;
  }

  threadParam -> session.is_login = 0;
  strcpy(threadParam -> session.user_id, "");
  strcpy(threadParam -> session.role, "");

  sendLen = snprintf(sendBuf, BUFSIZE, "%s%s", OK_STAT, ENTER);
  printf("%s", sendBuf);
  send(threadParam -> soc, sendBuf, sendLen, 0);

  return 0;
}
