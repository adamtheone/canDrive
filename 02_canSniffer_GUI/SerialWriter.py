from PyQt5.QtCore import QThread
import queue


class SerialWriterThread(QThread):
    writerQ = queue.Queue()
    tempQ = queue.Queue()
    writeDelay = 0

    def __init__(self, serial_=None):
        super(SerialWriterThread, self).__init__()
        self.serial = serial_
        self.is_running = False

    def stop(self):
        self.is_running = False

    def write(self, packet):
        self.writerQ.put(packet)

    def setRepeatedWriteDelay(self, delay):
        self.writeDelay = delay
        with self.tempQ.mutex:
            self.tempQ.queue.clear()

    def run(self):
        self.is_running = True
        while self.is_running:
            if not self.writerQ.empty():
                element = self.writerQ.get()
                num = self.serial.write(element.encode("utf-8"))
                # print(element.encode("utf-8"))

                if self.writeDelay != 0:
                    self.tempQ.put(element)

            else:
                if self.writeDelay != 0 and not self.tempQ.empty():
                    self.msleep(self.writeDelay)
                    while not self.tempQ.empty():
                        self.writerQ.put(self.tempQ.get())
                else:
                    self.msleep(1)
