import socket
import threading

BUFFER_SIZE = 4096

def handle_client(client_socket):
    request = client_socket.recv(BUFFER_SIZE)

    # Parse host from request
    try:
        first_line = request.split(b'\r\n')[0]
        url = first_line.split()[1].decode()
    except:
        client_socket.close()
        return

    if url.startswith("http://"):
        url = url[7:]

    host, _, path = url.partition("/")
    path = "/" + path

    # Connect to target server
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.connect((host, 80))

    # Forward the request to server
    server_request = b"GET " + path.encode() + b" HTTP/1.0\r\nHost: " + host.encode() + b"\r\n\r\n"
    server.send(server_request)

    # Receive server response and forward it back
    while True:
        data = server.recv(BUFFER_SIZE)
        if not data:
            break
        client_socket.send(data)

    server.close()
    client_socket.close()


def start_proxy(port=8888):
    proxy = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    proxy.bind(("0.0.0.0", port))
    proxy.listen(5)
    print(f"Proxy listening on port {port}...")

    while True:
        client_socket, addr = proxy.accept()
        print(f"Client connected: {addr}")
        thread = threading.Thread(target=handle_client, args=(client_socket,))
        thread.start()


if __name__ == "__main__":
    start_proxy(8888)
