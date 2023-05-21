 
from PyQt5.QtCore import QThread, pyqtSignal
import can


class SocketCanWriterThread(QThread):
    packetSentSignal = pyqtSignal()
    writerQ = []  # Queue for storing can.Message objects
    repeatedWriteDelay = 0
    normalWriteDelay = 0

    def __init__(self, bus=None):
        super(SocketCanWriterThread, self).__init__()
        self.bus = bus
        self.isRunning = False

    def clearQueues(self):
        self.writerQ.clear()

    def stop(self):
        self.isRunning = False
        self.clearQueue()

    def write(self, message):
        self.writerQ.append(message)

    def setRepeatedWriteDelay(self, delay):
        self.repeatedWriteDelay = delay

    def setNormalWriteDelay(self, delay):
        self.normalWriteDelay = delay

    def run(self):
        self.isRunning = True
        while self.isRunning:
            while self.writerQ:
                message = self.writerQ.pop(0)
                self.bus.send(message)

                if self.normalWriteDelay != 0:
                    self.msleep(self.normalWriteDelay)
                    self.normalWriteDelay = 0

                if self.repeatedWriteDelay != 0:
                    self.writerQ.append(message)

                self.packetSentSignal.emit()

            if self.repeatedWriteDelay != 0:
                self.msleep(self.repeatedWriteDelay)
            else:
                self.msleep(1)

