#include <arpa/inet.h>
#include <bits/types/FILE.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define h_addr h_addr_list[0] /* for backward compatibility */

int OpenConnection(const char *hostname, int port)
{
    int sd;
    struct hostent *host;
    struct sockaddr_in addr;
    if ((host = gethostbyname(hostname)) == NULL)
    {
        perror(hostname);
        abort();
    }
    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long *)(host->h_addr);
    if (connect(sd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        close(sd);
        perror(hostname);
        abort();
    }
    return sd;
}

int cert_mgt_get_crl_by_cdp()
{
    char recvBuff[1024];
    int digit_has_written = 0;
    memset(recvBuff, '0', sizeof(recvBuff));
    int server_fd = OpenConnection("10.169.37.248", 8200);
    char request[1024] = "GET /v1/pki_int_D8C0F20133/crl HTTP/1.1\r\n\r\nConnection: close\r\n";
    char response[1024] = "";
    write(server_fd, request, 1024);
    while ((digit_has_written = read(server_fd, recvBuff, sizeof(recvBuff) - 1)) > 0)
    {
        recvBuff[digit_has_written] = 0;
        if (fputs(recvBuff, stdout) == EOF)
        {
            printf("\n Error : Fputs error\n");
        }
    }
    return 0;
}

int main()
{
    cert_mgt_get_crl_by_cdp();
    return 0;
}
