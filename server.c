#include "nwp.h"

void service_start(int __lsoc);

int main(int argc, char *argv[]){
  int listenSoc;
  u_short port;

  if(argc == 2)
    port = atoi(argv[1]);
  else 
    port = PORT;

  if((listenSoc = setup_listen(port)) < 0){
    exit(1);
  }
  service_start(listenSoc);

  return 0;
}

void service_start(int __lsoc){
  char *dbHost = "localhost";
  char *dbPort = "5432";
  char *dbName = "test";
  char *dbLogin = "test";
  char *dbPwd = "test";
  char connInfo[BUFSIZE];
  pthread_t worker;
  ThreadParameter *threadParam;
  struct sockaddr_in client; unsigned int client_len; int s_new; while(1){ client_len = sizeof(client); printf("[MAIN THREAD] Accept Call...\n"); s_new = accept(__lsoc, (struct sockaddr*)&client, &client_len); threadParam = (ThreadParameter*)malloc(sizeof(ThreadParameter));
    threadParam -> c_ipaddr = client.sin_addr;
    threadParam -> c_port = client.sin_port;
    threadParam -> soc = s_new;

    sprintf(connInfo, "host=%s port=%s dbname=%s user=%s password=%s", dbHost, dbPort, dbName, dbLogin, dbPwd);
    threadParam -> con = PQconnectdb(connInfo);

    if(PQstatus(threadParam -> con) == CONNECTION_BAD){
      printf("Connection to database '%s:%s %s' failed.\n", dbHost, dbPort, dbName);
      printf("%s", PQerrorMessage(threadParam -> con));
      threadParam -> con = NULL;
    }
    else{
      printf("Connected to database %s:%s %s\n", dbHost, dbPort, dbName);
    }
    pthread_create(&worker, NULL, (void*)controller, (void*)threadParam);
    printf("[MAIN THREAD] Created thread ID: %ld\n", worker);
    pthread_detach(worker);
  }
}
