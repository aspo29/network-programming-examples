import socket
import threading
import struct

BUFFER_SIZE = 8192

def recv_exact(sock, size):
    data = b""
    while len(data) < size:
        chunk = sock.recv(size - len(data))
        if not chunk:
            raise ConnectionError("socket closed")
        data += chunk
    return data


def handle_client(client):
    try:
        # --- handshake ---
        header = recv_exact(client, 2)
        ver, nmethods = header[0], header[1]
        if ver != 5:
            client.close()
            return

        methods = recv_exact(client, nmethods)

        # reply: version 5, no auth (00)
        client.sendall(b"\x05\x00")

        # --- request ---
        req = recv_exact(client, 4)
        ver, cmd, _, atyp = req

        if atyp == 1:  # IPv4
            addr = socket.inet_ntoa(recv_exact(client, 4))
        elif atyp == 3:  # domain
            length = recv_exact(client, 1)[0]
            addr = recv_exact(client, length).decode()
        else:
            client.close()
            return

        port = struct.unpack(">H", recv_exact(client, 2))[0]

        if cmd != 1:  # only CONNECT
            client.close()
            return

        # --- connect to target ---
        remote = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        remote.connect((addr, port))

        # reply success (bind addr = 0.0.0.0:0)
        client.sendall(
            b"\x05\x00\x00\x01" + socket.inet_aton("0.0.0.0") + struct.pack(">H", 0)
        )

        # --- bidirectional forwarding ---
        def forward(src, dst):
            try:
                while True:
                    data = src.recv(BUFFER_SIZE)
                    if not data:
                        break
                    dst.sendall(data)
            except:
                pass
            finally:
                src.close()
                dst.close()

        threading.Thread(target=forward, args=(client, remote), daemon=True).start()
        threading.Thread(target=forward, args=(remote, client), daemon=True).start()

    except Exception as e:
        client.close()


def start_socks(port=1080):
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("0.0.0.0", port))
    server.listen(50)
    print(f"SOCKS5 proxy running on port {port}")

    while True:
        client, addr = server.accept()
        threading.Thread(target=handle_client, args=(client,), daemon=True).start()


if __name__ == "__main__":
    start_socks(1080)
