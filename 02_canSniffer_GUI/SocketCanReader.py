 
from PyQt5.QtCore import QThread, pyqtSignal
import can
from can import Message
import time

class SocketCanReaderThread(QThread):
    receivedPacketSignal = pyqtSignal(Message, float)

    def __init__(self, bus=None):
        super(SocketCanReaderThread, self).__init__()
        self.bus = bus
        self.isRunning = False

    def isRunning(self) -> bool:
        return self.isRunning

    def stop(self):
        self.isRunning = False

    def run(self):
        self.isRunning = True
        while self.isRunning:
            try:
                message = self.bus.recv(timeout=0.1)
            except can.CanError as e:
                print(e)
                continue

            if message is not None:
                self.receivedPacketSignal.emit(message, time.time())
                    
        self.msleep(100)
