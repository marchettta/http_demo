#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "../include/json.hpp"

using json = nlohmann::json;

class Database {
private:
    std::string filename; json data;
    void load() { std::ifstream f(filename); if (f.is_open()) f >> data; else data = json::array(); }
    void save() { std::ofstream f(filename); if (f.is_open()) f << data.dump(4); }
public:
    Database(const std::string& f) : filename(f) { load(); }
    json getAll() { return data; }
    json get(int id) { for (auto& i : data) if (i["id"] == id) return i; return nullptr; }
    json create(json j) { int id = 1; for (auto& i : data) if (i["id"] >= id) id = i["id"].get<int>() + 1; j["id"] = id; data.push_back(j); save(); return j; }
    bool update(int id, json j) { for (auto& i : data) if (i["id"] == id) { if (j.contains("nombre")) i["nombre"] = j["nombre"]; if (j.contains("curso")) i["curso"] = j["curso"]; save(); return true; } return false; }
    bool remove(int id) { for (auto it = data.begin(); it != data.end(); ++it) if ((*it)["id"] == id) { data.erase(it); save(); return true; } return false; }
};

void sendRes(int sock, int code, const std::string& status, const json& body) {
    std::string sBody = body.dump(4);
    std::ostringstream oss;
    oss << "HTTP/1.1 " << code << " " << status << "\r\nContent-Type: application/json\r\nContent-Length: " << sBody.length() << "\r\nConnection: close\r\n\r\n" << sBody;
    std::string res = oss.str(); 
    send(sock, res.c_str(), res.length(), 0);
    std::cout << ">>> RESPUESTA ENVIADA: " << code << " " << status << std::endl << std::endl;
}

int main() {
    int s_fd = socket(AF_INET, SOCK_STREAM, 0); int opt = 1; 
    setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr; addr.sin_family = AF_INET; addr.sin_addr.s_addr = INADDR_ANY; addr.sin_port = htons(8080);
    
    if (bind(s_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Error en bind");
        return 1;
    }
    listen(s_fd, 5); 
    Database db("alumnos.json");

    std::cout << "============================================" << std::endl;
    std::cout << "   SERVIDOR HTTP EDUCATIVO (CRUD Alumnos)   " << std::endl;
    std::cout << "   Escuchando en: http://localhost:8080     " << std::endl;
    std::cout << "============================================" << std::endl << std::endl;

    while (true) {
        int ns = accept(s_fd, NULL, NULL); if (ns < 0) continue;
        std::string request; char buf[4096]; int bytes;
        
        // Lectura robusta del socket
        while ((bytes = read(ns, buf, 4095)) > 0) {
            buf[bytes] = '\0'; request += buf;
            if (request.find("\r\n\r\n") != std::string::npos) {
                size_t clPos = request.find("Content-Length: ");
                if (clPos != std::string::npos) {
                    int cl = std::stoi(request.substr(clPos + 16));
                    size_t headerEnd = request.find("\r\n\r\n") + 4;
                    while (request.length() < headerEnd + cl) {
                        bytes = read(ns, buf, 4095);
                        if (bytes <= 0) break;
                        buf[bytes] = '\0'; request += buf;
                    }
                }
                break;
            }
        }

        if (!request.empty()) {
            std::cout << "---------- PETICIÓN RECIBIDA ----------" << std::endl;
            std::cout << request << std::endl;
            std::cout << "---------------------------------------" << std::endl;

            std::istringstream iss(request); 
            std::string method, path, proto; 
            iss >> method >> path >> proto;

            size_t bPos = request.find("\r\n\r\n");
            std::string bodyStr = (bPos != std::string::npos) ? request.substr(bPos + 4) : "";

            if (path == "/alumnos" || path == "/alumnos/") {
                if (method == "GET") {
                    std::cout << "[INFO] Listando todos los alumnos..." << std::endl;
                    sendRes(ns, 200, "OK", db.getAll());
                }
                else if (method == "POST") {
                    try { 
                        json j = json::parse(bodyStr);
                        std::cout << "[INFO] Creando nuevo alumno: " << j.dump() << std::endl;
                        sendRes(ns, 201, "Created", db.create(j)); 
                    } catch (...) { 
                        std::cout << "[ERROR] JSON inválido en POST" << std::endl;
                        sendRes(ns, 400, "Bad Request", {{"error","JSON invalido"}}); 
                    }
                }
            } else if (path.rfind("/alumnos/", 0) == 0) {
                try {
                    int id = std::stoi(path.substr(9));
                    if (method == "GET") { 
                        std::cout << "[INFO] Buscando alumno ID: " << id << std::endl;
                        json j = db.get(id); 
                        if (j != nullptr) sendRes(ns, 200, "OK", j); 
                        else sendRes(ns, 404, "Not Found", {{"error","No existe"}}); 
                    }
                    else if (method == "PUT") { 
                        try { 
                            json j = json::parse(bodyStr);
                            std::cout << "[INFO] Actualizando alumno ID " << id << " con: " << j.dump() << std::endl;
                            if (db.update(id, j)) sendRes(ns, 200, "OK", db.get(id)); 
                            else sendRes(ns, 404, "Not Found", {{"error","No existe"}}); 
                        } catch(...){ sendRes(ns, 400, "Bad Request", {{"error","JSON invalido"}}); }
                    }
                    else if (method == "DELETE") { 
                        std::cout << "[INFO] Eliminando alumno ID: " << id << std::endl;
                        if (db.remove(id)) sendRes(ns, 200, "OK", {{"status","Borrado"}}); 
                        else sendRes(ns, 404, "Not Found", {{"error","No existe"}}); 
                    }
                } catch (...) { 
                    sendRes(ns, 400, "Bad Request", {{"error","ID invalido"}}); 
                }
            } else {
                std::cout << "[WARN] Ruta no encontrada: " << path << std::endl;
                sendRes(ns, 404, "Not Found", {{"error","Ruta no encontrada"}});
            }
        }
        close(ns);
    }
    return 0;
}
