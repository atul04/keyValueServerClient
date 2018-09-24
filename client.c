/**
 * @Author: Atul Sahay <atul>
 * @Date:   2018-09-25T00:04:29+05:30
 * @Email:  atulsahay01@gmail.com
 * @Last modified by:   atul
 * @Last modified time: 2018-09-24T19:18:56+05:30
 */



#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include<string.h>

int main(int argc,char **argv)
{
    int sockfd,n;
    char sendline[100];
    char recvline[100];
    struct sockaddr_in servaddr;

    sockfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof servaddr);

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(22000);

    inet_pton(AF_INET,"127.0.0.1",&(servaddr.sin_addr));

    connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

    while(1)
    {
        bzero( sendline, 100);
        bzero( recvline, 100);
        fgets(sendline,100,stdin); /*stdin = 0 , for standard input */

        write(sockfd,sendline,strlen(sendline)+1);
        read(sockfd,recvline,100);
        printf("%s",recvline);
    }

}
