#include "nwp.h"

int setup_listen(u_short __port){
  struct sockaddr_in server;
  int soc;
  int opt = 1;
  
  if((soc = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("socket");
    return -1;
  }

  if (setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt failed");
    close(soc);
    exit(EXIT_FAILURE);
  }

  memset(&server, 0 , sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(__port);

  if(bind(soc, (struct sockaddr *)&server, sizeof(server)) < 0){
    perror("bind");
    return -1;
  }

  listen(soc, 5);

  printf("listen...\n");

  return soc;
}
