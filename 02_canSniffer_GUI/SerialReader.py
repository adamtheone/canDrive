from PyQt5.QtCore import QThread, pyqtSignal
import serial


class SerialReaderThread(QThread):
    received_package_signal = pyqtSignal(str)
    buf = bytearray()

    def __init__(self, serial_=None):
        super(SerialReaderThread, self).__init__()
        self.serial = serial_
        self.is_running = False

    def stop(self):
        self.is_running = False

    def run(self):
        self.is_running = True
        while self.is_running:
            i = self.buf.find(b"\n")
            if i >= 0:
                r = self.buf[:i + 1]
                self.buf = self.buf[i + 1:]
                # print(r.decode("utf-8"))
                try:
                    decodedData = r.decode("utf-8")
                    self.received_package_signal.emit(decodedData)
                except UnicodeDecodeError as e:
                    print(e)
            try:
                incoming_bytes_num = max(1, min(2048, self.serial.in_waiting))
                data = self.serial.read(incoming_bytes_num)
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