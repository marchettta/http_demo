#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main() {
    int port = 8080;
    std::cout << "=== CLIENTE HTTP CRUD ===\n";
    while (true) {
        std::cout << "ENTER para continuar, 'q' para salir: ";
        std::string choice; std::getline(std::cin, choice);
        if (choice == "q") break;

        std::string method, path, body;
        std::cout << "Metodo (GET, POST, PUT, DELETE): "; std::getline(std::cin, method);
        std::cout << "Ruta (/alumnos, /alumnos/1): "; std::getline(std::cin, path);
        std::cout << "JSON Body (opcional): "; std::getline(std::cin, body);

        std::string request = method + " " + path + " HTTP/1.1\r\n";
        request += "Host: localhost\r\n";
        if (!body.empty()) {
            request += "Content-Type: application/json\r\n";
            request += "Content-Length: " + std::to_string(body.length()) + "\r\n";
        }
        request += "\r\n" + body;

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0) {
            send(sock, request.c_str(), request.length(), 0);
            char buffer[8192] = {0};
            int val = read(sock, buffer, 8191);
            if (val > 0) std::cout << "\n--- RESPUESTA ---\n" << buffer << "\n-----------------\n\n";
        } else {
            std::cerr << "No se pudo conectar al servidor.\n";
        }
        close(sock);
    }
    return 0;
}
