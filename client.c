/**
 * @Author: Atul Sahay <atul>
 * @Date:   2018-09-25T00:04:29+05:30
 * @Email:  atulsahay01@gmail.com
 * @Last modified by:   atul
 * @Last modified time: 2018-09-26T00:22:38+05:30
 */



#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include<stdlib.h>

// Maximum input size from the interactive mode or batch mode
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

//tokenize the input string
char **tokenize(char *line);

int main(int argc,char **argv)
{

    int sockfd,n;
    char sendline[10000];
    char recvline[100];
    struct sockaddr_in servaddr;

    // whether the client have an active connection or not
    bool activeConn = false;

    char  line[MAX_INPUT_SIZE];
    char **tokens;
    // To select between batch and interactive modified
    if(argc == 2)
    {
        printf("Welcome to Interactive Mode\n");
        int i =0;
        while(true)
        {
            int tokenCount=0;
            printf("->");
            bzero(line, MAX_INPUT_SIZE);
            fgets(line,MAX_INPUT_SIZE,stdin);
            line[strlen(line)] = '\n'; //terminate with new line
            tokens = tokenize(line);

            //do whatever you want with the commands, here we just print them
            for(i=0;tokens[i]!=NULL;i++){
               printf("found token %s\n", tokens[i]);
               tokenCount++;
            }

            if(strncmp(tokens[0],"connect",7)==0)
            {
                if(!activeConn)
                {
                    char address[1024];
                    char portStr[1024];
                    strcpy(address,tokens[1]);
                    strcpy(portStr,tokens[2]);
                    // for string to int
                    int port;
                    sscanf(portStr, "%d", &port);

                    sockfd=socket(AF_INET,SOCK_STREAM,0);
                    bzero(&servaddr,sizeof servaddr);

                    servaddr.sin_family=AF_INET;
                    servaddr.sin_port=htons(port);

                    inet_pton(AF_INET,address,&(servaddr.sin_addr));

                    if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0)
                    {
                        printf("\nConnection Failed \n");
                    }
                    else{
                        activeConn = true;
                        printf("ok Connection made\n");
                    }
                }

                else{
                    printf("Already have a connection\n");
                }
            }
            else if(strncmp(tokens[0],"disconnect",10)==0)
            {
                if(activeConn)
                {
                    strcpy(sendline,"bye");
                    write(sockfd,sendline,strlen(sendline)+1);
                    read(sockfd,recvline,100);
                    printf("%s\n",recvline);
                }
                else
                {
                    printf("Please first make an active connection\n");
                }
            }
            else if(strncmp(tokens[0],"create",6)==0)
            {
                if(activeConn){
                    bool present = false;
                    strcpy(sendline,"create");
                    write(sockfd,sendline,strlen(sendline)+1);
                    read(sockfd,recvline,100);
                    printf("%s\n",recvline);

                    strcpy(sendline,tokens[1]);
                    write(sockfd,sendline,strlen(sendline)+1);
                    read(sockfd,recvline,100);
                    printf("%s\n",recvline);

                    if(strncmp(recvline,"present",7) == 0)
                    {
                        present=true;
                        printf("Error: key is already present\n");
                    }
                    if(!present){
                        strcpy(sendline,tokens[2]);
                        write(sockfd,sendline,strlen(sendline)+1);
                        read(sockfd,recvline,100);
                        printf("%s\n",recvline);

                        int i = 3;

                        while(i<tokenCount){
                            strcpy(sendline,tokens[i]);
                            write(sockfd,sendline,strlen(sendline)+1);
                            n = read(sockfd,recvline,100);
                            printf("%s\n",recvline);
                            i+=1;
                        }
                        n = read(sockfd,recvline,100);
                        printf("%s\n",recvline);
                    }
                }
                else{
                    printf("No active connection present\n");
                }
            }
            else if(strncmp(tokens[0],"read",4)==0)
            {
                if(activeConn){
                    bool present = true;
                    strcpy(sendline,"read");
                    write(sockfd,sendline,strlen(sendline)+1);
                    read(sockfd,recvline,100);
                    printf("%s\n",recvline);
                    strcpy(sendline,tokens[1]);
                    write(sockfd,sendline,strlen(sendline)+1);
                    read(sockfd,recvline,100);
                    printf("%s\n",recvline);

                    if(strncmp(recvline,"not",3) == 0)
                    {
                        present=false;
                        printf("Error: key is not present\n");
                    }
                    if(present){
                      unsigned int length;
                      int size;
                      char buffer[256];
                      bzero(buffer,256);
                      n=read(sockfd,buffer,255);
                      printf("%d\n",n);
                      sscanf(buffer, "%d", &size);
                      printf("size = %d\n",size);
                      char dummy[10];
                      bzero(dummy,10);
                      n=write(sockfd,dummy,strlen(dummy)+1);
                      int bytesRead = 0;
                      int bytesToRead =size+1;
                      int readThisTime;
                      char *bufferRead = (char *)malloc(bytesToRead*sizeof(char));
                      bzero(bufferRead,size+1);
                      while (bytesToRead != bytesRead)
                      {
                          do
                          {
                               readThisTime = read(sockfd, bufferRead + bytesRead, (bytesToRead - bytesRead));
                          }
                          while(readThisTime == -1);

                          if (readThisTime == -1)
                          {
                              /* Real error. Do something appropriate. */
                              return;
                          }
                          printf("%d\n",readThisTime);
                          bytesRead += readThisTime;
                      }

                      printf("TEXT : %s\n",bufferRead);
                    }
                }
                else{
                    printf("No active connection present\n");
                }
            }
            else if(strncmp(tokens[0],"update",6)==0)
            {

            }
            else if(strncmp(tokens[0],"delete",6)==0)
            {

            }
            else
            {
                printf("No command found!! Please try again\n");
            }
        }
    }
    else if(argc == 3)
    {
        printf("Welcome to batch mode\nFile: %s\n",argv[2]);
    }
    else
    {
        printf("Incorrect arguements\n");
        exit(0);
    }

      // sockfd=socket(AF_INET,SOCK_STREAM,0);
      // bzero(&servaddr,sizeof servaddr);
      //
      // servaddr.sin_family=AF_INET;
      // servaddr.sin_port=htons(5002);
      //
      // inet_pton(AF_INET,"127.0.0.1",&(servaddr.sin_addr));
      //
      // connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
      //
      // while(1)
      // {
      //     bzero( sendline, 100);
      //     bzero( recvline, 100);
      //     fgets(sendline,100,stdin); /*stdin = 0 , for standard input */
      //
      //     write(sockfd,sendline,strlen(sendline)+1);
      //     read(sockfd,recvline,100);
      //     printf("%s",recvline);
      // }
}



////////// Function Definition are written here ////////////////////////



//tokenize the input string
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0;
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }

  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}
