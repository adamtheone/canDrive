from PyQt5.QtCore import QThread, pyqtSignal
import serial


class SerialReaderThread(QThread):
    receivedPacketSignal = pyqtSignal(str)
    buf = bytearray()

    def __init__(self, serial=None):
        super(SerialReaderThread, self).__init__()
        self.serial = serial
        self.isRunning = False

    def stop(self):
        self.isRunning = False

    def run(self):
        self.isRunning = True
        while self.isRunning:
            # Because of the high transmission speed, we shouldn't assume that the internal serial buffer
            # will only contain one package at a time, so I split that buffer by end line characters.
            i = self.buf.find(b"\n")
            if i >= 0:
                r = self.buf[:i + 1]
                self.buf = self.buf[i + 1:]
                # print(r.decode("utf-8"))
                try:
                    decodedData = r.decode("utf-8")
                    self.receivedPacketSignal.emit(decodedData)
                except UnicodeDecodeError as e:
                    print(e)
            try:
                incomingBytesNum = max(1, min(2048, self.serial.in_waiting))
                data = self.serial.read(incomingBytesNum)
            except serial.SerialException as e:
                print(e)
                pass
                # There is no new data from serial port
            except TypeError as e:
                # Disconnect of USB->UART occured
                print("Serial disconnected")
                print(e)
                self.port.close()
            else:
                if len(data):
                    self.buf.extend(data)
        self.msleep(100)