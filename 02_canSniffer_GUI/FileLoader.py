from PyQt5.QtCore import QThread, pyqtSignal
import csv


class FileLoaderThread(QThread):
    newRowSignal = pyqtSignal(list)
    loadingFinishedSignal = pyqtSignal()
    path = None
    delayMs = 0

    def __init__(self):
        super(FileLoaderThread, self).__init__()
        self.isRunning = False

    def enable(self, path, delayMs):
        self.isRunning = True
        self.path = path
        self.delayMs = delayMs

    def stop(self):
        self.isRunning = False
        self.path = None

    def run(self):
        while self.isRunning:
            if self.path is not None:
                try:
                    with open(str(self.path), 'r') as stream:
                        for rowData in csv.reader(stream):
                            if not self.isRunning:
                                break
                            self.newRowSignal.emit(rowData)
                            self.msleep(self.delayMs)
                        self.loadingFinishedSignal.emit()
                except OSError:
                    print("file not found: " + self.path)
                self.stop()
