import asyncio
import json
import websockets

# Define WebSocket server handler
async def handle_message(websocket):
    try:
        data = await websocket.recv()  # Receive message from client
        print("Received raw data:", data)  # Debugging output

        # Try to parse JSON data
        try:
            data = json.loads(data)
        except json.JSONDecodeError:
            print("Error: Received invalid JSON format")
            await websocket.send(json.dumps({"error": "Invalid JSON format"}))
            return

        # Ensure "type" key exists
        if "type" not in data:
            print("Error: Missing 'type' key in received data")
            await websocket.send(json.dumps({"error": "Missing 'type' key"}))
            return

        # Process message
        if data["type"] == "plastic":
            print("Processing: Plastic waste detected")
        elif data["type"] == "metal":
            print("Processing: Metal waste detected")
        else:
            print(f"Unknown type: {data['type']}")

        # Send acknowledgment back to client
        await websocket.send(json.dumps({"status": "success", "received_type": data["type"]}))

    except Exception as e:
        print(f"Unexpected error: {e}")

# Start WebSocket server
async def main():
    async with websockets.serve(handle_message, "0.0.0.0", 8765):
        print("WebSocket server running on ws://0.0.0.0:8765")
        await asyncio.Future()  # Keeps server running indefinitely

# Run the WebSocket server
if __name__ == "__main__":
    asyncio.run(main())
