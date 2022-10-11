/* filename withssl.c */
#include "stdio.h"
#include "string.h"

#include "openssl/ssl.h"
#include "openssl/bio.h"
#include "openssl/err.h"

int main(int argc, char *argv[])
{
    BIO * bio;
    SSL * ssl;
    SSL_CTX * ctx;

    char resp[1024];
    int  ret;

    char* request = "GET /v1/pki_int_D8C0F20133/crl HTTP/1.1\x0D\x0A http://10.169.37.248:8200\x0D\x0A\x43onnection: Close\x0D\x0A\x0D\x0A";
    //char * request = "GET / HTTP/1.1\x0D\x0AHost: worktile.com\x0D\x0A\x43onnection: Close\x0D\x0A\x0D\x0A";
    //char * request = "GET /cas/login?service=https%3A%2F%2Fweb.corp.ema-tech.com%3A8888%2F HTTP/1.1\x0D\x0AHost: web.corp.ema-tech.com\x0D\x0A""Connection: Close\x0D\x0A\x0D\x0A";

    /* Attention: "\x43" is "C", why is "\x43onnection" but not "Connection" ?
       because "\x0AC" in "\x0D\x0AConnection" is taken as a hex value, not the string "\nConnection"
       also, "\x0D""C" can avoid upper problem too.
     */

    /* Init SSL library */
    SSL_library_init();

    /* Set up the library */
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    /* Set up the SSL context */
    const SSL_METHOD *method = SSLv23_client_method();	/* SSLv3 but can rollback to v2 */
    if (! method) {
        fprintf(stderr, "SSL client method failed\n");
        return 1;
    }

    ctx = SSL_CTX_new(method);
    if (! ctx) {
        fprintf(stderr, "SSL context is NULL\n");
        ERR_print_errors_fp(stderr);
        return 1;
    }

#ifdef NO_VERIFY
    /* Load the trust store */
    if(! SSL_CTX_load_verify_locations(ctx, argv[1], NULL)) {
        fprintf(stderr, "Error loading trust store\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return 0;
    }
#endif

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL); // Once way verification

    /* Setup the connection */
    bio = BIO_new_ssl_connect(ctx);

    /* Set the SSL_MODE_AUTO_RETRY flag */
    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    /* Create and setup the connection */
    BIO_set_conn_hostname(bio, "10.169.37.248:8200");
    //BIO_set_conn_hostname(bio, "worktile.com:https");
    //BIO_set_conn_hostname(bio, "web.corp.ema-tech.com:8888");

    if(BIO_do_connect(bio) <= 0) {
        fprintf(stderr, "Error attempting to connect\n");
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return 0;
    }

    /* Check the certificate */
    if(SSL_get_verify_result(ssl) != X509_V_OK)
    {
        fprintf(stderr, "Certificate verification error: %ld\n", SSL_get_verify_result(ssl));
        /*
          Error Tip "error : 19 self signed certificate in certificate chain" means:
          This means the certificate chain returned by the server ends with a ‘self signed certificate’.
          Since the self-signed certificate is not a trusted certificate, it is reported as an error.
          You can make the problem go away by specifying a trusted root CA (certificate authority)
         */
        fprintf(stderr, "Error: %s\n", ERR_reason_error_string(ERR_get_error()));
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return 0;
    }

    /* Send the request */
    BIO_write(bio, request, strlen(request));

    /* Read in the response */
    for(;;) {
        ret = BIO_read(bio, resp, 1023);
        if(ret <= 0) break;
        resp[ret] = 0;
        printf("%s", resp);
    }

    /* Close the connection and free the context */
    BIO_free_all(bio);
    SSL_CTX_free(ctx);
    return 0;
}

