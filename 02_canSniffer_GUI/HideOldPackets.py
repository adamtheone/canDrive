from PyQt5.QtCore import QThread, pyqtSignal


class HideOldPacketsThread(QThread):
    hide_old_packets_signal = pyqtSignal()

    def __init__(self):
        super(HideOldPacketsThread, self).__init__()
        self.is_running = False
        self.period = 5

    def stop(self):
        self.is_running = False

    def enable(self, _period):
        self.is_running = True
        self.period = _period

    def run(self):
        while self.is_running:
            self.sleep(self.period)
            self.hide_old_packets_signal.emit()
