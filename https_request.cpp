#include <errno.h>
#include <malloc.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <resolv.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fstream>
#define FAIL -1
using namespace std;
int OpenConnection(const char *hostname, int port) {
  int sd;
  struct hostent *host;
  struct sockaddr_in addr;
  if ((host = gethostbyname(hostname)) == NULL) {
    perror(hostname);
    abort();
  }
  sd = socket(PF_INET, SOCK_STREAM, 0);
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = *(long *)(host->h_addr);
  if (connect(sd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
    close(sd);
    perror(hostname);
    abort();
  }
  return sd;
}
SSL_CTX *InitCTX(void) {
  SSL_METHOD *method;
  SSL_CTX *ctx;
  OpenSSL_add_all_algorithms();     /* Load cryptos, et.al. */
  SSL_load_error_strings();         /* Bring in and register error messages */
  method = TLSv1_2_client_method(); /* Create new client-method instance */
  ctx = SSL_CTX_new(method);        /* Create new context */
  if (ctx == NULL) {
    ERR_print_errors_fp(stderr);
    abort();
  }
  return ctx;
}
void ShowCerts(SSL *ssl) {
  X509 *cert;
  char *line;
  cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
  if (cert != NULL) {
    printf("Server certificates:\n");
    line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    printf("Subject: %s\n", line);
    free(line); /* free the malloc'ed string */
    line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    printf("Issuer: %s\n", line);
    free(line);      /* free the malloc'ed string */
    X509_free(cert); /* free the malloc'ed certificate copy */
  } else
    printf("Info: No client certificates configured.\n");
}
int main() {
  SSL_CTX *ctx;
  int server;
  SSL *ssl;
  int bytes;
  char *hostname, *portnum;
  SSL_library_init();
  ctx = InitCTX();
  server = OpenConnection("10.169.37.248", 8200);
  ssl = SSL_new(ctx);           /* create new SSL connection state */
  SSL_set_fd(ssl, server);      /* attach the socket descriptor */
  if (SSL_connect(ssl) == FAIL) /* perform the connection */
    ERR_print_errors_fp(stderr);
  else {
    printf("\n\nConnected with %s encryption\n", SSL_get_cipher(ssl));
    ShowCerts(ssl); /* get any certs */
    // char request[1024] = "GET
    // https://10.169.37.248:8200/v1/pki_int_D8C0F20133/crl"; char * request =
    // "GET /v1/pki_int_D8C0F20133/crl
    // /HTTP/1.1\x0D\x0AHost: 10.169.37.248:8200\x0D\x0A\x43onnection:
    // Close\x0D\x0A\x0D\x0A";

    char request[1024] =
        "GET /v1/pki_int_D8C0F20133/crl HTTP/1.1\r\nHost: "
        "10.169.37.248\r\n\r\nConnection: close\r\n";
    SSL_write(ssl, request, 1024); /* encrypt & send message */

    char buffer_1k[1024] = {};
    char buf[2048] = {};
    int buffer_size1 = SSL_read(ssl, buf, 1024);
    int next_read_size = 0;
    int bytes_left = 0;
    do {
      bytes_left = SSL_pending(ssl);
      bytes_left > 1024 ? next_read_size = 1024 : next_read_size = bytes_left;
      if ((SSL_get_error(ssl, buffer_size1) == SSL_ERROR_WANT_READ)) {
        int buffer_size2 = SSL_read(ssl, buf + 1024, next_read_size);
      }
    } while (bytes_left != 0);

    char *crl = strstr(buf, "\r\n\r\n");
    crl += 4;
    sleep(1);
    ofstream MyFile("filename.crl");
    MyFile.write(crl, 2048);
    // Close the file
    MyFile.close();
  }
  close(server);     /* close socket */
  SSL_CTX_free(ctx); /* release context */
  return 0;
}
