#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <fstream>

const int PORT = 8080;
const std::string WEB_ROOT = "html_files";

void send_response(int client_socket, const std::string& response) {
    send(client_socket, response.c_str(), response.length(), 0);
    // what will happen if I won't close the client_socket 
    close(client_socket);
}

void handle_request(int client_socket, const std::string& request_path) {
    std::string file_path = WEB_ROOT + request_path;
    
    if (file_path.back() == '/') {
        file_path += "index.html";
    }

    std::ifstream file(file_path);
    std::ostringstream response;

    if (!file.is_open()) {
        // File not found, send a 404 error
        response << "HTTP/1.1 404 Not Found";
        file_path = WEB_ROOT + "/not-found.html";
        std::ifstream file(file_path);
        std::ostringstream file_content;
        file_content << file.rdbuf();
        response << "\r\nContent-Type: text/html\r\n\r\n"
                 << file_content.str();
        send_response(client_socket, response.str());
        return;
        
    } else {
        response << "HTTP/1.1 200 OK";
    }

    // Reading the file now 
    std::ostringstream file_content;
    file_content << file.rdbuf();

    response << "\r\nContent-Type: text/html\r\n\r\n"
             << file_content.str();

    send_response(client_socket, response.str());
}

// void handle_client(int client_socket, const std::string &request_path) {
//     std::ostringstream response;

//     if (request_path == "/") {
//         response << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n<h1>Hello ,World!</h1>";
//     } else if (request_path == "/about") {
//         response << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n<h1>About Us Page</h1>";
//     } else if (request_path == "/contact") {
//         response << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n<h1>Contact Us Page</h1>";
//     } else {
//         response << "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n<h1>Page not Found</h1>";
//     }
//     std::cout << "A Client Connected\n";
//     send(client_socket, response.str().c_str(), response.str().length(), 0);
//     close(client_socket);
// }

int main(int argc, char const *argv[])
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_address{};

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Error Binding Server Socket \n";
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, 10) == -1) {
        std::cerr << "Error listening on socket\n";
        close(server_socket);
        return -1;
    }

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        sockaddr_in client_address{};
        socklen_t client_address_size = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_size);
        if (client_socket == -1) {
            std::cerr << "Error accepting connection\n";
        }

        // Read the HTTP Request
        char buffer[1024];
        recv(client_socket, buffer, sizeof(buffer), 0);

        // Parse the request to get the path
        std::istringstream request_stream(buffer);
        std::string request_line;
        std::getline(request_stream, request_line);
        std::cerr << request_line << "\n";
        std::istringstream request_line_stream(request_line);
        std::string method, path, protocol;
        request_line_stream >> method >> path >> protocol;
        std::cout << method << " " << path << " " << protocol << "\n";

        handle_request(client_socket, path);
    }

    close(server_socket);
    return 0;
}
