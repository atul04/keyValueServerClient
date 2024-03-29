/**
 * @Author: Atul Sahay <atul>
 * @Date:   2018-09-25T00:04:29+05:30
 * @Email:  atulsahay01@gmail.com
 * @Last modified by:   atul
 * @Last modified time: 2018-10-07T15:44:41+05:30
 */



#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// Maximum input size from the interactive mode or batch mode
#define MAX_INPUT_SIZE 100000
#define MAX_TOKEN_SIZE 100000
#define MAX_NUM_TOKENS 100000

//tokenize the input string
char **tokenize(char *line);
////////////////////////////////

// Global vars for connection related things
int sockfd;
bool activeConn; /* Needed to check whether client has any active connection or not*/
struct sockaddr_in servaddr; /* structure where all connection related things are stored*/

void parser(char **tokens);
void lineByline(FILE * file);
char * readline(FILE *fp, char *buffer);

/**** function I tried to read line by line not worked accordingly****/
// char * readMy()
// {
//
//       int ch;
//       int i = 0;
//       size_t buff_len = 0;
//
//       char *buffer = (char *)malloc(buff_len + 1);
//       if (!buffer) return NULL;  // Out of memory
//
//       while ((ch = fgetc(stdin)) != '\n' && ch != EOF)
//       {
//           buff_len++;
//           void *tmp = realloc(buffer, buff_len + 1);
//           if (tmp == NULL)
//           {
//               free(buffer);
//               return NULL; // Out of memory
//           }
//           buffer = tmp;
//
//           buffer[i] = (char) ch;
//           i++;
//       }
//       buffer[i] = '\0';
//
//       // Detect end
//       if (ch == EOF && (i == 0 || ferror(stdin)))
//       {
//           free(buffer);
//           return NULL;
//       }
//       return buffer;
// }
////////////////////////////////////
int main(int argc,char **argv)
{

    int sockfd,n;
    char sendline[10000];
    char recvline[10000];
    struct sockaddr_in servaddr;

    // whether the client have an active connection or not
    bool activeConn = false;

    char  *line = (char *)malloc(MAX_INPUT_SIZE*sizeof(char));
    char **tokens;

    // To select between batch and interactive mood
    if(argc == 2)
    {
        printf("Welcome to Interactive Mode\n");
        int i =0;
        while(true)
        {
            int tokenCount=0;
            printf("->");
            bzero(line, MAX_INPUT_SIZE);
            // line = readMy();
            fgets(line,MAX_INPUT_SIZE,stdin);
            line[strlen(line)] = '\n'; //terminate with new line
            tokens = tokenize(line);
            parser(tokens);
            int i;
            //Freeing up the acquired space by the tokens
            for(i=0;tokens[i]!=NULL;i++){
                free(tokens[i]);
            }
            free(tokens);

        }
    }
    else if(argc == 3)
    {
        printf("Welcome to batch mode\nFile: %s\n",argv[2]);
        //open and get the file handle
        char *fileName = argv[2];
        FILE* file = fopen(fileName, "r");
        lineByline(file);
        return 0;
    }
    else
    {
        printf("Incorrect arguements\n");
        exit(0);
    }
}



////////// Function Definition are written here ////////////////////////

