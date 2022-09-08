/*outside_phone.c梯口机通话任务*/
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
#define PORT 2345     /*定义一个端口号*/
#define REMOTE_IP "127.0.0.1" /*定义一个IP ,服务端的IP*/
//消息最大长度
#define BUF_MAX_LEN 64
//下一步时间间隔
#define TIME_NEXT 50
//定义有名管道标识
#define FIFO "/home/chaonice/桌面/git/ctw/cplus_2022/进程/outside_fifo.txt"//---------->用于梯口机phone和梯口机ui交流
//#define FIFO2 "/home/chaonice/桌面/git/ctw/cplus_2022/进程/zuoye/tonghua.txt"   //----------->用于梯口机phone和室内机phone在接听状态下交流
//定义消息结构体：
#define MSG_DATA_LEN 128
struct message
{
    long msg_type;
    char msg_data[MSG_DATA_LEN];
};

//定义消息类型
enum MSG_TYPE_UI //枚举类型enum
{
    MSG_TYPE_UI_QUIT = 1,
};
enum MSG_TYPE_PHONE
{
    MSG_TYPE_PHONE_QUIT = 1,
};
//定义通话状态
enum TASK_PHONE_STATE
{
    TASK_PHONE_STATE_NONE =0,
    TASK_PHONE_STATE_RING,//响铃状态1
    TASK_PHONE_STATE_TALK,//接听状态2
    TASK_PHONE_STATE_HANGUP,//挂断状态3
};

int phone_state = TASK_PHONE_STATE_NONE;//-------->全局变量phone_state电话状态

//设置通话状态
void set_state(int state)
{
    phone_state =state;
}
//获取通话状态
int get_state(void)
{
    return phone_state;
}

int main(int argc,char **argv)
{

    int qid_ui =0,qid_phone=0;
  
    int time =0;
    struct message msg;


    int fd =-1;//有名管道（文件描述符）
    char buf[BUF_MAX_LEN]={0};
    int len =0;
    //步骤1：创建有名管道
    if(access(FIFO,F_OK)!=0)//判断文件是否存在，并判断文件是否可写
    {
        if(mkfifo(FIFO,0666)<0)//有名管道（创建实体文件）
        {
            perror("cannot create fifo!");
            
            exit(1);

        }
    }
    
    //步骤2：打开有名管道，并设置非阻塞标志
    if((fd=open(FIFO,O_RDWR|O_NONBLOCK,0))<0)//成功则返回文件描述符，否则返回-1
    {
        perror("open fifo!");
        exit(1);
    }

    while(1)
    {   //步骤3：读取管道内消息
        memset(buf,0,sizeof(buf));
        if(len=read(fd,buf,sizeof(buf))>0)
        {
            printf("read %s from FIFO\n",buf);    

        }
       
        sleep(1);
        //步骤4：模拟与其他用户处理通信协议，每隔5秒进入下一状态
         time++;
         if(time>=5)
             {
                time = 0;
                if(get_state()==TASK_PHONE_STATE_RING)//响铃状态1-->接听状态2
                {
                    set_state(TASK_PHONE_STATE_TALK);

                }
                else if(get_state()==TASK_PHONE_STATE_TALK)//接听状态2-->挂断状态3
                {
                    set_state(TASK_PHONE_STATE_HANGUP);

                }
                else//挂断状态3-->响铃状态1
                {
                    set_state(TASK_PHONE_STATE_RING);

                }
                printf("Current state is %d!\n",get_state());
            }//本程序为模拟程序，因此默认状态下循环3次主动退出
        //步骤5：若当前通话状态为挂断，则退出任务，并发送消息给ui---->能正确判断通信协议的不同状态，并截取挂断状态做出消息通知。
        ///////////////////////////////////////////////////////////////////////////

            if(buf[0]=='2')//---------------------->状态分析
            {
                set_state(TASK_PHONE_STATE_TALK);//设置接听状态

            }
            else if (buf[0]=='3')
            {
                set_state(TASK_PHONE_STATE_HANGUP);//设置挂断
            }
        //----------------------->挂断状态处理
              if(get_state()== TASK_PHONE_STATE_HANGUP)//当状态为挂断时
                 {
            /*添加消息到有名管道*/
                printf("Send quit msg!\n");
                memset(buf,0,sizeof(buf));
                    buf [0]= 'c';
                
                    if((len=write(fd,buf,sizeof(buf)))==-1)
                        {
                            perror("the FIFO has not been read yet.Please try later\n");

                        }
                        else
                        {
                            printf("write %s to the FIFO\n",buf);

                        }
                  break;
                    }
             usleep(100*1000);//功能把进程挂起一段时间， 单位是微秒（百万分之一秒）
             /////////////////////////////////////////////////////////////////////////
             //------------------------->接听状态处理
                if(get_state()==TASK_PHONE_STATE_TALK)//当状态为接听时-->思路：与室内机phone建立有名管道，互相发送文字信息--->client
                {
                        int sockfd;
                        struct sockaddr_in addr;/*定义IPv4套接口地址数据结构addr  */
                    
                        if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)/*建立一个socket*/
                        {
                            perror("socket created error!");
                            exit(1);
                        }
                        else/*创建socket成功*/
                        {
                            printf("socket created successfully!\n");
                            printf("socket id:%d\n",sockfd);
                        }
                        //初始化服务端socket参数
                        bzero(&addr,sizeof(struct sockaddr_in));/*清空表示地址的结构体变量*/
                        addr.sin_family=AF_INET;
                        addr.sin_port=htons(PORT);
                        addr.sin_addr.s_addr=inet_addr(REMOTE_IP);

                        if(connect(sockfd,(struct sockaddr*)(&addr),sizeof(struct sockaddr))<0)/*调用connect连接到远程服务器*/
                        {
                            perror("connect error!");
                            exit(1);
                        }
                        else{
                            printf("connect succesfully!\n");
                        }
                    
                    //接收文件名称/////////////////////////
                    
                    char buf3[1024]={0};//定义接收数据内存
                    

                        while(1)
                        {
                            memset(buf3,0,sizeof(buf3));   //初始化buf
                            printf("said something to server:");
                            scanf("%s",buf3);
                            write(sockfd,buf3,sizeof(buf3));//向服务端发送数据
                            if(strncmp(buf3,"bye",3) == 0)
                                        {
                                        printf("server quit\n");                    
                                        break;
                                        }
                            memset(buf3,0,sizeof(buf3));   //清空化buf
                            read(sockfd,buf3,sizeof(buf3));//接收消息
                            printf("server said:%s\n",buf3);
                            if(strncmp(buf3,"bye",3) == 0)
                                        {
                                        printf("server quit\n");                    
                                        break;
                                        }
                        }
                        close(sockfd);//结束通信，关闭socket套接字
                        set_state(TASK_PHONE_STATE_HANGUP);//通话结束，设置phone状态为挂断

                }
            

    }
    //步骤4：关闭有名管道
    close(fd);
    return 0;

}
