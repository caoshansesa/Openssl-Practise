#include <arpa/inet.h>
#include <openssl/bio.h>
#include <stdio.h>

int main()
{
    int ret = 0;
    BIO *bio = NULL;
    bio = BIO_new(BIO_s_connect());
    if (bio == NULL)
    {
        ret = -1;
        printf("BIO creation failed.\n");
    }
    BIO_set_conn_hostname(bio, "10.169.37.248");
    BIO_set_conn_port(bio, "80");
    ret = BIO_do_connect(bio);

    int num_bytes_written = 0;
    char request[128] = "GET  / HTTP/1.1\r\n";

    char response[1024] = "";
    BIO_puts(bio, request);

    int num_bytes_read = 0;
    do
    {
        num_bytes_read = BIO_read(bio, response, 1023);
        if ((num_bytes_read < 1023) && (num_bytes_read > 0))
        {
            break; // read in size < 1024, buffer is empty.
            response[1023] = '\0';
            if (fputs(response, stdout) == EOF)
            {
                printf("\n Error : Fputs error\n");
            }
        }

    } while (num_bytes_read > 0 || BIO_should_retry(bio));
    getchar();
    BIO_free(bio);
    return 0;
}