/**************** Parsing the commands ***********/
void parser(char **tokens){
  // char *sendline =(char*)malloc(MAX_INPUT_SIZE*sizeof(char));
  // char *recvline =(char*)malloc(MAX_INPUT_SIZE*sizeof(char));
  char sendline[MAX_INPUT_SIZE];
  char recvline[MAX_INPUT_SIZE];
  int n,i;          /* n for storing the reuturn values of each query made to the server*/
  int tokenCount = 0;
  bzero(sendline,MAX_INPUT_SIZE);
  bzero(recvline,MAX_INPUT_SIZE);
  //do whatever you want with the commands, here we just print them
  for(i=0;tokens[i]!=NULL;i++){
     // printf("found token %s\n", tokens[i]);
     tokenCount++;
  }

  /// All the functions are going to be written here
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
          activeConn = false;
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
              bzero(sendline,MAX_INPUT_SIZE);
              strcpy(sendline,tokens[2]);
              write(sockfd,sendline,strlen(sendline)+1);
              read(sockfd,recvline,100);
              printf("%s\n",recvline);

              int i = 3;
              int totalStrLen = 0;
              while(i<tokenCount)
              {
                  totalStrLen+=strlen(tokens[i]);
                  totalStrLen+=1;
                  i++;
              }
              bzero(sendline,MAX_INPUT_SIZE);
              i=3;
              while(i<tokenCount)
              {
                  strcat(sendline,tokens[i]);
                  strcat(sendline," ");
                  i+=1;
              }
              sendline[strlen(sendline)-1]=0;
              printf("\nYour text ::::-> %s \n\nText size:%ld\n\n",sendline,strlen(sendline));
              write(sockfd,sendline,strlen(sendline)+1);

              n = read(sockfd,recvline,10000);
              printf("%s\n",recvline);
              // while(i<tokenCount){
              //     strcpy(sendline,tokens[i]);
              //     write(sockfd,sendline,strlen(sendline)+1);
              //     n = read(sockfd,recvline,100);
              //     printf("%s\n",recvline);
              //     i+=1;
              // }
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
          bzero(sendline,MAX_INPUT_SIZE);
          strcpy(sendline,"read");
          write(sockfd,sendline,strlen(sendline)+1);
          read(sockfd,recvline,100);
          printf("%s\n",recvline);
          bzero(sendline,MAX_INPUT_SIZE);
          strcpy(sendline,tokens[1]);
          // bzero(sendline,MAX_INPUT_SIZE);
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
            // printf("%d\n",n);
            sscanf(buffer, "%d", &size);
            printf("Read: Got size value from the server= %d\n",size);
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
                    return 0;
                }
                // printf("%d\n",readThisTime);
                bytesRead += readThisTime;
            }

            printf("RECEIVED TEXT : %s\n",bufferRead);
          }
      }
      else{
          printf("No active connection present\n");
      }
  }
  else if(strncmp(tokens[0],"update",6)==0)
  {
      if(activeConn){
          bool present = true;
          strcpy(sendline,"update");
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
            bzero(sendline,MAX_INPUT_SIZE);
            strcpy(sendline,tokens[2]);
            write(sockfd,sendline,strlen(sendline)+1);
            read(sockfd,recvline,100);
            printf("%s\n",recvline);

            int i = 3;
            int totalStrLen = 0;
            while(i<tokenCount)
            {
                totalStrLen+=strlen(tokens[i]);
                totalStrLen+=1;
                i++;
            }
            bzero(sendline,MAX_INPUT_SIZE);
            i=3;
            // printf("%s\n","Going to count now" );
            while(i<tokenCount)
            {
                strcat(sendline,tokens[i]);
                strcat(sendline," ");
                i+=1;
            }
            sendline[strlen(sendline)-1]=0;
            printf("\nText Read:::-> %s \n\nText size: %ld\n\n",sendline,strlen(sendline));
            write(sockfd,sendline,strlen(sendline)+1);

            n = read(sockfd,recvline,10000);
            printf("%s\n",recvline);
            // while(i<tokenCount){
            //     strcpy(sendline,tokens[i]);
            //     write(sockfd,sendline,strlen(sendline)+1);
            //     n = read(sockfd,recvline,100);
            //     printf("%s\n",recvline);
            //     i+=1;
            // }
            n = read(sockfd,recvline,100);
            printf("%s\n",recvline);
          }
      }
      else{
          printf("No active connection present\n");
      }
  }
  else if(strncmp(tokens[0],"delete",6)==0)
  {
      if(activeConn){
          bool present = true;
          strcpy(sendline,"delete");
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
            printf("%s\n",buffer);
          }
      }
      else{
          printf("No active connection present\n");
      }
  }
  else
  {
      printf("No command found!! Please try again\n");
  }

}



//////// Reading FILE

char * readline(FILE *fp, char *buffer)
{
    int ch;
    int i = 0;
    size_t buff_len = 0;

    buffer = malloc(buff_len + 1);
    if (!buffer) return NULL;  // Out of memory

    while ((ch = fgetc(fp)) != '\n' && ch != EOF)
    {
        buff_len++;
        void *tmp = realloc(buffer, buff_len + 1);
        if (tmp == NULL)
        {
            free(buffer);
            return NULL; // Out of memory
        }
        buffer = tmp;

        buffer[i] = (char) ch;
        i++;
    }
    buffer[i] = '\0';

    // Detect end
    if (ch == EOF && (i == 0 || ferror(fp)))
    {
        free(buffer);
        return NULL;
    }
    return buffer;
}
////////// Ends (Reading the file)

/* Line by line interpretation of the command*/
void lineByline(FILE * file){
  char *s;
  char **tokens;
  int i;
  int tokenCount=0;
  char  *line = (char *)malloc(MAX_INPUT_SIZE*sizeof(char));

  char sendline[10000];
  char recvline[10000];

  // whether the client have an active connection or not
  bool activeConn = false;

  int sockfd,n;
  struct sockaddr_in servaddr;

  while ((s = readline(file, 0)) != NULL){
      printf("Command ---> %s\n",s);
      bzero(line, MAX_INPUT_SIZE);
      strcpy(line,s);
      line[strlen(line)] = '\n'; //terminate with new line
      // puts(line);
      tokens = tokenize(line);

      parser(tokens); // parser is called
      int i;
      //Freeing up the acquired space by tokens
      for(i=0;tokens[i]!=NULL;i++){
          free(tokens[i]);
      }
      free(tokens);
      free(s);
      printf("\n");
    }
}

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
