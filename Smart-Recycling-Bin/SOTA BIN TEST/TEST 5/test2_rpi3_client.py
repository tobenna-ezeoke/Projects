import asyncio
import websockets
import json
import time

async def send_bin_full():
    uri = "ws://172.17.92.9:8766"  # Replace with RPi1's IP Address
    async with websockets.connect(uri) as websocket:
        message = json.dumps({"bin": "3", "status": "Full"})
        await websocket.send(message)
        print(f"Sent: {message}")

        response = await websocket.recv()
        print(f"✅ Acknowledgment received: {response}")

asyncio.run(send_bin_full())
