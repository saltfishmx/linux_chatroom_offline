#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include<fcntl.h>

#define FTOK_FILE "/etc/profile"
#define test_FILE "/etc/passwd"
#define STR_LEN 32
#define SHM_SIZE 256

typedef struct _tag_shm
{
    char buf[SHM_SIZE];
    unsigned short num;
}shm_t;

char name[30];
char buf1[50];
shm_t * pshm = NULL;
shm_t * pshmname = NULL;
sem_t * sem;//sem是所有客户端写入信息到服务器公共信号量
sem_t * sem1;//sem1是客户端写入名字到服务器公共信号量
sem_t * sem_inner;

char buf[STR_LEN] = {0};
int i = 0; 
int username=0;

void wait(void *p)//读取数据信号，发送内部信号提示每个客户端线程读取
{
	while(1){
        	sem_wait(sem);
       	        for(int j=0;j<username;j++)
			sem_post(sem_inner);
        }        
}
void send_data(void *p)//读取内部信号并用专属信号量提醒客户端接收
{
        strcpy(pshm->buf,buf1);
        printf("buf1:%s\n, pshm->buf:%s\n",buf1,pshm->buf);
        sem_t *semspecial=(sem_t *)p;
       
        while(1){

        sem_wait(sem_inner);
        sleep(1);		
        printf("进入读取模式！\n");
        printf("客户端来信息: %s", pshm->buf);
        
        if(!strncmp(pshm->buf,"end:",4)){
        	username--;
                sem_post(semspecial);
        }
        
        sem_post(semspecial);
        printf("离开读取模式！\n");
	}
}
void create_sem(void *p)//客户端线程，根据名字创建专属信号量,为他创建独有的senddata函数
{
	char *name1=(char *)p;
        username++;
        //strcpy(buf1,name1);
        //strcat(buf1,"进入了聊天室");
       // strcpy(pshm->buf,buf1);
      //  printf("pshm->buf:%s\n",pshm->buf);
        sem_t *semspecial;
        pthread_t se;
        semspecial = sem_open(name1, O_CREAT | O_RDWR, 0666, 0);
        pthread_create(&se,NULL,(void *)&send_data,semspecial);	

}



int main(void)
{

    pthread_t tid1;
    pthread_t tid2;
   

    sem1 = sem_open("name", O_CREAT | O_RDWR, 0666, 0);

    sem = sem_open("word", O_CREAT | O_RDWR, 0666, 0);

    sem_inner = sem_open("inner", O_CREAT | O_RDWR, 0666, 0);
    int  shmid;
    int  shmidname;
  
    void *pshm_addr = NULL;
    void *pshm_name = NULL;

    char *alt=(char *)malloc(sizeof(char)*30);
    alt=name;

 
/*获取共享内存标识符*/
    shmid = shmget(ftok(FTOK_FILE,111), sizeof(shm_t), IPC_CREAT|0666);
    if(shmid == -1)
    {
        printf("%s: shmid = %d!\n", __func__, shmid);
        return -1;
    }

/*当前进程连接该共享内存段*/
    pshm_addr = shmat(shmid, 0, 0);
    if(pshm_addr == (void *)-1)
    {
        printf("%s : pshm_addr = (void*)-1!\n",__func__);
        return -1;
    }

    pshm = pshm_addr;
    //printf("read process :  shmid is %d!\n",shmid);
    
    shmidname=shmget(ftok(test_FILE,121),sizeof(shm_t),IPC_CREAT|0660);//建立姓名段共享内存
    if(shmidname == -1)
    {
        printf("%s: shmidname = %d!\n", __func__, shmidname);
        return -1;
    }
    
    pshm_name = shmat(shmidname, 0, 0);
    if(pshm_name == (void *)-1)
    {
        printf("%s : pshm_name = (void*)-1!\n",__func__);
        return -1;
    }
    pshmname = pshm_name;
    //printf("read process :  shmidname is %d!\n",shmidname);    
   




    pthread_create(&tid1,NULL,(void *)&wait,NULL);	
    while(1){
        sem_wait(sem1);
	strcpy(name,pshmname->buf);
        pthread_create(&tid2,NULL,(void *)&create_sem,alt);    


    }


    pthread_join(tid2,NULL);
    sem_destroy(sem);
    sem_destroy(sem1);
/*进程和共享内存脱离*/
    if(shmdt(pshm_addr) == -1)
    {
        printf("%s : shmdt is failed!\n",__func__);
        return -1;
    }
    if(shmdt(pshm_name) == -1)
    {
        printf("%s : shmdt is failed!\n",__func__);
        return -1;
    }
    printf(" Good Bye! \n");
    return 0;

}
