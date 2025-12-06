import socket
import threading

BUFFER_SIZE = 8192

def handle_https_tunnel(client_socket, address, host, port):
    # Connect to the real HTTPS server
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.connect((host, port))

    # Tell the client the tunnel is established
    client_socket.send(b"HTTP/1.1 200 Connection Established\r\n\r\n")

    # Two-way tunnel
    def forward(src, dst):
        while True:
            data = src.recv(BUFFER_SIZE)
            if not data:
                break
            dst.send(data)

    threading.Thread(target=forward, args=(client_socket, server)).start()
    threading.Thread(target=forward, args=(server, client_socket)).start()


def handle_client(client_socket, address):
    request = client_socket.recv(BUFFER_SIZE)

    if not request:
        client_socket.close()
        return

    first_line = request.split(b'\r\n')[0]

    # HTTPS CONNECT method
    if first_line.startswith(b"CONNECT"):
        try:
            target = first_line.split()[1].decode()  # host:port
            host, port = target.split(":")
            port = int(port)
            handle_https_tunnel(client_socket, address, host, port)
        except:
            client_socket.close()
        return

    # Normal HTTP GET proxying
    try:
        url = first_line.split()[1].decode()
        if url.startswith("http://"):
            url = url[7:]
        host, _, path = url.partition("/")
        path = "/" + path

        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.connect((host, 80))

        http = b"GET " + path.encode() + b" HTTP/1.0\r\nHost: " + host.encode() + b"\r\n\r\n"
        server.send(http)

        while True:
            data = server.recv(BUFFER_SIZE)
            if not data:
                break
            client_socket.send(data)

        server.close()
        client_socket.close()
    except:
        client_socket.close()


def start_proxy(port=8888):
    proxy = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    proxy.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    proxy.bind(("0.0.0.0", port))
    proxy.listen(20)
    print(f"Proxy running on port {port}")

    while True:
        client_socket, addr = proxy.accept()
        threading.Thread(target=handle_client, args=(client_socket, addr)).start()


if __name__ == "__main__":
    start_proxy(8888)

