#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

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
shm_t * pshm = NULL;
shm_t * pshmname = NULL;
sem_t * sem;//sem是所有客户端写入信息到服务器公共信号量
sem_t * sem1;//sem1是客户端写入名字到服务器公共信号量
sem_t * semspecial;

char buf[STR_LEN] = {0};
int i = 0; 

void read_data(void)
{
	while(1){
	
        sem_wait(semspecial);	
        //printf("进入读取模式！\n");

        printf("%s", pshm->buf);
        //printf("离开读取模式！\n\n");
	}
}

void writename(void *p)
{

      
        char *name1=(char *)p;
        printf("%s",name1);
        sem_post(sem1);
	
      		
        //printf("进入写名模式！\n");
        strcpy(pshmname->buf,name1);
        //printf("%s\n", pshmname->buf);
        //printf("离开写名模式！\n\n");
	
}
void write_data(void *p)
{
    char *name1=(char *)p;
    char leave[40]="end:";
    char comein[40]="";
    strcpy(comein,name1);;
    strcat(comein," says:");
    strcpy(leave,name1);
    strcat(leave,"离开了\n");
    strcat(name1," says:");
    
    for(; ;i++)
    {
       
        char *word=malloc(sizeof(char)*80);
        sleep(1);
        //printf("\n你的名字为%s\n",name1);       
        strcpy(word,name1);
        //printf("进入输入模式！\n");    
      
        printf("现在可以在下方输入 >\n");
        fgets(buf, STR_LEN, stdin);
       
        strcat(word,buf);
        strcpy(pshm->buf, word); 
        //printf("pshm->buf:%s",pshm->buf);  
             
        //printf("离开输入模式！\n\n");
        sleep(1);

    
       

        if(!strncmp(buf , "end", 3)){
            strcpy(pshm->buf,leave);
            sem_post(sem);   
            exit(0);
        }
        if(!strcmp(buf , comein)){
            strcpy(comein,name1);
            strcat(comein,"进入了聊天室");
            strcpy(pshm->buf,comein);
            sem_post(sem);   
           
        }
        sem_post(sem);
        
    }

}
int main(void)
{
    pthread_t read;
    pthread_t write;
    pthread_t kkk;
    printf("请输入您的名字：");
    scanf("%s",name); 
    char *alt=(char *)malloc(sizeof(char)*30);
    alt=name;
    printf("欢迎来到聊天室，本聊天室输入end可退出:)\n");
    sem1 = sem_open("name", O_CREAT | O_RDWR, 0666, 0);
    sem = sem_open("word", O_CREAT | O_RDWR, 0666, 0);
    semspecial = sem_open(name, O_CREAT | O_RDWR, 0666, 0);
    int  shmid;
    int  shmidname;
  
    void *pshm_addr = NULL;
    void *pshm_name = NULL;
 

 
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
    
    
    pthread_create(&kkk,NULL,(void *)&writename,alt);
    pthread_join(kkk,NULL);  
    pthread_create(&read,NULL,(void *)&read_data,NULL);
 
    pthread_create(&write,NULL,(void *)&write_data,alt);

    pthread_join(read,NULL);
    pthread_join(write,NULL);
    sem_destroy(semspecial);
  




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
