const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 8080 });

console.log("WebSocket server running on ws://localhost:8080");

wss.on('connection', function connection(ws) {
    console.log("New client connected");

    ws.send("Welcome to the WebSocket server!");

    ws.on('message', function incoming(message) {
        console.log('Received:', message);

        // Broadcast to all clients
        wss.clients.forEach(function each(client) {
            if (client !== ws && client.readyState === WebSocket.OPEN) {
                client.send(`Client says: ${message}`);
            }
        });
    });

    ws.on('close', () => {
        console.log("Client disconnected");
    });
});
