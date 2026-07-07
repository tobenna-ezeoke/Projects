
import asyncio
import websockets
import json

async def send_classification():
    uri = "ws://172.17.162.196:8765"  # Replace with RPi2's IP
    async with websockets.connect(uri) as websocket:
        message = json.dumps({"type": "plastic", "bin": "2"})
        await websocket.send(message)
        print(f"Sent: {message}")

        response = await websocket.recv()
        print(f"Received: {response}")

asyncio.run(send_classification())

