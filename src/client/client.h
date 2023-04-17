#ifndef TCP_SERVER_CLIENT_H
#define TCP_SERVER_CLIENT_H

#include <cstddef>
#include <iostream>
#include <sys/fcntl.h>
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
    void Connect(const SocketAdress &serverAddr) ;
};

class HttpHeader {
    std::string name_;
    std::string value_;
public:
    HttpHeader() = default;
    HttpHeader(const std::string& n, const std::string& v) : name_(n), value_(v) {};
    HttpHeader(const HttpHeader& copy);
    std::string UnionString() const;
    static HttpHeader ParseHeader(const std::string& line);
};

class HttpRequest {
    std::vector<std::string> lines_;
public:
    HttpRequest();
    std::string UnionString() const;
};

class HttpResponse {
    HttpHeader response;
    HttpHeader* other;
    std::string body;
    int len;
public:
    HttpResponse(std::vector<std::string> lines);
    void Print() const;
    ~HttpResponse();
};

void ClientConnection();

#endif //TCP_SERVER_CLIENT_H
