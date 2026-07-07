import unittest
from unittest.mock import patch, MagicMock
import full_bin_notifier as notifier

class TestFullBinNotifierRPi4(unittest.TestCase):

    # Patch the show_message method of the SenseHat object
    @patch('full_bin_notifier.sense.show_message')
    
    # Patch other dependencies
    @patch('full_bin_notifier.requests.post')
    @patch('full_bin_notifier.db.reference')
    def test_compost_and_paper_alerts(self, mock_db_ref, mock_requests, mock_show_message):
        # Simulate compost = full, paper = not full on Firebase
        mock_db_ref.side_effect = lambda path: MagicMock(get=MagicMock(return_value="full" if "bin1" in path else "not full"))

        # Print the mock responses for debugging
        print("Landfill bin status:", mock_db_ref('/bins/bin3/status').get())
        print("Plastic/metal bin status:", mock_db_ref('/bins/bin4/status').get())

        # Call main loop but stop it after 1 iteration
        with patch('time.sleep', side_effect=KeyboardInterrupt):
            with self.assertRaises(KeyboardInterrupt):
                notifier.main()

        # Ensure show_message was called for the compost bin being full
        mock_show_message.assert_called_with("Compost bin full", text_colour=[0, 255, 0])

        # Assert that show_message was not called for the paper bin since it is not full
        mock_show_message.assert_called_once_with("Compost bin full", text_colour=[0, 255, 0])

if __name__ == "__main__":
    unittest.main()

