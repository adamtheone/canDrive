import serial
import canSniffer_ui
from PyQt5.QtWidgets import QMainWindow, QApplication, QTableWidgetItem, QHeaderView, QFileDialog
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor
import serial.tools.list_ports

import sys
import time
import qtmodern
from qtmodern import styles
from qtmodern import windows
import csv

import HideOldPackets
import SerialReader
import SerialWriter
import FileLoader


class canSnifferGUI(QMainWindow, canSniffer_ui.Ui_MainWindow):
    def __init__(self):
        super(canSnifferGUI, self).__init__()
        self.setupUi(self)
        self.portScanButton.clicked.connect(self.scanPorts)
        self.portConnectButton.clicked.connect(self.serialPortConnect)
        self.portDisconnectButton.clicked.connect(self.serialPortDisconnect)
        self.startSniffingButton.clicked.connect(self.startSniffing)
        self.stopSniffingButton.clicked.connect(self.stopSniffing)
        self.saveSelectedIdInDictButton.clicked.connect(self.saveIdLabelToDictCallback)
        self.saveSessionToFileButton.clicked.connect(self.saveSessionToFile)
        self.loadSessionFromFileButton.clicked.connect(self.loadSessionFromFile)
        self.showOnlyIdsLineEdit.textChanged.connect(self.showOnlyIdsTextChanged)
        self.hideIdsLineEdit.textChanged.connect(self.hideIdsTextChanged)
        self.clearLabelDictButton.clicked.connect(self.clearLabelDict)
        self.serialController = serial.Serial()
        self.serialWriterThread = SerialWriter.SerialWriterThread(self.serialController)
        self.serialReaderThread = SerialReader.SerialReaderThread(self.serialController)
        self.fileLoaderThread = FileLoader.FileLoaderThread()
        self.fileLoaderThread.new_row_signal.connect(self.mainTablePopulatorCallback)
        self.serialReaderThread.received_package_signal.connect(self.serialPacketReceiverCallback)
        self.hideOldPacketsThread = HideOldPackets.HideOldPacketsThread()
        self.hideOldPacketsThread.hide_old_packets_signal.connect(self.hideOldPacketsCallback)
        self.tableWidget.cellClicked.connect(self.cellWasClicked)
        self.newTxTableRow.clicked.connect(self.newTxTableRowCallback)
        self.removeTxTableRow.clicked.connect(self.removeTxTableRowCallback)
        self.sendTxTableButton.clicked.connect(self.sendTxTableCallback)
        self.abortSessionLoadingButton.clicked.connect(self.abortSessionLoadingCallback)
        self.showSendingTableCheckBox.clicked.connect(self.showSendingTableButtonCallback)
        self.addToDecodedPushButton.clicked.connect(self.addToDecodedCallback)
        self.deleteDecodedPacketLinePushButton.clicked.connect(self.deleteDecodedLineCallback)
        self.tableWidget_2.itemChanged.connect(self.decodedTableItemChangedCallback)
        self.clearTableButton.clicked.connect(self.clearTableCallback)
        self.sendSelectedDecodedPacketButton.clicked.connect(self.sendDecodedPacketCallback)
        self.playbackMainTableButton.clicked.connect(self.playbackMainTableCallback)
        self.sendingGroupBox.hide()
        self.hideOldPacketsThread.enable(5)
        self.hideOldPacketsThread.start()
        self.scanPorts()
        self.start_time = 0
        self.receivedPackets = 0
        self.labelDictFile = None
        self.idDict = dict([])
        self.showOnlyIdsSet = set([])
        self.hideIdsSet = set([])
        self.idLabelDict = dict()
        self.inited = False
        self.init()

        # self.table.setColumnWidth(index, width)
        for i in range(3, self.tableWidget.columnCount()):
            self.tableWidget_2.setColumnWidth(i, 15)
        for i in range(5, self.tableWidget.columnCount()):
            self.tableWidget.setColumnWidth(i, 15)
        self.tableWidget_2.horizontalHeader().setSectionResizeMode(0, QHeaderView.Stretch)
        #     self.txTable.horizontalHeader().setSectionResizeMode(0, QHeaderView.ResizeToContents)
        # self.tableWidget.horizontalHeader().setSectionResizeMode(0, QHeaderView.Stretch)
        self.txTable.horizontalHeader().setSectionResizeMode(3, QHeaderView.Stretch)
        self.showFullScreen()

    def scanPorts(self):
        #self.portSelectorComboBox.addItem('COM3')
        comPorts = serial.tools.list_ports.comports()
        nameList = list(port.device for port in comPorts)
        #print(nameList)
        for name in nameList:
            self.portSelectorComboBox.addItem(name)

    def serialPortConnect(self):
        try:
            self.serialController.port = self.portSelectorComboBox.currentText()
            self.serialController.baudrate = 250000
            self.serialController.open()
            self.serialReaderThread.start()
            self.serialWriterThread.start()
            print(self.serialController.is_open)
            self.serialConnectedCheckBox.setChecked(True)
            self.portDisconnectButton.setEnabled(True)
            self.portConnectButton.setEnabled(False)
            self.startSniffingButton.setEnabled(True)
            self.stopSniffingButton.setEnabled(False)
        except serial.SerialException as e:
            print('Error opening port:')
            print(e)

    def serialPortDisconnect(self):
        if self.stopSniffingButton.isEnabled():
            self.stopSniffing()
        try:
            self.serialReaderThread.stop()
            self.serialWriterThread.stop()
            self.portDisconnectButton.setEnabled(False)
            self.portConnectButton.setEnabled(True)
            self.startSniffingButton.setEnabled(False)
            self.serialConnectedCheckBox.setChecked(False)
            self.serialController.close()
            print(self.serialController.is_open)
        except serial.SerialException as e:
            print('Error closing port')
            print(e)

    def startSniffing(self):
        if self.autoclearCheckBox.isChecked():
            self.idDict.clear()
            self.tableWidget.setRowCount(0)
        self.startSniffingButton.setEnabled(False)
        self.stopSniffingButton.setEnabled(True)
        self.sendTxTableButton.setEnabled(True)

        self.start_time = time.time()

    def stopSniffing(self):
        self.startSniffingButton.setEnabled(True)
        self.stopSniffingButton.setEnabled(False)
        self.sendTxTableButton.setEnabled(False)

    def serialPacketReceiverCallback(self, packet):
        if self.startSniffingButton.isEnabled():
            return
        packetSplit = packet[:-1].split(',')

        if len(packetSplit) != 4:
            print("wrong packet!" + packet)
            self.snifferMsgPlainTextEdit.document().setPlainText(packet)
            return

        rowData = [str(time.time()-self.start_time)[:7]]  # timestamp
        rowData += packetSplit[0:3]  # IDE, RTR, EXT
        DLC = len(packetSplit[3]) // 2
        rowData += str(DLC)  # DLC
        if DLC > 0:
            rowData += [packetSplit[3][i:i+2] for i in range(0, len(packetSplit[3]), 2)]  # data
            
        self.mainTablePopulatorCallback(rowData)

    def mainTablePopulatorCallback(self, rowData):
        self.rxDataRadioButton.setChecked(True)
        if self.showOnlyIdsCheckBox.isChecked():
            if str(rowData[1]) not in self.showOnlyIdsSet:
                return
        if self.hideIdsCheckBox.isChecked():
            if str(rowData[1]) in self.hideIdsSet:
                return

        new_id = str(rowData[1])

        row = 0  # self.tableWidget.rowCount()
        if self.groupModeCheckBox.isChecked():
            if new_id in self.idDict.keys():
                row = self.idDict[new_id]
            else:
                row = self.tableWidget.rowCount()
                self.tableWidget.insertRow(row)
        else:
            self.tableWidget.insertRow(row)

        if self.tableWidget.isRowHidden(row):
            self.tableWidget.setRowHidden(row, False)

        for i in range(self.tableWidget.columnCount()):
            if i < len(rowData):
                data = str(rowData[i])
                item = self.tableWidget.item(row, i)
                new_item = QTableWidgetItem(data)
                if item:
                    if item.text() != data:
                        if self.highlightNewDataCheckBox.isChecked() and \
                                self.groupModeCheckBox.isChecked() and \
                                i > 4:
                            new_item.setBackground(QColor(104, 37, 98))
                else:
                    if self.highlightNewDataCheckBox.isChecked() and \
                            self.groupModeCheckBox.isChecked() and \
                            i > 4:
                        new_item.setBackground(QColor(104, 37, 98))
            else:
                new_item = QTableWidgetItem()
            self.tableWidget.setItem(row, i, new_item)

        isFamiliar = False

        if self.highlightNewIdCheckBox.isChecked():
            if new_id not in self.idDict.keys():
                for j in range(5):
                    self.tableWidget.item(row, j).setBackground(QColor(52, 44, 124))

        self.idDict[new_id] = row

        if new_id in self.idLabelDict.keys():
            value = new_id + " (" + self.idLabelDict[new_id] + ")"
            self.tableWidget.setItem(row, 1, QTableWidgetItem(value))
            isFamiliar = True

        for i in range(self.tableWidget.columnCount()):
            if (isFamiliar or (new_id.find("(") >= 0)) and i < 5:
                self.tableWidget.item(row, i).setBackground(QColor(53, 81, 52))

            self.tableWidget.item(row, i).setTextAlignment(Qt.AlignVCenter | Qt.AlignHCenter)

        self.receivedPackets = self.receivedPackets + 1
        self.packageCounterLabel.setText(str(self.receivedPackets))
        self.rxDataRadioButton.setChecked(False)

    def playbackMainTableCallback(self):
        p25 = p50 = p75 = False
        print('playing back...')
        maxRows = self.tableWidget.rowCount()
        self.serialWriterThread.setRepeatedWriteDelay(0)
        for row in range(maxRows-1, -1, -1):
            txbuf = ""
            id = ((self.tableWidget.item(row, 1).text().lower()).split(" "))[0]
            if len(id) % 2:
                txbuf += '0'
            txbuf += id + ',' + self.tableWidget.item(row, 2).text() + ',' +\
                    self.tableWidget.item(row, 3).text() + ','
            for i in range(5, self.tableWidget.columnCount()):
                txbuf += self.tableWidget.item(row, i).text()
            txbuf += '\n';
            if row < maxRows - 1:
                dt = float(self.tableWidget.item(row, 0).text()) - float(self.tableWidget.item(row+1, 0).text())
                time.sleep(dt)
            self.serialWriterThread.write(txbuf)
            #print(txbuf)
            if not p75 and (maxRows - row) / maxRows > 0.75:
                print('75%')
                p75 = True
            else:
                if not p50 and (maxRows - row) / maxRows > 0.5:
                    print('50%')
                    p50 = True
                else:
                    if not p25 and (maxRows - row) / maxRows > 0.25:
                        print('25%')
                        p25 = True
        print('finished')

    def clearTableCallback(self):
        self.idDict.clear()
        self.tableWidget.setRowCount(0)

    def sendDecodedPacketCallback(self):
        self.newTxTableRowCallback()
        new_row = 0
        new_id = str(self.tableWidget_2.item(self.tableWidget_2.currentRow(), 1).text()).split(" ")
        new_item = QTableWidgetItem(new_id[0])
        self.txTable.setItem(new_row, 0, new_item)
        new_data = ""
        for i in range(int(self.tableWidget_2.item(self.tableWidget_2.currentRow(), 2).text())):
            new_data += str(self.tableWidget_2.item(self.tableWidget_2.currentRow(), 3 + i).text())
        self.txTable.setItem(new_row, 3, QTableWidgetItem(new_data))
        self.txTable.selectRow(new_row)
        if self.sendTxTableButton.isEnabled():
            self.sendTxTableCallback()

    def decodedTableItemChangedCallback(self):
        if self.inited:
            self.saveTableToFile(self.tableWidget_2, "save/decodedPackets.csv")

    def deleteDecodedLineCallback(self):
        self.tableWidget_2.removeRow(self.tableWidget_2.currentRow())

    def addToDecodedCallback(self):
        newRow = self.tableWidget_2.rowCount()
        self.tableWidget_2.insertRow(newRow)
        new_item = QTableWidgetItem(self.tableWidget.item(self.tableWidget.currentRow(), 1))
        self.tableWidget_2.setItem(newRow, 1, new_item)
        new_item = QTableWidgetItem(self.tableWidget.item(self.tableWidget.currentRow(), 4))
        self.tableWidget_2.setItem(newRow, 2, new_item)
        for i in range(3, self.tableWidget_2.columnCount()):
            new_item = QTableWidgetItem(self.tableWidget.item(self.tableWidget.currentRow(), 2+i))
            self.tableWidget_2.setItem(newRow, i, new_item)

    def showSendingTableButtonCallback(self):
        if self.showSendingTableCheckBox.isChecked():
            self.sendingGroupBox.show()
        else:
            self.sendingGroupBox.hide()

    def hideOldPacketsCallback(self):
        if not self.hideOldPacketsCheckBox.isChecked():
            return
        if not self.groupModeCheckBox.isChecked():
            return
        for i in range(self.tableWidget.rowCount()):
            if self.tableWidget.isRowHidden(i):
                continue
            packet_time = float(self.tableWidget.item(i, 0).text())
            if (time.time() - self.start_time) - packet_time > self.hideOldPeriod.value():
                # print("Hiding: " + str(self.tableWidget.item(i,1).text()))
                # print(time.time() - self.start_time)
                self.tableWidget.setRowHidden(i, True)

    def sendTxTableCallback(self):
        for row in range(self.txTable.rowCount()):
            if self.txTable.item(row, 0).isSelected():
                txbuf = ""
                for i in range(self.txTable.columnCount()):
                    substr = self.txTable.item(row, i).text() + ","
                    if not len(substr) % 2:
                        substr = '0' + substr
                    txbuf += substr
                txbuf = txbuf[:-1] + '\n'
                if self.repeatedDelayCheckBox.isChecked():
                    self.serialWriterThread.setRepeatedWriteDelay(self.repeatTxDelayValue.value())
                else:
                    self.serialWriterThread.setRepeatedWriteDelay(0)
                self.serialWriterThread.write(txbuf)


    def abortSessionLoadingCallback(self):
        self.fileLoaderThread.stop()
        self.abortSessionLoadingButton.setEnabled(False)

    def removeTxTableRowCallback(self):
        try:
            self.txTable.removeRow(self.txTable.currentRow())
        except:
            print('cannot remove')

    def newTxTableRowCallback(self):
        newRow = 0
        self.txTable.insertRow(newRow)
        self.txTable.setItem(newRow, 1, QTableWidgetItem("00"))
        self.txTable.setItem(newRow, 2, QTableWidgetItem("00"))

    def showOnlyIdsTextChanged(self):
        self.showOnlyIdsSet.clear()
        self.showOnlyIdsSet = set(self.showOnlyIdsLineEdit.text().split(" "))

    def hideIdsTextChanged(self):
        self.hideIdsSet.clear()
        self.hideIdsSet = set(self.hideIdsLineEdit.text().split(" "))

    def init(self):
        self.loadTableFromFile(self.tableWidget_2, "save/decodedPackets.csv")
        self.loadTableFromFile(self.idLabelDictTable, "save/labelDict.csv")
        for row in range(self.idLabelDictTable.rowCount()):
            self.idLabelDict[str(self.idLabelDictTable.item(row, 0).text())] = \
                str(self.idLabelDictTable.item(row, 1).text())
        self.inited = True

    def clearLabelDict(self):
        self.idLabelDictTable.setRowCount(0)
        self.saveTableToFile(self.idLabelDictTable, "save/labelDict.csv")

    def saveTableToFile(self, table, path):
        if path is None:
            path, _ = QFileDialog.getSaveFileName(self, 'Save File', './save', 'CSV(*.csv)')
        if path != '':
            with open(str(path), 'w', newline='') as stream:
                writer = csv.writer(stream)
                for row in range(table.rowCount()-1, -1, -1):
                    rowdata = []
                    for column in range(table.columnCount()):
                        item = table.item(row, column)
                        if item is not None:
                            rowdata.append(str(item.text()))
                        else:
                            rowdata.append('')
                    writer.writerow(rowdata)

    def loadTableFromFile(self, table, path):
        if path is None:
            path, _ = QFileDialog.getOpenFileName(self, 'Open File', './save', 'CSV(*.csv)')
        if path != '':
            if table == self.tableWidget:
                self.fileLoaderThread.start()
                self.fileLoaderThread.enable(path, self.playbackDelaySpinBox.value())
                self.abortSessionLoadingButton.setEnabled(True)
                return True
            try:
                with open(str(path), 'r') as stream:
                    for rowdata in csv.reader(stream):
                        row = table.rowCount()
                        table.insertRow(row)
                        for i in range(len(rowdata)):
                            if len(rowdata[i]):
                                item = QTableWidgetItem(str(rowdata[i]))
                                table.setItem(row, i, item)
            except OSError:
                print("file not found: " + path)

    def loadSessionFromFile(self):
        if self.autoclearCheckBox.isChecked():
            self.idDict.clear()
            self.tableWidget.setRowCount(0)
        self.loadTableFromFile(self.tableWidget, None)

    def saveSessionToFile(self):
        self.saveTableToFile(self.tableWidget, None)

    def cellWasClicked(self):
        self.saveIdToDictLineEdit.setText(self.tableWidget.item(self.tableWidget.currentRow(), 1).text())

    def saveIdLabelToDictCallback(self):
        if (not self.saveIdToDictLineEdit.text()) or (not self.saveLabelToDictLineEdit.text()):
            return
        newRow = self.idLabelDictTable.rowCount()
        self.idLabelDictTable.insertRow(newRow)
        self.idLabelDictTable.setItem(newRow, 0, QTableWidgetItem(self.saveIdToDictLineEdit.text()))
        self.idLabelDictTable.setItem(newRow, 1, QTableWidgetItem(self.saveLabelToDictLineEdit.text()))
        self.idLabelDict[str(self.saveIdToDictLineEdit.text())] = str(self.saveLabelToDictLineEdit.text())
        self.saveIdToDictLineEdit.setText('')
        self.saveLabelToDictLineEdit.setText('')
        self.saveTableToFile(self.idLabelDictTable, "save/labelDict.csv")


def exception_hook(exctype, value, traceback):
    print(exctype, value, traceback)
    sys._excepthook(exctype, value, traceback)
    sys.exit(1)

def main():
    sys._excepthook = sys.excepthook
    sys.excepthook = exception_hook

    app = QApplication(sys.argv)
    gui = canSnifferGUI()

    qtmodern.styles.dark(app)
    darked_gui = qtmodern.windows.ModernWindow(gui)

    darked_gui.show()
    app.exec_()


if __name__ == "__main__":
    main()
