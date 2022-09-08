/*indoor_ui.c梯口机ui人物*/
#include <sys/types.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>//linux编程
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include<string.h>
#include <ctype.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<errno.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
//消息最大长度
#define BUF_MAX_LEN 64


//定义消息结构体
#define MSG_DATA_LEN 120
struct message
{
    long msg_type;
    char msg_data[MSG_DATA_LEN];
};
//定义消息类型
enum MSG_TYPE_UI
{
    MASG_TYPE_UI_QUIT =1,
};
enum MSG_TYPE_PHONE
{
    MSG_TYPE_PHONE_QUIT=1,
};

//非阻塞检测键盘输入
   int kbhit (void)//kbhit函数用于非阻塞检测键盘输入，无输入为0，有输入大于0
    {
    struct timeval tv;
    fd_set rdfs;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rdfs);
    FD_SET (STDIN_FILENO, &rdfs);

    select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &rdfs);

    }
static void change_state(int fd)//改变通话状态函数
{
    int qid_phone =0;


    printf("Send quit ,msg ok!\n");

}

//定义有名管道标识
#define FIFO "/home/chaonice/桌面/git/ctw/cplus_2022/进程/indoor.txt"   

int main (void)
{
    int qid_ui =0;
    int ch =0;
    
    struct message msg;

    int fd =-1;
    char flag[BUF_MAX_LEN]={0};//flag用来发送消息
     char buf2[BUF_MAX_LEN]={0};//buf2用来接收消息
    int len=0;

    //步骤1：打开FIFO管道，并设置非阻塞标志

    fd=open(FIFO,O_RDWR|O_NONBLOCK,0);
    if(fd==-1)
    {
        perror("open errror;no reading process\n");
        return -1;
    }
    //步骤2：获取将要发送的消息字符串

    while (1)
    {
            ch=0;
            //步骤3：读取管道中的信息
            memset(buf2,0,sizeof(buf2));
        if(len=read(fd,buf2,sizeof(buf2))>0)
        {
            printf("read %s from FIFO\n",buf2);
            break;

        }
        //printf("no data yet\n");
        sleep(1);
          //获取键盘输入，若为“2”，则通知任务改变状态为接听，若为‘3’改变为挂断
        //printf("please write:");
         
            if(kbhit()>0)
            {
                ch=getchar();
            }
      
        if(ch=='2')
        {
            flag[0]='2';
            if((len=write(fd,flag,sizeof(flag)))==-1)
            {
                printf("the FIFO has not been read yet.Please try later\n");

            }
            else
            {
                printf("write %s to the FIFO\n",flag);

            }

        }
        else if(ch=='3')
        {
            flag[0]='3';
            if((len=write(fd,flag,sizeof(flag)))==-1)
            {
                printf("the FIFO has not been read yet.Please try later\n");

            }
            else
            {
                printf("write %s to the FIFO\n",flag);

            }

        }
        else if(ch=='c')//退出进程
        {
            break;
        }

    }
    
    //步骤4：关闭有名管道
    close(fd);
    return 0;

}
