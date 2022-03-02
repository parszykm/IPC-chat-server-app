#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<stdio.h>
#include<sys/wait.h>
#include<unistd.h>
#define users_num 9
#define groups_num 3
struct msgbuf
{
    long type;
    char text[200];
};

struct group{

    char group_name[20];
    char participants[users_num][20];
};

struct user
{
    key_t key;
    char name[20];
    char password[20];
    int logged;
    int msqid;
};
int find_msqid(struct user users[],char rec[]){
    for(int h=0;h<users_num;h++)
    { 
        if(strcmp(users[h].name,rec)==0) 
            {
            return users[h].msqid;
            }
    }
    return -1;

}
int find_if_group(struct group groups[],char groupname[],char name[] ){
    for(int i=0;i<groups_num;i++){
        if(strcmp(groupname,groups[i].group_name)==0)
        {
        for(int j=0;j<users_num;j++){
            
            if(strcmp(groups[i].participants[j],name)==0)
            {
                return 1;
            }
        }
        }
    }
    return 0;
}
int main()
{

    char logged_string[100],tmp_string[200],tmp_string2[200];
    char spr[50];
    char delimeter[]=":";
    int tmp_count=0;
    struct user users[users_num];
    struct group groups[groups_num]; 
    FILE *fp=fopen("passwords.txt", "r");
    char *ptr;
    for(int i=0;i<groups_num;i++){
        for(int j=0;j<users_num;j++)
        {
            strcpy(groups[i].participants[j],"");
        }
    }
    for(int i=0;i<users_num;i++)
    {
        fgets(spr,sizeof(spr),fp);
        ptr=strtok(spr,delimeter);
        strcpy(users[i].name,ptr);
        ptr=strtok(NULL,delimeter);
        strcpy(users[i].password,ptr);
        ptr=strtok(NULL,delimeter);
        users[i].key=atoi(ptr);
        users[i].logged=0;
    }
    fclose(fp);
    FILE *fp2=fopen("groups.txt", "r");
    for(int i=0;i<groups_num;i++)
    {
        fgets(spr,sizeof(spr),fp2);
        ptr=strtok(spr,delimeter);
        strcpy(groups[i].group_name,ptr);
        ptr=strtok(NULL,delimeter);
        while(ptr != NULL){
            ptr[strcspn(ptr, "\n")] = 0;
            strcpy(groups[i].participants[tmp_count],ptr);
            tmp_count+=1;
            ptr=strtok(NULL,delimeter);


        }
        tmp_count=0;
    }
    fclose(fp2);
    key_t key_log = 1111;
    for(int i=0;i<users_num;i++)
    {
        if((users[i].msqid= msgget(users[i].key, 0666|IPC_CREAT)) == -1)
        {
            perror(" msgget error\n");
            exit(1);
        }
    }
    int msqid_log;
    if((msqid_log = msgget(key_log, 0666|IPC_CREAT)) == -1)
        {
            perror(" msgget error\n");
            exit(1);
        }
    int msqid_srv;
    key_t key_srv=1001;
    if((msqid_srv = msgget(key_srv, 0666|IPC_CREAT)) == -1)
        {
            perror(" msgget server error\n");
            exit(1);
        }

    printf("Server ready...\n");


    struct msgbuf odb;
    struct msgbuf log;
    char rec[20];
    int id,log_succes=0, msqid_tmp;
    //LOGIN

    if((id=fork())==0){
        while(1){
            msgrcv(msqid_log,&log,sizeof(log.text)+1,0,0);
            log_succes=0;
            switch (log.type)
            {
            case 1:
                ptr=strtok(log.text,"-");
                log.type=2;
                for(int j=0;j<users_num;j++){
                    if(strcmp(ptr,users[j].name)==0)
                    {   
                        ptr=strtok(NULL,"-");
                        if(strcmp(ptr,users[j].password)==0){
                            sprintf(log.text,"%d",users[j].key);
                            msgsnd(msqid_log,&log,sizeof(log.text)+1,0);
                            log_succes=1;
                        }
                    }
                    }
                if(log_succes==0){
                    strcpy(log.text,"0");
                    msgsnd(msqid_log,&log,sizeof(log.text)+1,0);
                }
                
                break;
            case 3:
                printf("%s logged in...\n",log.text);
                for(int h=0;h<users_num;h++)
                { 
                    log.text[strcspn(log.text,"\n")]=0;
                    if(strcmp(users[h].name,log.text)==0) 
                        {
                            users[h].logged=1;
                            break;
                        }
                }  
                break;
            
            default:
                break;
            }
                }      

            
        }
    while(1){
        msgrcv(msqid_srv,&odb,sizeof(odb.text)+1,0,0);
        printf("Operation %ld request...\n",odb.type);
        switch (odb.type)
        {
            
        case 1:
            printf("To Client: %s", odb.text);
            strcpy(rec,odb.text);
            for(int h=0;h<users_num;h++)
            { 
                rec[strcspn(rec, "\n")] = 0;
                if(strcmp(users[h].name,rec)==0) 
                    {
                    msgrcv(msqid_srv,&odb, sizeof(odb.text)+1,2,0);
                    odb.type=1;
                    msgsnd(users[h].msqid,&odb,sizeof(odb.text)+1,0);
                    break;
                    }
            }
            break;
        case 3:
            for(int j=0;j<users_num;j++){
                if(users[j].logged==1) {
                    strcat(logged_string,users[j].name);
                    strcat(logged_string,"\n");   
                }  
            }
            msqid_tmp=find_msqid(users,odb.text);
            strcpy(odb.text,logged_string);
            odb.type=2;
            msgsnd(msqid_tmp,&odb,sizeof(odb.text)+1,0);
            strcpy(logged_string,"");
            break;
        case 4:
            printf("%s logged in...\n",odb.text);
            for(int h=0;h<users_num;h++)
            { 
                odb.text[strcspn(odb.text,"\n")]=0;
                if(strcmp(users[h].name,odb.text)==0) 
                    {
                        users[h].logged=1;
                        break;
                    }
            }  
            break;
        case 5:
            printf("%s logged out...\n",odb.text);
            for(int j=0;j<users_num;j++){
                if(strcmp(odb.text,users[j].name)==0)
                {
                    users[j].logged=0;
                }
            }
            break;
        case 6:
            msqid_tmp=find_msqid(users,odb.text);
            strcpy(odb.text,"Groups\n");
            for(int j=0;j<groups_num;j++){
                strcat(odb.text,groups[j].group_name);
                strcat(odb.text,"\n");
            }
            odb.type=3;
            msgsnd(msqid_tmp,&odb,sizeof(odb.text)+1,0);
            break;
        case 7:
            ptr=strtok(odb.text,"-");
            msqid_tmp=find_msqid(users,ptr);
            ptr=strtok(NULL,"-");
            
            for(int i=0;i<groups_num;i++){
                ptr[strcspn(ptr, "\n")] = 0;
                if(strcmp(ptr,groups[i].group_name)==0){
                    
                    for(int j=0;j<users_num;j++){
                        if(strcmp(groups[i].participants[j],"")!=0)
                        {
                            strcat(tmp_string,groups[i].participants[j]);
                            strcat(tmp_string," ");
                        }

                      

                    }
                    strcat(tmp_string,"\n");

                }
                
            }
            strcpy(odb.text,"Participants\n");
            strcat(odb.text,tmp_string);
            odb.type=4;
            msgsnd(msqid_tmp,&odb,sizeof(odb.text)+1,0);
            strcpy(tmp_string,"");
            break;
        case 8:
            ptr=strtok(odb.text,"-");
            msqid_tmp=find_msqid(users,ptr);
            strcpy(tmp_string,ptr);
            ptr=strtok(NULL,"-");
            for(int i=0;i<groups_num;i++){
                if(strcmp(ptr,groups[i].group_name)==0){
                    for(int j=0;i<users_num;j++){
                        if(strcmp(groups[i].participants[j],"")==0)
                        {
                            strcpy(groups[i].participants[j],tmp_string);
                            printf("%s joined %s\n",tmp_string,groups[i].group_name);
                            strcpy(tmp_string,"");
                            break;
                        }
                    }
                    break;
                }

                
            }
            strcpy(tmp_string,"");
            break;
        case 9:
            ptr=strtok(odb.text,"-");
            msqid_tmp=find_msqid(users,ptr);
            strcpy(tmp_string,ptr);
            ptr=strtok(NULL,"-");
            for(int i=0;i<groups_num;i++){
                if(strcmp(ptr,groups[i].group_name)==0){
                    for(int j=0;i<users_num;j++){
                        if(strcmp(groups[i].participants[j],tmp_string)==0)
                        {
                            strcpy(groups[i].participants[j],"");
                            printf("%s left %s\n",tmp_string,groups[i].group_name);
                            strcpy(tmp_string,"");
                            break;
                        }
                    }
                    break;
                }

                
            }
            strcpy(tmp_string,"");
            break;
        case 10:
            printf("To Group: %s", odb.text);
            strcpy(rec,odb.text);
            for(int h=0;h<groups_num;h++)
            { 
                rec[strcspn(rec, "\n")] = 0;
                if(strcmp(groups[h].group_name,rec)==0) 
                    {
                    msgrcv(msqid_srv,&odb, sizeof(odb.text)+1,11,0);
                    strcpy(tmp_string,odb.text);
                    strcpy(tmp_string2,odb.text);
                    ptr=strtok(tmp_string,":");
                    odb.type=5;
                    if(find_if_group(groups,groups[h].group_name,ptr)==0){
                        strcpy(odb.text,"You are not member of this group\n");
                        msqid_tmp=find_msqid(users,ptr);
                        msgsnd(msqid_tmp,&odb,sizeof(odb.text)+1,0);
                        break;
                    }
                    else{
                    
                    for(int j=0;j<users_num;j++)
                    {
                        if(strcmp(groups[h].participants[j],"")!=0 && strcmp(groups[h].participants[j],ptr)!=0)
                        {
                            msqid_tmp=find_msqid(users,groups[h].participants[j]);
                            strcpy(odb.text,"From: ");
                            strcat(odb.text,groups[h].group_name);
                            strcat(odb.text," \n");
                            strcat(odb.text,tmp_string2);
                            msgsnd(msqid_tmp,&odb,sizeof(odb.text)+1,0);
                            
                        }
                    }
                    strcpy(tmp_string,"");
                    break;
                    }
                    }
            }
            break;
        default:
            break;
        }
        printf("Operation done\n");


    }


    // msgctl(msqid1,IPC_RMID,NULL);
    // msgctl(msqid2,IPC_RMID,NULL);
    // msgctl(msqid2,IPC_RMID,NULL);
}

