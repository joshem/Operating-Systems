#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

/*NOTE: Future Josh, if you want to review this, also check parser.c, shell.c, sheller.c, they have better comments. sheller was made cause of seg faults in shell function.. */
//keeps track of stuff
int pipes = 0;
int inRedirect = 0;
int outRedirect =0;
int backgrnd = 0;
int position =0;
char line[512];
char **tokenize(char line[512]){
        //reinit
        pipes=0;
        inRedirect=0;
        outRedirect = 0;
        backgrnd = 0;
        position = 0;

        //char line[512]; //= malloc(512*sizeof(char));
        //if(fgets(line,512,stdin)==NULL) break;
        char *parsed=malloc(512*sizeof(char));

        int letterCnt = 0;
        int parsCnt = 0;

        //insert spaces when needed for metachars, ie cat a|sort --> cat a | sort
        while(line[letterCnt] != '\0'){
                if(line[letterCnt]=='|'){
                        pipes++;
                        strcat(parsed," | ");
                        parsCnt+=3;
                }
                else if(line[letterCnt]=='<'){
                        strcat(parsed," < ");
                        parsCnt+=3;
                        inRedirect++;
                }
                else if(line[letterCnt]=='>'){
                        strcat(parsed," > ");
                        parsCnt+=3;
                        outRedirect++;
                }
                else if(line[letterCnt]=='&'){
                        strcat(parsed," & ");
                        parsCnt+=3;
                        backgrnd++;
                }

                else{
                        parsed[parsCnt]=line[letterCnt];
                        parsCnt++;
                }
                letterCnt++;
        }



        //tokenize the string, put tokens in own string in array of strings (ie 2d char arr)
        char * str;
        char ** token = malloc(512 * sizeof(char*));
       // int position = 0;
        str = strtok(parsed,"|");
        while(str!=NULL){
                token[position] = str;
                str = strtok(NULL,"|");
                position++;
                
        }
        //printf("%s\n", token[0]);
        //i think its a prob with returning..it's not returning token correctly, possibly cause we're removing \n?? not that, cause i tried doing that above..
        return token;
}


