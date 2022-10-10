#include <iostream>
#include <unistd.h>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

int main(){
    int socket_desc;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[4096];

    std::string host = "10.169.37.248";
    std::string port = "8200";
    std::string resource = "v1/pki_int_D8C0F20133/crl";
    std::string query = "";

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0){
        std::cout << "failed to create socket" << std::endl;
        return 0;
    }

    server = gethostbyname(host.c_str());
    if (server == NULL){
        std::cout << "could Not resolve hostname :(" << std::endl;
        close(socket_desc);
        return 0;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(std::stoi(port));
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    if (connect(socket_desc, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        std::cout << "connection failed :(" << std::endl;
        close(socket_desc);
        return 0;
    }

    std::string request = "GET /v1/pki_int_D8C0F20133/crl HTTP/1.1\r\nHost: https://" + host+":"+ port + "\r\nConnection: close\r\n\r\n";
    if (send(socket_desc, request.c_str(), request.size(), 0) < 0){
        std::cout << "failed to send request..." << std::endl;
        close(socket_desc);
        return 0;
    }

    int n;
    std::string raw_site;
    while ((n = recv(socket_desc, buffer, sizeof(buffer), 0)) > 0){
        raw_site.append(buffer, n);
    }

    close(socket_desc);

    std::cout << raw_site << std::endl;
    return 0;
}
