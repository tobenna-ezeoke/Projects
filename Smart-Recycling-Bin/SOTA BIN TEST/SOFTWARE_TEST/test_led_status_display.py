import asyncio
import unittest
from unittest.mock import AsyncMock, patch
import led_status_display as lcd_node

class TestLCDStatusDisplay(unittest.TestCase):

    @patch('led_status_display.LCD')
    @patch('led_status_display.websockets')
    def test_receive_ready_and_sorting(self, mock_websockets, mock_lcd_class):
        mock_lcd = mock_lcd_class.return_value

        # Simulate async websocket context manager
        mock_websocket = AsyncMock()
        mock_websocket.recv = AsyncMock(side_effect=[
            '{"status": "ready"}',
            '{"status": "sorting"}'
        ])
        mock_websockets.connect.return_value.__aenter__.return_value = mock_websocket

        # Run listen_for_status() with 0 delay and max 2 messages for test
        asyncio.run(lcd_node.listen_for_status(delay=0, max_messages=2))

        # Assert LCD behavior for both messages
        mock_lcd.clear.assert_called()
        mock_lcd.text.assert_any_call("Place one item at a time", 1)
        mock_lcd.text.assert_any_call("wait, sorting...", 1)

if __name__ == "__main__":
    unittest.main()
