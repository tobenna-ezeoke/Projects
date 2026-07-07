

import asyncio
import websockets
import json

async def handle_message(websocket):
    async for message in websocket:
        data = json.loads(message)
        print(f"Received bin full alert: {data}")

        # Notify user via LED Display (Simulated)
        print(f"⚠️ Bin {data['bin']} is full. Redirecting waste...")

        # Send acknowledgment back to RPi3
        response = json.dumps({"status": "received", "bin": data["bin"]})
        await websocket.send(response)

async def main(): 
	async with websockets.serve(handle_message, "0.0.0.0", 8780): 
		print("WebSocket server started on ws://0.0.0.0:8780") 
		await asyncio.Future() # Run forever 

if __name__ == "__main__": 
	asyncio.run(main())
