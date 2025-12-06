# relay_server.py
# Relay server + static file server (aiohttp)
# Python 3.8+
import asyncio
import json
import uuid
from aiohttp import web, WSMsgType

# Configuration
WS_PORT = 8765
HTTP_PORT = 8000

# In-memory maps
clients = {}  # client_id -> websocket
names = {}    # client_id -> human name (optional)

async def websocket_handler(request):
    ws = web.WebSocketResponse()
    await ws.prepare(request)

    client_id = str(uuid.uuid4())[:8]
    clients[client_id] = ws
    names[client_id] = f"client-{client_id}"
    print(f"[+] Connected: {client_id} ({request.remote})")

    # send assigned id to client
    await ws.send_json({"type": "welcome", "id": client_id, "name": names[client_id]})

    try:
        async for msg in ws:
            if msg.type == WSMsgType.TEXT:
                try:
                    data = json.loads(msg.data)
                except:
                    data = {"type": "text", "text": msg.data}

                # Simple protocol:
                # { "type":"register", "name":"alice" }
                # { "type":"message", "to":"<id>" or "broadcast", "text":"..." }
                typ = data.get("type", "text")

                if typ == "register":
                    names[client_id] = data.get("name", names[client_id])
                    await ws.send_json({"type": "registered", "id": client_id, "name": names[client_id]})
                    print(f"[i] {client_id} set name -> {names[client_id]}")

                elif typ == "message":
                    to = data.get("to")
                    text = data.get("text", "")
                    payload = {
                        "type": "message",
                        "from": client_id,
                        "from_name": names.get(client_id),
                        "to": to,
                        "text": text
                    }

                    if to == "broadcast" or to is None:
                        # broadcast to all except sender
                        for cid, cws in clients.items():
                            if cid != client_id:
                                await cws.send_json(payload)
                    else:
                        target_ws = clients.get(to)
                        if target_ws:
                            await target_ws.send_json(payload)
                        else:
                            # send back error
                            await ws.send_json({"type":"error","message":"target_not_found","to":to})

                elif typ == "list":
                    # return list of connected clients (id + name)
                    arr = [{"id":cid, "name":names.get(cid)} for cid in clients.keys()]
                    await ws.send_json({"type":"list", "clients":arr})

                else:
                    # unknown type -> echo
                    await ws.send_json({"type":"echo","received":data})
            elif msg.type == WSMsgType.ERROR:
                print(f'ws connection closed with exception {ws.exception()}')
    finally:
        # cleanup
        clients.pop(client_id, None)
        names.pop(client_id, None)
        print(f"[-] Disconnected: {client_id}")
    return ws

# Serve a small web chat page (static files)
async def index(request):
    return web.FileResponse('./static/chat.html')

async def start_servers():
    app = web.Application()
    app.router.add_get('/ws', websocket_handler)
    app.router.add_get('/', index)
    # static folder (for css/js if any)
    app.router.add_static('/static/', path='./static/', name='static')
    runner = web.AppRunner(app)
    await runner.setup()
    site_ws = web.TCPSite(runner, '0.0.0.0', WS_PORT)
    site_http = web.TCPSite(runner, '0.0.0.0', HTTP_PORT)
    await site_ws.start()
    await site_http.start()
    print(f"WebSocket server running on ws://0.0.0.0:{WS_PORT}/ws")
    print(f"HTTP server running on http://0.0.0.0:{HTTP_PORT}/  (serves chat page)")

    # run forever
    while True:
        await asyncio.sleep(3600)

if __name__ == '__main__':
    try:
        asyncio.run(start_servers())
    except KeyboardInterrupt:
        print("Server stopped.")

