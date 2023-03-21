#include <iostream>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#define PORT 8000

class SocketAdress {

    struct sockaddr_in saddr{};
public:
    SocketAdress() = default;

    SocketAdress(const char *adr, short port) {
        saddr.sin_family = AF_INET;
        inet_pton(AF_INET, adr, &saddr.sin_addr);
        saddr.sin_port = htons(port);
    }
};

class Socket {
protected:
    int sd_;

    explicit Socket(int sd) : sd_(sd) {}

public:
    Socket() {
        sd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sd_ == -1) {
            std::cout << "Can't create socket" << std::endl;
        }
        int opt = 1;
        int i = setsockopt(sd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
        if (i == -1) {
            std::cout << "Can't setsockopt" << std::endl;
        }
    }

    void Shutdown() {
        shutdown(sd_, SHUT_RDWR);
    }

    ~Socket() {
        close(sd_);
    }
};

class ServerSocket : public Socket {
public:
    void Bind(const sockaddr_in &ipAddr) {
        int i = bind(sd_, (struct sockaddr*)&ipAddr, sizeof(ipAddr));
        if (i == -1) {
            std::cout << "Can't bind" << std::endl;
        }
    }

    int Accept(sockaddr_in &ipAddr) {
        int new_socket;
        int ipAddrLen = sizeof(ipAddr);
        new_socket = accept(sd_, (struct sockaddr*)&ipAddr, (socklen_t *)&ipAddrLen);
        if (new_socket == -1) {
            std::cout << "Can't accept" << std::endl;
        }
        return new_socket;
    }

    void Listen(int backlog) {
        int i = listen(sd_, backlog);
        if (i == -1) {
            std::cout << "Can't listen" << std::endl;
        }
    }
};

class ConnectedSocket : public Socket {
public:
    ConnectedSocket() = default;

    explicit ConnectedSocket(int sd) : Socket(sd) {}

    void Write(const std::string &str) {
        send(sd_, str.c_str(), str.size(), 0);
    }

    void Write(const std::vector<uint8_t> &bytes) {
        send(sd_, bytes.data(), bytes.size(), 0);
    }

    void Read(std::string &str) {
        read(sd_, (char *) str.c_str(), 1024);
    }

    void Read(std::vector<uint8_t> &bytes) {
        read(sd_, bytes.data(), 1024);
    }
};

class ClientSocket : public ConnectedSocket {
public:
    void Connect(const sockaddr_in &serverAddr) {
        int i = connect(sd_, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
        if (i == -1) {
            std::cout << "Can't connect" << std::endl;
        }
    }
};

const int BACKLOG = 5;

std::vector<std::string> SplitLines(const std::string &str) {
    std::vector<std::string> res;
    int j = 0, k = 0;
    for (char i: str) {
        if (i == '\n') {
            k = 0;
            j++;
        } else {
            res[j][k++] = i;
        }
    }
    return res;
}

std::string JoinLines(const std::vector<std::string> &lines) {
    std::string res;
    int j = 0;
    for (const auto &line: lines) {
        for (char k: line) {
            res[j++] = k;
        }
        res[j++] = '\n';
    }
    return res;
}

void ProcessConnection(int cd, const sockaddr_in &clAddr) {
    ConnectedSocket cs(cd);
    std::string request;
    cs.Read(request);
    std::vector<std::string> lines = SplitLines(request);
    for (int i = 0; i < lines.size(); i++) {
        std::cout << "Server: " << lines[i] << std::endl;
        lines[i][0] = '@';
    }
    std::string resp;
    resp = JoinLines(lines);
    cs.Write(resp);
    cs.Shutdown();
}

void ServerLoop() {
    struct sockaddr_in saddr{};
    saddr.sin_family = AF_INET;
    std::cout << 5555;
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr);
    saddr.sin_port = htons(PORT);
    ServerSocket ss;
    std::cout << 3333;
    ss.Bind(saddr);
    std::cout << 1111;
    ss.Listen(BACKLOG);
    std::cout << 2222;
    for (;;) {
        struct sockaddr_in clAddr{};

        int cd = ss.Accept(clAddr);

        ProcessConnection(cd, clAddr);
    }
}

void ClientConnection() {
    ClientSocket s;
    struct sockaddr_in saddr{};
    saddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr);
    saddr.sin_port = htons(PORT);
    s.Connect(saddr);
    std::string strRq = "qweqr \nasfdasf \nsdgd";
    s.Write(strRq);
    std::string strRes;
    s.Read(strRes);
    std::cout << "Client: " << strRes << std::endl;
    std::vector<std::string> lines = SplitLines(strRes);
}

int main() {
    std::cout << "Server or Client 1/2: ";
    int t = 0;
    std::cin >> t;
    if (t == 1) {
        ServerLoop();
    } else if (t == 2) {
        ClientConnection();
    }
}
