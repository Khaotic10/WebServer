#ifndef TCP_SERVER_SERVER_H
#define TCP_SERVER_SERVER_H

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
#include <cstring>


#define BASE_ADDR       "127.0.0.1"
#define ERROR_PAGE      "src/404.html"

#define CLEAR_SCREEN    "\033[2J\033[1;1H"
#define RESET_COLOR     "\033[0m"
#define CYAN_COLOR      "\x1b[36m"

#define CONTENT_TYPE    "CONTENT_TYPE=text/plain"
#define REMOTE_ADDR     "REMOTE_ADDR=127.0.0.5"
#define REMOTE_PORT     "REMOTE_PORT=8845"
#define QUERY_STRING    "QUERY_STRING="
#define SERVER_ADDR     "SERVER_ADDR=127.0.0.1"
#define SERVER_NAME     "www.primarysite.com"
#define SERVER_PORT     "SERVER_PORT=8080"
#define SERVER_PROTOCOL "SERVER_PROTOCOL=HTTP/1.0"
#define SCRIPT_NAME     "SCRIPT_NAME="

const int BACKLOG = 5;
const int PORT = 8092;

class SocketAdress {
    struct sockaddr_in saddr{};
public:
    SocketAdress();

    SocketAdress(const char *adr, short port);

    SocketAdress(unsigned int ip, short port);

    struct sockaddr *GetAddr() const { return (sockaddr *) &saddr; }

    int GetAddrLen() const { return sizeof(saddr); }
};

class Socket {
protected:
    int sd_;

    explicit Socket(int sd) : sd_(sd) {}

public:
    Socket();

    void Shutdown() { shutdown(sd_, SHUT_RDWR); }

    ~Socket() { close(sd_); }
};

class ServerSocket : public Socket {
public:
    void Bind(const SocketAdress &ipAddr);

    int Accept(SocketAdress &ipAddr);

    void Listen(int backlog);
};

class ConnectedSocket : public Socket {
public:
    ConnectedSocket() = default;

    explicit ConnectedSocket(int sd) : Socket(sd) {}

    void Write(const std::string &str);

    void Write(const std::vector<uint8_t> &bytes);

    void WorkFile(int fd);

    void Read(std::string &str);

    void Read(std::vector<uint8_t> &bytes);
};

void ServerLoop();

void ProcessConnection(int, const SocketAdress&);

std::vector<std::string> SplitLines(const std::string&);

std::string ParsePath(std::string);

std::vector<uint8_t> ToVector(int);

std::string GetFileName(std::string);

std::string GetQuery(std::string);

std::string GetPath(std::string);

char **CteareArray(std::vector<std::string>);

bool IsCgiCinnection(std::string);

void CgiConnection(std::string, int, const SocketAdress, ConnectedSocket);

void DefaultConnection(std::string, ConnectedSocket);

void CheckError(ConnectedSocket cs);

void ServerLoop();

#endif //TCP_SERVER_SERVER_H
