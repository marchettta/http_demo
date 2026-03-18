import urllib.request
import json

def request(method, url, data=None):
    req = urllib.request.Request(url, method=method)
    if data:
        data_bytes = json.dumps(data).encode('utf-8')
        req.add_header('Content-Type', 'application/json')
        req.add_header('Content-Length', len(data_bytes))
        req.data = data_bytes
    try:
        with urllib.request.urlopen(req) as res:
            return res.read().decode('utf-8')
    except Exception as e:
        return str(e)

print("--- POST ---")
print(request("POST", "http://localhost:8080/alumnos", {"nombre":"Test", "curso":"1 DAM"}))

print("\n--- GET ALL ---")
print(request("GET", "http://localhost:8080/alumnos"))

print("\n--- PUT 1 ---")
print(request("PUT", "http://localhost:8080/alumnos/1", {"curso":"2 DAM"}))

print("\n--- GET 1 ---")
print(request("GET", "http://localhost:8080/alumnos/1"))

print("\n--- DELETE 1 ---")
print(request("DELETE", "http://localhost:8080/alumnos/1"))

print("\n--- GET ALL FINAL ---")
print(request("GET", "http://localhost:8080/alumnos"))
