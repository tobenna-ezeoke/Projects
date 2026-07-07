import unittest
from unittest.mock import patch, MagicMock
import full_bin_notifier as notifier

class TestFullBinNotifierRPi3(unittest.TestCase):

    @patch('full_bin_notifier.requests.post')
    @patch('full_bin_notifier.sense')
    @patch('full_bin_notifier.db.reference')
    def test_compost_and_paper_alerts(self, mock_db_ref, mock_sense, mock_requests):
        # Simulate compost = full, paper = not full on Firebase
        mock_db_ref.side_effect = lambda path: MagicMock(get=MagicMock(return_value="full" if "bin1" in path else "not full"))

        # Call main loop but stop it after 1 iteration
        with patch('time.sleep', side_effect=KeyboardInterrupt):
            with self.assertRaises(KeyboardInterrupt):
                notifier.main()

        # Assert correct alert triggered
        mock_sense.show_message.assert_called_with("Compost bin full", text_colour=[0, 255, 0])
        mock_requests.post.assert_called_once()

if __name__ == "__main__":
    unittest.main()
