#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/wait.h>
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
struct msgbuf
{
    long type;
    char text[200];
};
#define groups_num 3
int main(){

    char name[20];
    char password[40],input[200];
    char choice;
    key_t key1;
    key_t key_log=1111;
    key_t key_srv=1001;
    int msqid_log,msqid,msqid_srv,id1;
    char name_pass[60]="";
    struct msgbuf log_in,snd,odb;
    if((msqid_log = msgget(key_log, 0666|IPC_CREAT)) == -1)
    {
        perror(" msgget error\n");
        exit(1);
    }
    if((msqid_srv = msgget(key_srv,0666 | IPC_CREAT)) == -1){
        perror("server not active...\n ");
        exit(1);
    }
    while(1)
    {
        printf("Log In \nlogin: ");
        scanf("%s",name);
        printf("\npassword: ");
        scanf("%s",password);
        strcpy(name_pass,name);
        strcat(name_pass,"-");
        strcat(name_pass,password);
        strcpy(log_in.text,name_pass);
        log_in.type=1;
        msgsnd(msqid_log,&log_in,sizeof(log_in.text)+1,0);
        msgrcv(msqid_log,&log_in,sizeof(log_in.text)+1,2,0);
        if(atoi(log_in.text)>0){
            key1=atoi(log_in.text);
            printf("Success Logged in\n");
            break;
        }
        else{
            printf("Try again\n");
        }
    }
    log_in.type=4;
    strcpy(log_in.text,name);
    msgsnd(msqid_srv,&log_in,sizeof(log_in.text)+1,0);
    if((msqid = msgget(key1,0666 | IPC_CREAT)) == -1){
        perror("server not active...\n ");
        exit(1);
    }

    if((id1=fork())==0)
    {
        while(1){
            msgrcv(msqid,&odb,sizeof(odb.text)+1,0,0);
            switch (odb.type)
            {
            case 1:
                printf("%s\n",odb.text);
                break;
            case 2:
                printf("Online:\n");
                printf("%s\n",odb.text);
                break;
            case 3:
                printf("%s\n", odb.text);
                break;
            case 4:
                printf("%s\n",odb.text);
                break;
            case 5:
                printf("%s\n",odb.text);
                break;
            default:
                break;
            }

        }

    }
    printf("1 - send message to single person\n2- send message to group\nl - list logged in users\ng - list available groups\nz - list group members\nj - join group\nq - log out\n");
    while(1){
        scanf("%c",&choice);
        switch (choice)
        {
        case '1':
            getchar();
            printf("To who: ");
            fgets(snd.text,200,stdin);
            snd.type=1;
            msgsnd(msqid_srv,&snd,sizeof(snd.text)+1,0);
            printf("You: ");
            fgets(input,200,stdin);
            strcpy(snd.text,name);
            strcat(snd.text,": ");
            strcat(snd.text,input);
            snd.type=2;
            msgsnd(msqid_srv,&snd,sizeof(snd.text)+1,0);                    
            break;
        case 'l':
            strcpy(odb.text,name);
            odb.type=3;
            msgsnd(msqid_srv,&odb,sizeof(odb.text)+1,0);
            break;    
        case 'q':
            odb.type=5;
            strcpy(odb.text,name);
            msgsnd(msqid_srv,&odb,sizeof(odb.text)+1,0);
            kill(id1,SIGTERM);
            exit(0);
            break;
        case 'g':
            odb.type=6;
            strcpy(odb.text,name);
            msgsnd(msqid_srv,&odb,sizeof(odb.text)+1,0);
            break;
        case 'z':
            printf("Type in group: \n");
            odb.type=7;
            strcpy(odb.text,name);
            getchar();
            fgets(input,30,stdin);
            strcat(odb.text,"-");
            strcat(odb.text,input);
            msgsnd(msqid_srv,&odb,sizeof(odb.text)+1,0);
            break;
        case 'j':
            getchar();
            printf("Type in group: \n");
            fgets(input,200,stdin);
            input[strcspn(input, "\n")] = 0;
            strcpy(odb.text,name);
            strcat(odb.text,"-");
            strcat(odb.text,input); 
            odb.type=8;
            msgsnd(msqid_srv,&odb,sizeof(odb.text)+1,0);
            printf("Now you are member of this group...\n");
            break;
        case 'd':
            getchar();
            fgets(input,200,stdin);
            input[strcspn(input, "\n")] = 0;
            strcpy(odb.text,name);
            strcat(odb.text,"-");
            strcat(odb.text,input); 
            odb.type=9;
            msgsnd(msqid_srv,&odb,sizeof(odb.text)+1,0);
            printf("You are no longer memeber of this group...\n");
            break;
        case '2':
            getchar();
            printf("Type in group: ");
            fgets(odb.text,200,stdin);
            odb.type=10;
            msgsnd(msqid_srv,&odb,sizeof(odb.text)+1,0);
            printf("You: ");
            fgets(input,200,stdin);
            strcpy(odb.text,name);
            strcat(odb.text,": ");
            strcat(odb.text,input);
            odb.type=11;
            msgsnd(msqid_srv,&odb,sizeof(odb.text)+1,0);                    
            break;
        default:
            break;
        }

    }


       

}