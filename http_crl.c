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
    int server_fd = OpenConnection("10.169.37.180", 5000);
    char request[1024] = "GET /PKIClient/api/crl/pki_int_953B9DA915 HTTP/1.1\r\n";
    char response[1024] = "";
    int length =  strlen(request);
    write(server_fd, request, length);
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
