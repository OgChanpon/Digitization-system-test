#include "nwp.h"

void *controller(void* __arg)
{
    ThreadParameter* threadParam = (ThreadParameter*)__arg;
    char recvBuf[BUFSIZE], sendBuf[BUFSIZE];
    int recvLen, sendLen;
    pthread_t selfId;
    char comm[BUFSIZE];
    int flag;

    selfId = pthread_self();
    printf("[C_THREAD %ld] CONTROLLER START (%d)\n", selfId, threadParam->soc);

    while (1) {
        recvLen = receive_message(threadParam->soc, recvBuf, BUFSIZE);
        if (recvLen < 1)
            break;
        recvBuf[recvLen - 1] = '\0';
        printf("[C_THREAD %ld] RECV=> %s\n", selfId, recvBuf);

        sscanf(recvBuf, "%s", comm);

        if(strcmp(comm, LOGIN) == 0){
            flag = Login(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, FMTGT) == 0){
            flag = FormatGet(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, APPAD) == 0){
            flag = AppAdd(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, APLGT) == 0){
            flag = AppGet(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, APDET) == 0){
            flag = AppDetail(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, APUPD) == 0){
            flag = AppUpdate(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, APDLE) == 0){
            flag = AppDelete(selfId, threadParam, recvBuf, sendBuf);  
        }
        else if(strcmp(comm, REGST) == 0){
            flag = Register(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, LOGOT) == 0){
            flag = Logout(selfId, threadParam, recvBuf, sendBuf);
        }
        else{
            flag = -1;
            sprintf(sendBuf, "%s %d%s", ER_STAT, E_CODE_2, ENTER);
        }

        if (flag < 0) {
            sendLen = strlen(sendBuf);
            send(threadParam->soc, sendBuf, sendLen, 0);
            printf("[C_THREAD %ld] SEND=> %s", selfId, sendBuf);
        }

        sendLen = sprintf(sendBuf, "%s", DATA_END);
        send(threadParam->soc, sendBuf, sendLen, 0);
        printf("[C_THREAD %ld] SEND=> %s\n", selfId, sendBuf);
    }

    printf("[C_THREAD %ld] CONTROLLER END (%d)\n\n", selfId, threadParam->soc);

    PQfinish(threadParam->con);
    close(threadParam->soc);
    free(threadParam);
}
