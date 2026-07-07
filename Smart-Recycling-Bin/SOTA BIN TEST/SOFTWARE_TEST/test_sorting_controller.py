import unittest
from unittest.mock import patch, AsyncMock, MagicMock
import asyncio
import sorting_controller as sorter

class TestSortingController(unittest.TestCase):

    @patch('sorting_controller.rotate_stepper')
    @patch('sorting_controller.set_servo_angle')
    @patch('sorting_controller.websockets.serve', new_callable=AsyncMock)
    def test_receive_and_sort(self, mock_ws_serve, mock_servo, mock_stepper):
        # Mock websocket
        mock_websocket = AsyncMock()
        mock_websocket.__aiter__.return_value = ['{"type": "plastic", "bin": "2"}']
        mock_websocket.send = AsyncMock()

        # Run the receive_and_sort function with the mock
        async def test_async():
            await sorter.receive_and_sort(mock_websocket, "/")

        asyncio.run(test_async())

        # Assertions: Ensure motors are controlled properly
        mock_stepper.assert_any_call(90)  # Bin 2 = 90 degrees
        mock_servo.assert_any_call(sorter.SERVO_1, 0)
        mock_servo.assert_any_call(sorter.SERVO_2, 180)
        mock_websocket.send.assert_called_once_with("Sorted Successfully")

    @patch('sorting_controller.rotate_stepper')
    @patch('sorting_controller.set_servo_angle')
    def test_invalid_data_handling(self, mock_servo, mock_stepper):
        mock_websocket = AsyncMock()
        mock_websocket.__aiter__.return_value = ['INVALID_JSON']

        async def test_async():
            await sorter.receive_and_sort(mock_websocket, "/")

        asyncio.run(test_async())

        mock_stepper.assert_not_called()
        mock_servo.assert_not_called()

if __name__ == "__main__":
    unittest.main()
