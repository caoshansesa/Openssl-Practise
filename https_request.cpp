#include <bits/types/FILE.h>
#include <errno.h>
#include <fcntl.h>
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
#include <string>
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

void update_crl_file() {
  FILE *f = fopen("filename.crl", "rb");
  if (f == NULL) {
    printf("can not read file");
  }
  int i = 0;
  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *resp = (char *)malloc(size);
  char *crl = (char *)malloc(size);
  fread(resp, size, 1, f);

  char* crl_buffer_after_length = strstr(resp, "\r\n\r\n");
  crl_buffer_after_length +=4;
  
  char* crl_buffer = strstr(crl_buffer_after_length, "\r\n");
  crl_buffer +=2;
  
  int crl_length = size - (crl_buffer -resp);
  crl_length = crl_length - 7; // "\r\n \r\n\r\n" 7 bytes in the end;


  ofstream MyFile("filename1.crl");
  MyFile.write(crl_buffer, crl_length);

  // Close the file
  MyFile.close();
  free(resp);
}

#define BUFSIZZ 1024
char s2c[BUFSIZZ] = {};
void read_crl_buffer(SSL *ssl, int sock) {
  int width;
  int r, c2sl = 0, c2s_offset = 0;
  int read_blocked_on_write = 0, write_blocked_on_read = 0, read_blocked = 0;
  fd_set readfds, writefds;
  int shutdown_wait = 0;
  int ofcmode;
  FILE *f = fopen("filename.crl", "wb+");

  /*First we make the socket nonblocking*/
  ofcmode = fcntl(sock, F_GETFL, 0);
  ofcmode |= O_NDELAY;
  if (fcntl(sock, F_SETFL, ofcmode)) printf("Couldn't make socket nonblocking");

  width = sock + 1;

  while (1) {
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);

    FD_SET(sock, &readfds);

    /* If we're waiting for a read on the socket don't
       try to write to the server */
    if (!write_blocked_on_read) {
      /* If we have data in the write queue don't try to
         read from stdin */
      if (c2sl || read_blocked_on_write)
        FD_SET(sock, &writefds);
      else
        FD_SET(fileno(stdin), &readfds);
    }

    r = select(width, &readfds, &writefds, 0, 0);
    if (r == 0) continue;

    /* Now check if there's data to read */
    if ((FD_ISSET(sock, &readfds) && !write_blocked_on_read) ||
        (read_blocked_on_write && FD_ISSET(sock, &writefds))) {
      do {
        read_blocked_on_write = 0;
        read_blocked = 0;

        r = SSL_read(ssl, s2c, BUFSIZZ);
           if (strstr(s2c, "HTTP/1.1 400 Bad Request")) {
               r = 0;
               SSL_shutdown(ssl);
        }

        switch (SSL_get_error(ssl, r)) {
          case SSL_ERROR_NONE:
            fwrite(s2c, r, 1, f);
            break;
          case SSL_ERROR_ZERO_RETURN:
            /* End of data */
            if (!shutdown_wait) SSL_shutdown(ssl);
            goto end;
            break;
          case SSL_ERROR_WANT_READ:
            read_blocked = 1;
            break;

            /* We get a WANT_WRITE if we're
               trying to rehandshake and we block on
               a write during that rehandshake.

               We need to wait on the socket to be
               writeable but reinitiate the read
               when it is */
          case SSL_ERROR_WANT_WRITE:
            read_blocked_on_write = 1;
            break;
        }

        /* We need a check for read_blocked here because
           SSL_pending() doesn't work properly during the
           handshake. This check prevents a busy-wait
           loop around SSL_read() */
      } while (SSL_pending(ssl) && !read_blocked);
    }
  }
end:
  fclose(f);
  SSL_free(ssl);
  close(sock);
  return;
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
    char buf[BUFSIZZ] = {};
    read_crl_buffer(ssl, server);
    update_crl_file();
  }
  close(server);     /* close socket */
  SSL_CTX_free(ctx); /* release context */
  return 0;
}
