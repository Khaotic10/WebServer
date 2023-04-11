#include "client.h"

//class ClientSocket

void ClientSocket::Connect(const SocketAdress &serverAddr) {
    int i = connect(sd_, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    if (i == -1) {
        std::cout << "Can't connect" << std::endl;
    }
}

//class HttpHeader

HttpHeader::HttpHeader(const HttpHeader &copy) {
    name_ = copy.name_;
    value_ = copy.value_;
}

std::string HttpHeader::UnionString() const {
    std::string temp = name_ + " " + value_;
    return temp;
}

HttpHeader HttpHeader::ParseHeader(const std::string &line) {
    int i = 0;
    std::string new_name, new_value;
    if (!line.empty()) {
        while (line[i] != ' ') {
            new_name += line[i];
            i++;
        }
        new_name += '\0';

        while (i < line.size()) {
            new_value += line[i];
            i++;
        }
        new_value += '\0';
    } else {
        new_name = " ";
        new_value = " ";
    }

    HttpHeader temp(new_name, new_value);
    return temp;
}

//class HttpRequest

HttpRequest::HttpRequest() {
    lines_ = {"GET / HTTP/1.1"};
}

std::string HttpRequest::UnionString() const {
    std::string tmp;
    for (size_t i = 0; i < lines_.size(); i++) {
        tmp += lines_[i];
    }
    return tmp;
}

//class HttpResponse

HttpResponse::HttpResponse(std::vector<std::string> lines) {
    response = HttpHeader::ParseHeader(lines[0]);
    other = new HttpHeader[lines.size() - 1];
    size_t i;
    for (i = 1; i < lines.size(); i++) {
        other[i - 1] = HttpHeader::ParseHeader(lines[i]);
        if ((lines[i]).empty()) {
            body = lines[i + 1];
            break;
        }
    }
    len = (int) i;
}

void HttpResponse::Print() const {
    std::cout << "'" << response.UnionString() << "'" << std::endl;
    for (size_t j = 0; j < len; j++) {
        std::cout << (other[j]).UnionString() << std::endl;
    }
    std::cout << body << std::endl;
}

HttpResponse::~HttpResponse() {
    delete[] other;
}

void ClientConnection() {
    ClientSocket s;
    SocketAdress saddr{BASE_ADDR, PORT};
    s.Connect(saddr);
    HttpRequest rq;
    std::string req = rq.UnionString();
    std::cout << "'req = " << req  << "'" << std::endl;
    s.Write(req);
    std::vector<std::string> lines;
    std::string str_responce;
    std::string tmp;

    for (auto i = 0; i < 3; ++i) {
        s.Read(str_responce);
        tmp += str_responce;
    }

    lines = SplitLines(tmp);
    HttpResponse resp(lines);
    resp.Print();
    s.Shutdown();
}