import asyncio
import websockets
from aioconsole import ainput
import sys

async def send(websocket):
    while True:
        message = await ainput("You: ")
        await websocket.send(message)

async def receive(websocket):
    while True:
        try:
            response = await websocket.recv()
            print(f"\nReceived: {response}")
            sys.stdout.write("You: ")
            sys.stdout.flush()
        except websockets.exceptions.ConnectionClosed:
            print("\nDisconnected from server.")
            break

async def chat():
    uri = "ws://localhost:8765"
    try:
        async with websockets.connect(uri) as websocket:
            print("Connected to server. Type your messages below.")
            await asyncio.gather(send(websocket), receive(websocket))
    except asyncio.CancelledError:
        print("\nClient shutting down gracefully.")
    except KeyboardInterrupt:
        print("\nClient stopped by user.")

if __name__ == "__main__":
    try:
        asyncio.run(chat())
    except KeyboardInterrupt:
        print("\nClient stopped by user.")
