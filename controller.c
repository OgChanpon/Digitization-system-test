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
        else if(strcmp(comm, FMTGET) == 0){
            flag = FormatGet(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, APPADD) == 0){
            flag = AppAdd(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, APPLS) == 0){
            flag = AppGet(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, APPDET) == 0){
            flag = AppDetail(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, APPUPD) == 0){
            flag = AppUpdate(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, APPDEL) == 0){
            flag = AppDelete(selfId, threadParam, recvBuf, sendBuf);  
        }
        else if(strcmp(comm, USRREG) == 0){
            flag = UserRegister(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, LOGOUT) == 0){
            flag = Logout(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, FMTADD) == 0){
            flag = FormatAdd(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, FMTUPD) == 0){
            flag = FormatUpdate(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, APPCHK) == 0){
            flag = AppCheck(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, NOTICE) == 0){
            flag = Notice(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, USRDEL) == 0){
            flag = UserDelete(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, USRUPD) == 0){
            flag = UserUpdate(selfId, threadParam, recvBuf, sendBuf);
        }
        else if(strcmp(comm, APPDWN) == 0){
            flag = AppDownload(selfId, threadParam, recvBuf, sendBuf);
        }
        else{
            flag = -1;
            sprintf(sendBuf, "%s %d%s", ER_STAT, E_CODE_1002, ENTER);
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
