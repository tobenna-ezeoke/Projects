import unittest
from unittest.mock import patch, MagicMock
import rpi4_bin_monitor as monitor

class TestRpi4BinMonitor(unittest.TestCase):

    @patch('rpi4_bin_monitor.db')
    @patch('rpi4_bin_monitor.sqlite3.connect')
    @patch('rpi4_bin_monitor.GPIO')
    def test_check_bin3_and_bin4_status(self, mock_gpio, mock_sqlite, mock_firebase):
        mock_conn = MagicMock()
        mock_cursor = MagicMock()
        mock_conn.cursor.return_value = mock_cursor
        mock_sqlite.return_value = mock_conn
        mock_ref = MagicMock()
        mock_firebase.reference.return_value = mock_ref

        full_count = {3: 4, 4: 4}

        # ✅ Bin 3 logic
        monitor.check_bin_status(3, distance=2.5, full_count=full_count)
        mock_firebase.reference.assert_any_call('/bins/bin3')
        mock_ref.set.assert_any_call({
            "status": "full",
            "last_updated": unittest.mock.ANY
        })

        # ✅ Bin 4 logic
        monitor.check_bin_status(4, distance=2.5, full_count=full_count)
        mock_firebase.reference.assert_any_call('/bins/bin4')
        mock_ref.set.assert_any_call({
            "status": "full",
            "last_updated": unittest.mock.ANY
        })

    @patch('rpi4_bin_monitor.GPIO')
    def test_measure_distance_mock(self, mock_gpio):
        mock_gpio.input.side_effect = [0, 1, 0, 1]
        distance = monitor.measure_distance(TRIG=17, ECHO=27)
        self.assertIsInstance(distance, float)

if __name__ == "__main__":
    unittest.main()

