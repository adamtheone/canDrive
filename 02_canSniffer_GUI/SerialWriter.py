from PyQt5.QtCore import QThread, pyqtSignal
import queue


class SerialWriterThread(QThread):
    packetSentSignal = pyqtSignal()
    writerQ = queue.Queue()
    tempQ = queue.Queue()
    repeatedWriteDelay = 0
    normalWriteDelay = 0

    def __init__(self, serial=None):
        super(SerialWriterThread, self).__init__()
        self.serial = serial
        self.isRunning = False

    def clearQueues(self):
        self.writerQ.queue.clear()
        self.tempQ.queue.clear()

    def stop(self):
        self.isRunning = False
        self.clearQueues()

    def write(self, packet):
        self.writerQ.put(packet)

    def setRepeatedWriteDelay(self, delay):
        self.repeatedWriteDelay = delay
        with self.tempQ.mutex:
            self.tempQ.queue.clear()

    def setNormalWriteDelay(self, delay):
        self.normalWriteDelay = delay

    def run(self):
        self.isRunning = True
        while self.isRunning:
            if not self.writerQ.empty():
                element = self.writerQ.get()
                if isinstance(element, list):
                    num = self.serial.write(bytearray(element))
                    #print(bytearray(element))
                else:
                    num = self.serial.write(element.encode("utf-8"))
                    #print(element.encode("utf-8"))

                if self.normalWriteDelay != 0:
                    self.msleep(self.normalWriteDelay)
                    self.normalWriteDelay = 0

                if self.repeatedWriteDelay != 0:
                    self.tempQ.put(element)

                self.packetSentSignal.emit()
            else:
                if self.repeatedWriteDelay != 0 and not self.tempQ.empty():
                    self.msleep(self.repeatedWriteDelay)
                    while not self.tempQ.empty():
                        self.writerQ.put(self.tempQ.get())
                else:
                    self.msleep(1)
