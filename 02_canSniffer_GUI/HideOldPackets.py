from PyQt5.QtCore import QThread, pyqtSignal


class HideOldPacketsThread(QThread):
    hideOldPacketsSignal = pyqtSignal()

    def __init__(self):
        super(HideOldPacketsThread, self).__init__()
        self.isRunning = False
        self.period = 5

    def stop(self):
        self.isRunning = False

    def enable(self, _period):
        self.isRunning = True
        self.period = _period

    def run(self):
        while self.isRunning:
            self.sleep(self.period)
            self.hideOldPacketsSignal.emit()
