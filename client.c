/**
 * @Author: Atul Sahay <atul>
 * @Date:   2018-09-25T00:04:29+05:30
 * @Email:  atulsahay01@gmail.com
 * @Last modified by:   atul
 * @Last modified time: 2018-09-25T17:07:08+05:30
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
    char sendline[100];
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
        int tokenCount=0;
        int i =0;
        while(true)
        {
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
                    sscanf(str, "%d", &port);

                    sockfd=socket(AF_INET,SOCK_STREAM,0);
                    bzero(&servaddr,sizeof servaddr);

                    servaddr.sin_family=AF_INET;
                    servaddr.sin_port=htons(port);

                    inet_pton(AF_INET,address,&(servaddr.sin_addr));

                    connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
                    activeConn = true;
                    printf("ok Connection made\n");
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

            }
            else if(strncmp(tokens[0],"read",4)==0)
            {

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
