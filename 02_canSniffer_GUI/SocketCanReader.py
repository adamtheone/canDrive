 
from PyQt5.QtCore import QThread, pyqtSignal
import can
import time

class SocketCanReaderThread(QThread):
    receivedPacketSignal = pyqtSignal(str, float)

    def __init__(self, bus=None):
        super(SocketCanReaderThread, self).__init__()
        self.bus = bus
        self.isRunning = False

    def stop(self):
        self.isRunning = False

    def run(self):
        self.isRunning = True
        while self.isRunning:
            try:
                message = self.bus.recv()
            except can.CanError as e:
                print(e)
                continue

            if message is not None:
                decodedData = message.data.hex()
                self.receivedPacketSignal.emit(decodedData, time.time())
                    
        self.msleep(100)
