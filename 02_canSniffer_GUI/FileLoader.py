from PyQt5.QtCore import QThread, pyqtSignal
import csv


class FileLoaderThread(QThread):
    new_row_signal = pyqtSignal(list)
    path = None
    delayMs = 0

    def __init__(self):
        super(FileLoaderThread, self).__init__()
        self.is_running = False

    def enable(self, path_, delayMs_):
        self.is_running = True
        self.path = path_
        self.delayMs = delayMs_

    def stop(self):
        self.is_running = False
        self.path = None

    def run(self):
        while self.is_running:
            if self.path is not None:
                try:
                    with open(str(self.path), 'r') as stream:
                        for rowData in csv.reader(stream):
                            if not self.is_running:
                                break
                            self.new_row_signal.emit(rowData)
                            self.msleep(self.delayMs)
                except OSError:
                    print("file not found: " + self.path)
                self.stop()
