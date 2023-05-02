#include "server.h"
#include <sys/fcntl.h>
#include <sys/wait.h>

//class SocketAdress

SocketAdress::SocketAdress() {
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
}

SocketAdress::SocketAdress(const char *adr, short port) {
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(adr);
}

SocketAdress::SocketAdress(unsigned int ip, short port) {
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl(ip);
}

//class Socket

Socket::Socket() {
    sd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sd_ == -1) {
        std::cout << "Can't create socket" << std::endl;
    }
}

//class ServerSocket

void ServerSocket::Bind(const SocketAdress &ipAddr) {
    if (bind(sd_, ipAddr.GetAddr(), ipAddr.GetAddrLen()) < 0) {
        std::cout << "Error: Can't bind in ServerSocket" << std::endl;
        exit(1);
    }
}

int ServerSocket::Accept(SocketAdress &ipAddr) {
    size_t ipAddrLen = ipAddr.GetAddrLen();
    size_t new_socket = accept(sd_, ipAddr.GetAddr(), (socklen_t *) &ipAddrLen);
    if (new_socket == -1) {
        std::cout << "Can't accept" << std::endl;
        exit(1);
    }
    return new_socket;
}

void ServerSocket::Listen(int backlog) {
    int i = listen(sd_, backlog);
    if (i == -1) {
        std::cout << "Can't listen" << std::endl;
        exit(1);
    }
}

//class ConnectedSocket

void ConnectedSocket::Write(const std::string &str) {
    const int strlen = str.length();
    char buff[strlen];
    for (size_t i = 0; i < strlen; i++) {
        buff[i] = str[i];
    }
    if (send(sd_, buff, strlen, 0) < 0) {
        std::cout << "Error: Can't write in ConnectedSocket" << std::endl;
        exit(1);
    }
}

void ConnectedSocket::Write(const std::vector<uint8_t> &bytes) {
    if (send(sd_, bytes.data(), bytes.size(), 0) < 0) {
        std::cout << "Error: Can't write in ConnectedSocket" << std::endl;
        exit(1);
    }
}

void ConnectedSocket::WorkFile(int fd) {
    std::string str;
    str += "\r\nContent-length: ";
    char c;
    int len = 0;

    while (read(fd, &c, 1)) len++;
    lseek(fd, 0, 0);
    str += std::to_string(len) + "\r\n\r\n";
    char *buf = (char *) malloc(sizeof(char) * (str.length() + 1));
    strcpy(buf, str.c_str());
    len = strlen(buf);
    send(sd_, buf, len, 0);
    free(buf);

    int buflen = 1024;
    char bufer[buflen];
    while ((len = read(fd, bufer, buflen)) > 0) {
        send(sd_, bufer, len, 0);
    }
}

void ConnectedSocket::Read(std::string &str) {
    int buflen = 4096;
    char buf[buflen];
    if (recv(sd_, buf, buflen, 0) < 0) { // or read(sd_, buf, buflen)
        std::cout << "Error: Can't read in ConnectedSocket" << std::endl;
        exit(1);
    }
    str = buf;
}

std::vector<std::string> SplitLines(const std::string &str) {
    std::string delimiter = "\r\n";
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = str.find(delimiter, pos_start)) != -1) {
        token = str.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }
    res.push_back(str.substr(pos_start));
    return res;
}

std::string ParsePath(std::string str) {
    std::string res_path = "src/";
    for (auto i = 0; i < str.length() - 1; ++i) {
        if (str[i] == ' ') {
            while (str[i + 1] != ' ' && str[i + 1] != '\n') {
                res_path += str[i + 1];
                i++;
            }
            break;
        }
    }
    if (res_path == "src//") {
        res_path = "src/index.html";
    }
    return res_path;
}

std::vector<uint8_t> ToVector(int fd) {
    std::vector<uint8_t> v;
    char c;
    while (read(fd, &c, 1)) v.push_back(c);
    return v;
}

std::string GetFileName(std::string path) {
    std::string temp;
    int i = 0;
    while (path[i] != '?') {
        temp += path[i];
        i++;
    }
    return temp;
}

std::string GetQuery(std::string path) {
    std::string temp;
    int i = GetFileName(path).length() + 1;
    while (i != path.length()) {
        temp += path[i];
        i++;
    }
    return temp;
}

char **CreateArray(std::vector<std::string> &v) {
    char **env = new char *[v.size() + 1];
    for (auto i = 0; i < v.size(); ++i) {
        env[i] = (char *) v[i].c_str();
    }
    env[v.size()] = NULL;
    return env;
}

