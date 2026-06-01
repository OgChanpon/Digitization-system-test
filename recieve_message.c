#include "nwp.h"

int receive_message(int __s, char *__buf, int __maxlen){
  int recv_total = 0;
  int delim_count = 0;
  int recv_len;

  while((recv_total < __maxlen) && (delim_count < 1)){
    recv_len = recv(__s, __buf+recv_total, 1, 0);
    if(recv_len < 1){
      return recv_len;
    }
    if( *(__buf+recv_total) == DELIMITER )
      delim_count++;

    recv_total += recv_len;
  }

  __buf[recv_total] = '\0';
  return recv_total;
}
