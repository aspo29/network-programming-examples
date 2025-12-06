import sys
import socket
import threading
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QLabel,
    QPushButton, QLineEdit, QTextEdit
)
from PyQt5.QtCore import Qt

BUFFER_SIZE = 8192


class ProxyServer:
    def __init__(self, log_callback):
        self.running = False
        self.log_callback = log_callback

    def log(self, text):
        self.log_callback(text)

    def start(self, port):
        if self.running:
            return

        self.running = True
        self.port = port

        def server_thread():
            try:
                self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                self.server.bind(("0.0.0.0", port))
                self.server.listen(50)
                self.log(f"[+] Proxy started on port {port}\n")

                while self.running:
                    client_socket, addr = self.server.accept()
                    self.log(f"[+] Connection from {addr}\n")
                    threading.Thread(target=self.handle_client, args=(client_socket,)).start()

            except Exception as e:
                self.log(f"[!] Error: {e}\n")

        threading.Thread(target=server_thread, daemon=True).start()

    def stop(self):
        self.running = False
        try:
            self.server.close()
        except:
            pass
        self.log("[×] Proxy stopped\n")

    def handle_client(self, client_socket):
        try:
            req = client_socket.recv(BUFFER_SIZE)
            if not req:
                client_socket.close()
                return

            first_line = req.split(b"\r\n")[0].decode()

            if first_line.startswith("CONNECT"):
                host = first_line.split()[1]
                host, port = host.split(":")
                port = int(port)

                self.log(f"[HTTPS] CONNECT {host}:{port}\n")

                remote = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                remote.connect((host, port))

                client_socket.send(b"HTTP/1.1 200 Connection Established\r\n\r\n")

                threading.Thread(target=self.pipe, args=(client_socket, remote)).start()
                threading.Thread(target=self.pipe, args=(remote, client_socket)).start()

            else:
                url = first_line.split()[1]
                host = url.split("/")[2]

                self.log(f"[HTTP] GET {url}\n")

                remote = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                remote.connect((host, 80))
                remote.send(req)

                while True:
                    data = remote.recv(BUFFER_SIZE)
                    if not data:
                        break
                    client_socket.send(data)

                remote.close()
                client_socket.close()

        except Exception as e:
            self.log(f"[!] Client error: {e}\n")
            client_socket.close()

    def pipe(self, src, dst):
        try:
            while self.running:
                data = src.recv(BUFFER_SIZE)
                if not data:
                    break
                dst.sendall(data)
        except:
            pass
        finally:
            src.close()
            dst.close()


# ---------------------------------------
#                UI (PyQt5)
# ---------------------------------------

class ProxyUI(QWidget):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("macOS Proxy (PyQt5)")
        self.setGeometry(200, 200, 600, 500)

        layout = QVBoxLayout()

        layout.addWidget(QLabel("Proxy Port:"))

        self.port_entry = QLineEdit()
        self.port_entry.setText("8888")
        layout.addWidget(self.port_entry)

        self.start_button = QPushButton("Start Proxy")
        self.start_button.clicked.connect(self.start_proxy)
        layout.addWidget(self.start_button)

        self.stop_button = QPushButton("Stop Proxy")
        self.stop_button.clicked.connect(self.stop_proxy)
        layout.addWidget(self.stop_button)

        layout.addWidget(QLabel("Logs:"))

        self.log_box = QTextEdit()
        self.log_box.setReadOnly(True)
        layout.addWidget(self.log_box)

        self.proxy = ProxyServer(self.add_log)

        self.setLayout(layout)

    def add_log(self, text):
        self.log_box.append(text)

    def start_proxy(self):
        port = int(self.port_entry.text())
        self.proxy.start(port)

    def stop_proxy(self):
        self.proxy.stop()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    ui = ProxyUI()
    ui.show()
    sys.exit(app.exec_())