void CheckError(ConnectedSocket cs) {
    std::string error_str;
    switch (errno) {
        case EACCES: // permission denied
            error_str = "HTTP/1.1 403 Forbidden\n";
            break;
        case ENETRESET: // connection aborted by network
            error_str = "HTTP/1.1 503 Service Unavailable\n";
            break;
        default:
            error_str = "HTTP/1.1 404 Not Found\n";
            break;
    }
    std::cout << error_str << std::endl;
    cs.Write(error_str);
}

bool IsCgiCinnection(std::string str) {
    return !(str.find('?') == -1); // true - if the '?' character is found
}

void CgiConnection(std::string path, int cd, const SocketAdress &client_addr, ConnectedSocket cs, std::string request) {
    int fd;
    pid_t pid = fork();
    switch (pid) {
        case -1: {
            perror("System error with pid");
            exit(1);
        }
        case 0: {
            fd = open("log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                std::cout << "Error: Can't open a new file" << std::endl;
            }
            dup2(fd, 1);
            close(fd);
            std::string file_name = GetFileName(path);
            std::string query = GetQuery(path);
            char *argv[] = {(char *) file_name.c_str(), NULL};

            std::vector<std::string> v;
            v.push_back(request);
            v.push_back(SERVER_ADDR);
            v.push_back(SERVER_PORT);
            v.push_back(SERVER_PROTOCOL);
            v.push_back(CONTENT_TYPE);
            v.push_back(QUERY_STRING + query);
            v.push_back(file_name);

            char **env = CreateArray(v);

            execve(file_name.c_str(), argv, env);
            CheckError(cs);
            perror("exec");
            exit(2);
        }
        default: { // case > 0
            int status;
            wait(&status);

            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                fd = open("log", O_RDONLY);
                std::vector<uint8_t> vect = ToVector(fd);
                cs.Write("HTTP/1.1 200 OK\0");
                std::cout << "HTTP/1.1 200 OK" << std::endl;
                std::string str =
                        "\r\nVersion: HTTP/1.1\r\nContent-length: " + std::to_string(vect.size()) + "\r\n\r\n";
                std::cout << "Version: " << "HTTP/1.1" << std::endl;
                std::cout << "Content-length: " << std::to_string(vect.size()) << std::endl;

                cs.Write(str);
                cs.Write(vect);
                close(fd);
                cs.Shutdown();
            }
            break;
        }
    }
}

void DefaultConnection(std::string path, ConnectedSocket cs) {
    int fd = 0;
    if ((fd = open(path.c_str(), O_RDONLY)) < 0) {
        std::cout << "HTTP/1.1 404 Not Found" << std::endl;
        cs.Write("HTTP/1.1 404 Not Found\r");
        if ((fd = open(ERROR_PAGE, O_RDONLY)) < 0) {
            std::cout << "Error: Page 404 is missing" << std::endl;
        }
    } else {
        cs.Write("HTTP/1.1 200 OK\0");
    }
    std::vector<uint8_t> vect = ToVector(fd);
    std::string str = "\r\nVersion: HTTP/1.1\r\nContent-length: " + std::to_string(vect.size()) + "\r\n\r\n";

    std::cout << "Version: " << "HTTP/1.1" << std::endl;
    std::cout << "Content-length: " << std::to_string(vect.size()) << std::endl;

    cs.Write(str);
    cs.Write(vect);
    close(fd);
    cs.Shutdown();
}

void ProcessConnection(int cd, const SocketAdress &client_addr) {
    ConnectedSocket cs(cd);
    std::string request;
    cs.Read(request);
    std::vector<std::string> lines = SplitLines(request);
    if (lines.size() > 0) {
        std::cout << lines[0] << std::endl;
    } else {
        std::cout << "Error: lines.size() <= 0 in ProcessConnection()" << std::endl;
    }

    std::string path = ParsePath(lines[0]);
    std::cout << "Path: " << path << std::endl;

    if (IsCgiCinnection(path)) {
        CgiConnection(path, cd, client_addr, cs);
    } else {
        DefaultConnection(path, cs);
    }
}

void ServerLoop() {
    SocketAdress server_address(BASE_ADDR, PORT);
    ServerSocket server_socket;
    server_socket.Bind(server_address); // bind to an address - what port am I on?
    std::cout << "The client was successfully binded" << std::endl;
    server_socket.Listen(BACKLOG); // listen on a port, and wait for a connection to be established
    for (;;) {
        SocketAdress client_addr;
        int cd = server_socket.Accept(client_addr);
        ProcessConnection(cd, client_addr); // process cilent-server connection
        std::cout << std::endl;
    }
}