//where the magic happens
void shell(char** tokens){

       //printf("pipes is: %d\n", pipes);
        int Pipe_in = 0;
        int outFD = dup(STDOUT_FILENO);
        int inFD  = dup(STDIN_FILENO);
        
        char *inFile = malloc(32*sizeof(char));
        char *outfile = malloc(32*sizeof(char));
        
        //

         if(pipes==0){

                char * str;
                //printf("tokens[0]: %s\n", tokens[0]);
                str=strtok(tokens[0]," \n");
                
                char ** words = malloc(512*sizeof(char*));
               // printf("saved content: %s\n", words[0]);
               // printf("toks content:%s\n", toks[1]);
                int wrdCnt = 0;
                if(inRedirect==0 &&outRedirect==0){
                        while(str!=NULL){
                          // printf("str :%s\n",str );
                           words[wrdCnt] = str;
                          wrdCnt++;
                         str = strtok(NULL," \n");
                        }

                }
                else{
                       // printf("lol jk\n");
                        bool inDirect = false;
                        bool outDirect = false;
                        while(str!=NULL){
                                if(strcmp(str,"<")==0){
                                        inDirect=true;
                                }
                                else if((strcmp(str,">")==0)){
                                        outDirect=true;
                                }
                                if(inDirect){
                                        printf("%s\n",str );
                                        str = strtok(NULL," \n");
                                        //str = strtok(NULL," \n");
                                        inFile = str;
                                        inFD = open(inFile,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                                        if(inFD<0){
                                                perror("ERROR");
                                        }
                                        //printf("infile%s\n",inFile );
                                        inDirect=false;

                                }
                                else if(outDirect){
                                        
                                        str = strtok(NULL," \n");
                                        
                                        //str = strtok(NULL," \n");
                                        outfile=str;
                                        
                                        outFD = creat(outfile,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                                        if(outFD<0){
                                                perror("ERROR");
                                        }
                                        outDirect=false;
                                }
                                else{
                                        words[wrdCnt] = str;
                                        str = strtok(NULL," \n");
                                       
                                }
                                wrdCnt++;
                        }
                }

                pid_t pid, wpid;
                pid = fork();
                int status;

                //error forking
                if(pid < 0){
                        perror("ERROR");
                        return;
                }

                //child process
                else if(pid == 0){
                        //do something
                        dup2(inFD,STDIN_FILENO);
                        dup2(outFD,STDOUT_FILENO);
                        if(execvp(words[0],words)<0){
                                perror("ERROR");
                                return;
                        }
                }

                //parent process
                else{
                        //wait() or     waitpid()???
                        do{
                                wpid=waitpid(pid, &status, WUNTRACED);
                        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
                }

        }
   
        //Hardcode 
        //there are pipes, do black magic
        else if(pipes==1){
               // printf("there were pipes\n");
              //  printf("position: %d\n", position);
                //int std = dup(1);
                //printf("position is: %d\n", position);
                int i;
               //for(i=0;i<pipes;i++){
                        
                        char * str = tokens[0];//before pipe
                        char * strPiped = tokens[1];//after pipe
                        
                        //tokenize str1
                        char ** execArgs = malloc(512*sizeof(char*));
                        char * strArgs = strtok(str," \n");
                        int word = 0;
                        while(strArgs!=NULL){
                                execArgs[word]=strArgs;
                                word++;
                                strArgs=strtok(NULL," \n");
                        }
                       

                        //tokenize strPiped
                        char ** execArgsPiped = malloc(512*sizeof(char*));
                        char * strArgsPipe = strtok(strPiped," \n");
                        word=0;
                        while(strArgsPipe!=NULL){
                                execArgsPiped[word]=strArgsPipe;
                                word++;
                                strArgsPipe=strtok(NULL," \n");
                        }
                        //there, now have two arrays, one holding stuff before a pipe, one holding stuff after, both tokenized as words


                        pid_t pid_1, pid_2, wpid_1, wpid_2;
                        int status_1, status_2;
                        int pipeFd[2];

                        if(pipe(pipeFd)<0){
                                perror("ERROR");
                                return;
                        }

                        pid_1=fork();
                        if(pid_1<0){
                                perror("ERROR");
                                return;
                        }

                        //child is executing
                        else if(pid_1==0){
                                close(STDOUT_FILENO);
                                close(pipeFd[0]);
                                dup2(pipeFd[1],STDOUT_FILENO);
                                //close(pipeFd[1]);
                                if(execvp(execArgs[0],execArgs)<0){
                                        perror("ERROR");
                                        return;
                                }
                                
                        }

                        //parent executing
                        else{
                                pid_2=fork();

                                if(pid_2<0){
                                        perror("ERROR");
                                        return;
                                }

                                //child 2 is executing
                                if(pid_2==0){
                                       // dup2(std,STDOUT_FILENO);
                                        close(STDIN_FILENO);
                                        close(pipeFd[1]);
                                        dup2(pipeFd[0],STDIN_FILENO);
                                       // close(pipeFd[0]);
                                        if(execvp(execArgsPiped[0],execArgsPiped)){
                                                perror("ERROR");
                                                return;
                                        }
                                        
                                }

                                //wait
                                else{
                                        /*do{
                                                wpid_1=waitpid(pid_1,&status_1,WUNTRACED);
                                                wpid_2=waitpid(pid_2,&status_2,WUNTRACED);

                                        }while(!WIFEXITED(status_1) && !WIFSIGNALED(status_1) && !WIFEXITED(status_2) && !WIFSIGNALED(status_2));*/
                                        close(pipeFd[0]);
                                        close(pipeFd[1]);
                                        wait(NULL);
                                        wait(NULL);
                                }

                        }

                //}    //for loop breacket    
        }

        else if(pipes==2){
               // printf("you got me\n");
                return;
        }
        

                return;
}

int main(){
        while(1){
                //printf("my_shell>");
               // char line[512];

                if(fgets(line,sizeof(char)*512,stdin)==NULL) break;
                //system(line);
                char ** toks = malloc(512*sizeof(char*)); 
                toks = tokenize(line);
                //printf("tokens works\n");

                shell(toks);
        }
        return 0;
}