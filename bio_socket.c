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
    BIO_set_conn_hostname(bio, "10.169.37.180");
    BIO_set_conn_port(bio, "5000");
    ret = BIO_do_connect(bio);


    int num_bytes_written = 0;
    char request[500] =
        "GET  /PKIClient/api/crl/pki_int_953B9DA915 HTTP/1.1\r\n\0\0";

    char response[1024] = "";
    BIO_puts(bio, request);
    getchar();
    BIO_free(bio);
    return 0;
}
