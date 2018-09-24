/**
 * @Author: Atul Sahay <atul>
 * @Date:   2018-09-24T19:18:06+05:30
 * @Email:  atulsahay01@gmail.com
 * @Last modified by:   atul
 * @Last modified time: 2018-09-24T19:18:27+05:30
 */



/*Required Headers*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include<string.h>

int main()
{

    char str[100];
    int listen_fd, comm_fd;

    struct sockaddr_in servaddr;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    bzero( &servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(22000);

    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listen_fd, 10);

    comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);

    while(1)
    {

        bzero( str, 100);

        read(comm_fd,str,100);

        printf("Echoing back - %s",str);

        write(comm_fd, str, strlen(str)+1);

    }
}
