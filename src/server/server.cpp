#include "server.h"

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

//class Socket

Socket::Socket() {
    sd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sd_ == -1) {
        std::cout << "Can't create socket" << std::endl;
    }
}

//class ServerSocket

void ServerSocket::Bind(const SocketAdress &ipAddr) {
    if (bind(sd_, ipAddr.GetAddr(), ipAddr.GetAddrLen()) < 0) { // returns 0 if successful
        std::cout << "Error: Can't bind in ServerSocket" << std::endl;
    }
}

int ServerSocket::Accept(SocketAdress &ipAddr) {
    size_t ipAddrLen = ipAddr.GetAddrLen();
    size_t new_socket = accept(sd_, ipAddr.GetAddr(), (socklen_t *)&ipAddrLen);
    if (new_socket == -1) {
        std::cout << "Can't accept" << std::endl;
    }
    return new_socket;
}

void ServerSocket::Listen(int backlog) {
    int i = listen(sd_, backlog);
    if (i == -1) {
        std::cout << "Can't listen" << std::endl;
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
        std::cout << "Can't write in ConnectedSocket" << std::endl;
    }
}

void ConnectedSocket::Write(const std::vector<uint8_t> &bytes) {}

void ConnectedSocket::WorkFile(int fd) {
    std::string str;
    str += "\r\nContent-length: ";
    char c;
    int len = 0;

    while(read(fd, &c, 1)) len++;
    lseek(fd, 0, 0);
    str += std::to_string(len) + "\r\n\r\n";
    char* buf = (char*) malloc(sizeof(char) * (str.length() + 1));
    strcpy(buf, str.c_str());
    len = strlen(buf);
    send(sd_, buf, len, 0);
    free(buf);

    int buflen = 1024;
    char bufer[buflen];
    while((len = read(fd, bufer, buflen)) > 0){
        send(sd_, bufer, len, 0);
    }
}

void ConnectedSocket::Read(std::string& str) {
    int buflen = 1024;
    char buf[buflen];
    if (recv(sd_, buf, buflen, 0) < 0) { // or read(sd_, buf, buflen)
        std::cout << "Error: Can't read in ConnectedSocket" << std::endl;
    }
    str = buf;
}

void ConnectedSocket::Read(std::vector<uint8_t> &bytes) {}

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

std::string parse_path(std::string str) {
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

void ProcessConnection(int cd, const SocketAdress &clAddr) {
    ConnectedSocket cs(cd);
    std::string request;
    cs.Read(request);
    std::vector<std::string> lines = SplitLines(request);
    if (lines.size() > 0) {
        std::cout << lines[0] << std::endl;
    } else {
        std::cout << "Error: lines.size() <= 0 in ProcessConnection()" << std::endl;
    }
    std::string path = parse_path(lines[0]);
    path = "../" + path;
    std::cout << "Path: " << path << std::endl;

    // Process request:
    int fd = 0;

    if ((fd = open(path.c_str(), O_RDONLY, 666)) < 0) {
        std::cout << "HTTP/1.1 404 Not Found" << std::endl;
        cs.Write("HTTP/1.1 404 Not Found");
        if ((fd = open(ERROR_PAGE, O_RDONLY, 666)) < 0) {
            std::cout << "Error: Page 404 is missing" << fd << std::endl;
        }
    } else {
        cs.Write("HTTP/1.1 200 OK");
    }

    cs.WorkFile(fd);
    close(fd);
    cs.Shutdown();
}

void ServerLoop() {
    SocketAdress server_addr(BASE_ADDR, PORT);
    ServerSocket server_socket;
    server_socket.Bind(server_addr);
    std::cout << "The client was binded" << std::endl;
    server_socket.Listen(BACKLOG);
    for (;;) {
        SocketAdress client_addr;
        int cd = server_socket.Accept(client_addr);
        ProcessConnection(cd, client_addr);
    }
}

