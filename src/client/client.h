#ifndef TCP_SERVER_CLIENT_H
#define TCP_SERVER_CLIENT_H

#include <cstddef>
#include <iostream>
#include <sys/fcntl.h>
#include <utility>
#include <vector>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include "../server/server.h"

class ClientSocket : public ConnectedSocket {
public:
    void Connect(const SocketAdress &serverAddr);
};

class HttpHeader {
    std::string name;
    std::string value;
public:
    HttpHeader() = default;

    HttpHeader(std::string n, std::string v) : name(std::move(n)), value(std::move(v)) {};

    HttpHeader(const HttpHeader &copy);

    std::string UnionString() const;

    static HttpHeader ParseHeader(const std::string &line);
};

class HttpRequest {
    std::vector<std::string> lines_;
public:
    HttpRequest();

    std::string UnionString() const;
};

class HttpResponse {
    HttpHeader response;
    HttpHeader *other;
    std::string body;
    int len;
public:
    HttpResponse(std::vector<std::string> lines);

    ~HttpResponse();

    void Print() const;
};

void ClientConnection();

#endif //TCP_SERVER_CLIENT_H
