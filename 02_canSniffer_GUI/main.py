# canDrive @ 2020
# To create a one-file executable, call: pyinstaller -F main.spec
#----------------------------------------------------------------
import serial
import canSniffer_ui
from PyQt5.QtWidgets import QMainWindow, QApplication, QTableWidgetItem, QHeaderView, QFileDialog, QRadioButton
from PyQt5.QtWidgets import QVBoxLayout, QSizeGrip
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QColor
import serial.tools.list_ports

import sys
import os
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
        self.mainMessageTableWidget.cellClicked.connect(self.cellWasClicked)
        self.newTxTableRow.clicked.connect(self.newTxTableRowCallback)
        self.removeTxTableRow.clicked.connect(self.removeTxTableRowCallback)
        self.sendTxTableButton.clicked.connect(self.sendTxTableCallback)
        self.abortSessionLoadingButton.clicked.connect(self.abortSessionLoadingCallback)
        self.showSendingTableCheckBox.clicked.connect(self.showSendingTableButtonCallback)
        self.addToDecodedPushButton.clicked.connect(self.addToDecodedCallback)
        self.deleteDecodedPacketLinePushButton.clicked.connect(self.deleteDecodedLineCallback)
        self.decodedMessagesTableWidget.itemChanged.connect(self.decodedTableItemChangedCallback)
        self.clearTableButton.clicked.connect(self.clearTableCallback)
        self.sendSelectedDecodedPacketButton.clicked.connect(self.sendDecodedPacketCallback)
        self.playbackMainTableButton.clicked.connect(self.playbackMainTableCallback)
        self.stopPlayBackButton.clicked.connect(self.stopPlayBackCallback)
        self.hideAllPacketsButton.clicked.connect(self.hideAllPackets)
        self.showControlsButton.hide()

        self.serialWriterThread = SerialWriter.SerialWriterThread(self.serialController)
        self.serialReaderThread = SerialReader.SerialReaderThread(self.serialController)
        self.serialReaderThread.receivedPacketSignal.connect(self.serialPacketReceiverCallback)
        self.fileLoaderThread = FileLoader.FileLoaderThread()
        self.fileLoaderThread.newRowSignal.connect(self.mainTablePopulatorCallback)
        self.fileLoaderThread.loadingFinishedSignal.connect(self.fileLoadingFinishedCallback)
        self.hideOldPacketsThread = HideOldPackets.HideOldPacketsThread()
        self.hideOldPacketsThread.hideOldPacketsSignal.connect(self.hideOldPacketsCallback)

        self.stopPlayBackButton.setVisible(False)
        self.playBackProgressBar.setVisible(False)
        self.sendingGroupBox.hide()
        self.hideOldPacketsThread.enable(5)
        self.hideOldPacketsThread.start()

        self.scanPorts()
        self.startTime = 0
        self.receivedPackets = 0
        self.playbackMainTableIndex = 0
        self.labelDictFile = None
        self.idDict = dict([])
        self.showOnlyIdsSet = set([])
        self.hideIdsSet = set([])
        self.idLabelDict = dict()
        self.isInited = False
        self.init()

        if not os.path.exists("save"):
            os.makedirs("save")

        for i in range(5, self.mainMessageTableWidget.columnCount()):
            self.mainMessageTableWidget.setColumnWidth(i, 32)
        for i in range(5, self.mainMessageTableWidget.columnCount()):
            self.decodedMessagesTableWidget.setColumnWidth(i, 32)
        self.decodedMessagesTableWidget.setColumnWidth(1, 150)
        self.decodedMessagesTableWidget.horizontalHeader().setSectionResizeMode(0, QHeaderView.Stretch)
        self.txTable.horizontalHeader().setSectionResizeMode(3, QHeaderView.Stretch)
        self.showFullScreen()

    def stopPlayBackCallback(self):
        try:
            self.serialWriterThread.packetSentSignal.disconnect()
        except:
            pass
        self.serialWriterThread.clearQueues()
        self.playbackMainTableButton.setVisible(True)
        self.stopPlayBackButton.setVisible(False)
        self.playBackProgressBar.setVisible(False)

    def setRadioButton(self, radioButton:QRadioButton, mode):
        radioButton.setAutoExclusive(False)
        if mode == 0:
            radioButton.setChecked(False)
        if mode == 1:
            radioButton.setChecked(True)
        if mode == 2:
            radioButton.setChecked(not radioButton.isChecked())
        radioButton.setAutoExclusive(True)
        QApplication.processEvents()

    def playbackMainTable1Packet(self):
        row = self.playbackMainTableIndex

        if row < 0:
            self.stopPlayBackCallback()
            return
        maxRows = self.mainMessageTableWidget.rowCount()
        txBuf = ""
        id = ((self.mainMessageTableWidget.item(row, 1).text()).split(" "))[0]
        if len(id) % 2:
            txBuf += '0'
        txBuf += id + ',' + self.mainMessageTableWidget.item(row, 2).text() + ',' + \
                 self.mainMessageTableWidget.item(row, 3).text() + ','
        for i in range(5, self.mainMessageTableWidget.columnCount()):
            txBuf += self.mainMessageTableWidget.item(row, i).text()
        txBuf += '\n'
        if row < maxRows - 1:
            dt = float(self.mainMessageTableWidget.item(row, 0).text()) - float(
                self.mainMessageTableWidget.item(row + 1, 0).text())
            dt = abs(int(dt * 1000))
            self.serialWriterThread.setNormalWriteDelay(dt)
        self.playBackProgressBar.setValue(int((maxRows - row) / maxRows * 100))
        self.playbackMainTableIndex -= 1

        self.serialWriterThread.write(txBuf)

    def playbackMainTableCallback(self):
        self.playbackMainTableButton.setVisible(False)
        self.stopPlayBackButton.setVisible(True)
        self.playBackProgressBar.setVisible(True)
        self.playbackMainTableIndex = self.mainMessageTableWidget.rowCount() - 1
        self.serialWriterThread.setRepeatedWriteDelay(0)
        print('playing back...')
        self.serialWriterThread.packetSentSignal.connect(self.playbackMainTable1Packet)
        self.playbackMainTable1Packet()

    def clearTableCallback(self):
        self.idDict.clear()
        self.mainMessageTableWidget.setRowCount(0)

    def sendDecodedPacketCallback(self):
        self.newTxTableRowCallback()
        newRow = 0
        decodedCurrentRow = self.decodedMessagesTableWidget.currentRow()
        newId = str(self.decodedMessagesTableWidget.item(decodedCurrentRow, 1).text()).split(" ")
        newItem = QTableWidgetItem(newId[0])
        self.txTable.setItem(newRow, 0, QTableWidgetItem(newItem))
        for i in range(1, 3):
            self.txTable.setItem(newRow, i, self.decodedMessagesTableWidget.item(decodedCurrentRow, i+1))
        newData = ""
        for i in range(int(self.decodedMessagesTableWidget.item(decodedCurrentRow, 4).text())):
            newData += str(self.decodedMessagesTableWidget.item(decodedCurrentRow, 5 + i).text())
        self.txTable.setItem(newRow, 3, QTableWidgetItem(newData))
        self.txTable.selectRow(newRow)
        if self.sendTxTableButton.isEnabled():
            self.sendTxTableCallback()

    def decodedTableItemChangedCallback(self):
        if self.isInited:
            self.saveTableToFile(self.decodedMessagesTableWidget, "save/decodedPackets.csv")

    def deleteDecodedLineCallback(self):
        self.decodedMessagesTableWidget.removeRow(self.decodedMessagesTableWidget.currentRow())

    def addToDecodedCallback(self):
        newRow = self.decodedMessagesTableWidget.rowCount()
        self.decodedMessagesTableWidget.insertRow(newRow)
        for i in range(1, self.decodedMessagesTableWidget.columnCount()):
            new_item = QTableWidgetItem(self.mainMessageTableWidget.item(self.mainMessageTableWidget.currentRow(), i))
            self.decodedMessagesTableWidget.setItem(newRow, i, new_item)

    def showSendingTableButtonCallback(self):
        if self.showSendingTableCheckBox.isChecked():
            self.sendingGroupBox.show()
        else:
            self.sendingGroupBox.hide()

    def hideAllPackets(self):
        text = ""
        for id in self.idDict:
            text += id + " "
        self.hideIdsLineEdit.setText(text)
        self.clearTableCallback()

    def hideOldPacketsCallback(self):
        if not self.hideOldPacketsCheckBox.isChecked():
            return
        if not self.groupModeCheckBox.isChecked():
            return
        for i in range(self.mainMessageTableWidget.rowCount()):
            if self.mainMessageTableWidget.isRowHidden(i):
                continue
            packetTime = float(self.mainMessageTableWidget.item(i, 0).text())
            if (time.time() - self.startTime) - packetTime > self.hideOldPeriod.value():
                # print("Hiding: " + str(self.mainMessageTableWidget.item(i,1).text()))
                # print(time.time() - self.start_time)
                self.mainMessageTableWidget.setRowHidden(i, True)

    def sendTxTableCallback(self):
        self.setRadioButton(self.txDataRadioButton, 2)
        for row in range(self.txTable.rowCount()):
            if self.txTable.item(row, 0).isSelected():
                txBuf = ""
                for i in range(self.txTable.columnCount()):
                    subStr = self.txTable.item(row, i).text() + ","
                    if not len(subStr) % 2:
                        subStr = '0' + subStr
                    txBuf += subStr
                txBuf = txBuf[:-1] + '\n'
                if self.repeatedDelayCheckBox.isChecked():
                    self.serialWriterThread.setRepeatedWriteDelay(self.repeatTxDelayValue.value())
                else:
                    self.serialWriterThread.setRepeatedWriteDelay(0)
                self.serialWriterThread.write(txBuf)

    def fileLoadingFinishedCallback(self):
        self.abortSessionLoadingButton.setEnabled(False)

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
        self.loadTableFromFile(self.decodedMessagesTableWidget, "save/decodedPackets.csv")
        self.loadTableFromFile(self.idLabelDictTable, "save/labelDict.csv")
        for row in range(self.idLabelDictTable.rowCount()):
            self.idLabelDict[str(self.idLabelDictTable.item(row, 0).text())] = \
                str(self.idLabelDictTable.item(row, 1).text())
        self.isInited = True

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
                    rowData = []
                    for column in range(table.columnCount()):
                        item = table.item(row, column)
                        if item is not None:
                            rowData.append(str(item.text()))
                        else:
                            rowData.append('')
                    writer.writerow(rowData)

    def mainTablePopulatorCallback(self, rowData):
        self.setRadioButton(self.rxDataRadioButton, 2)

        if self.showOnlyIdsCheckBox.isChecked():
            if str(rowData[1]) not in self.showOnlyIdsSet:
                return
        if self.hideIdsCheckBox.isChecked():
            if str(rowData[1]) in self.hideIdsSet:
                return

        newId = str(rowData[1])

        row = 0  # self.mainMessageTableWidget.rowCount()
        if self.groupModeCheckBox.isChecked():
            if newId in self.idDict.keys():
                row = self.idDict[newId]
            else:
                row = self.mainMessageTableWidget.rowCount()
                self.mainMessageTableWidget.insertRow(row)
        else:
            self.mainMessageTableWidget.insertRow(row)

        if self.mainMessageTableWidget.isRowHidden(row):
            self.mainMessageTableWidget.setRowHidden(row, False)

        for i in range(self.mainMessageTableWidget.columnCount()):
            if i < len(rowData):
                data = str(rowData[i])
                item = self.mainMessageTableWidget.item(row, i)
                newItem = QTableWidgetItem(data)
                if item:
                    if item.text() != data:
                        if self.highlightNewDataCheckBox.isChecked() and \
                                self.groupModeCheckBox.isChecked() and \
                                i > 4:
                            newItem.setBackground(QColor(104, 37, 98))
                else:
                    if self.highlightNewDataCheckBox.isChecked() and \
                            self.groupModeCheckBox.isChecked() and \
                            i > 4:
                        newItem.setBackground(QColor(104, 37, 98))
            else:
                newItem = QTableWidgetItem()
            self.mainMessageTableWidget.setItem(row, i, newItem)

        isFamiliar = False

        if self.highlightNewIdCheckBox.isChecked():
            if newId not in self.idDict.keys():
                for j in range(3):
                    self.mainMessageTableWidget.item(row, j).setBackground(QColor(52, 44, 124))

        self.idDict[newId] = row

        if newId in self.idLabelDict.keys():
            value = newId + " (" + self.idLabelDict[newId] + ")"
            self.mainMessageTableWidget.setItem(row, 1, QTableWidgetItem(value))
            isFamiliar = True

        for i in range(self.mainMessageTableWidget.columnCount()):
            if (isFamiliar or (newId.find("(") >= 0)) and i < 3:
                self.mainMessageTableWidget.item(row, i).setBackground(QColor(53, 81, 52))

            self.mainMessageTableWidget.item(row, i).setTextAlignment(Qt.AlignVCenter | Qt.AlignHCenter)

        self.receivedPackets = self.receivedPackets + 1
        self.packageCounterLabel.setText(str(self.receivedPackets))


    def loadTableFromFile(self, table, path):
        if path is None:
            path, _ = QFileDialog.getOpenFileName(self, 'Open File', './save', 'CSV(*.csv)')
        if path != '':
            if table == self.mainMessageTableWidget:
                self.fileLoaderThread.start()
                self.fileLoaderThread.enable(path, self.playbackDelaySpinBox.value())
                self.abortSessionLoadingButton.setEnabled(True)
                return True
            try:
                with open(str(path), 'r') as stream:
                    for rowData in csv.reader(stream):
                        row = table.rowCount()
                        table.insertRow(row)
                        for i in range(len(rowData)):
                            if len(rowData[i]):
                                item = QTableWidgetItem(str(rowData[i]))
                                if not (table == self.decodedMessagesTableWidget and i == 0):
                                    item.setTextAlignment(Qt.AlignVCenter | Qt.AlignHCenter)
                                table.setItem(row, i, item)
            except OSError:
                print("file not found: " + path)

    def loadSessionFromFile(self):
        if self.autoclearCheckBox.isChecked():
            self.idDict.clear()
            self.mainMessageTableWidget.setRowCount(0)
        self.loadTableFromFile(self.mainMessageTableWidget, None)

    def saveSessionToFile(self):
        self.saveTableToFile(self.mainMessageTableWidget, None)

    def cellWasClicked(self):
        self.saveIdToDictLineEdit.setText(self.mainMessageTableWidget.item(self.mainMessageTableWidget.currentRow(), 1).text())

    def saveIdLabelToDictCallback(self):
        if (not self.saveIdToDictLineEdit.text()) or (not self.saveLabelToDictLineEdit.text()):
            return
        newRow = self.idLabelDictTable.rowCount()
        self.idLabelDictTable.insertRow(newRow)
        widgetItem = QTableWidgetItem()
        widgetItem.setTextAlignment(Qt.AlignVCenter | Qt.AlignHCenter)
        widgetItem.setText(self.saveIdToDictLineEdit.text())
        self.idLabelDictTable.setItem(newRow, 0, QTableWidgetItem(widgetItem))
        widgetItem.setText(self.saveLabelToDictLineEdit.text())
        self.idLabelDictTable.setItem(newRow, 1, QTableWidgetItem(widgetItem))
        self.idLabelDict[str(self.saveIdToDictLineEdit.text())] = str(self.saveLabelToDictLineEdit.text())
        self.saveIdToDictLineEdit.setText('')
        self.saveLabelToDictLineEdit.setText('')
        self.saveTableToFile(self.idLabelDictTable, "save/labelDict.csv")

    def startSniffing(self):
        if self.autoclearCheckBox.isChecked():
            self.idDict.clear()
            self.mainMessageTableWidget.setRowCount(0)
        self.startSniffingButton.setEnabled(False)
        self.stopSniffingButton.setEnabled(True)
        self.sendTxTableButton.setEnabled(True)
        self.activeChannelComboBox.setEnabled(False)

        txBuf = [0x42, self.activeChannelComboBox.currentIndex()]   # TX FORWARDER
        self.serialWriterThread.write(txBuf)
        txBuf = [0x41, 1 << self.activeChannelComboBox.currentIndex()]  # RX FORWARDER
        self.serialWriterThread.write(txBuf)

        self.startTime = time.time()

    def stopSniffing(self):
        self.startSniffingButton.setEnabled(True)
        self.stopSniffingButton.setEnabled(False)
        self.sendTxTableButton.setEnabled(False)
        self.activeChannelComboBox.setEnabled(True)
        self.setRadioButton(self.rxDataRadioButton, 0)

    def serialPacketReceiverCallback(self, packet):
        if self.startSniffingButton.isEnabled():
            return
        packetSplit = packet[:-1].split(',')

        if len(packetSplit) != 4:
            print("wrong packet!" + packet)
            self.snifferMsgPlainTextEdit.document().setPlainText(packet)
            return

        rowData = [str(time.time() - self.startTime)[:7]]  # timestamp
        rowData += packetSplit[0:3]  # IDE, RTR, EXT
        DLC = len(packetSplit[3]) // 2
        rowData.append(str("{:02X}".format(DLC)))  # DLC
        if DLC > 0:
            rowData += [packetSplit[3][i:i + 2] for i in range(0, len(packetSplit[3]), 2)]  # data

        self.mainTablePopulatorCallback(rowData)

    def serialPortConnect(self):
        try:
            self.serialController.port = self.portSelectorComboBox.currentText()
            self.serialController.baudrate = 250000
            self.serialController.open()
            self.serialReaderThread.start()
            self.serialWriterThread.start()
            self.serialConnectedCheckBox.setChecked(True)
            self.portDisconnectButton.setEnabled(True)
            self.portConnectButton.setEnabled(False)
            self.startSniffingButton.setEnabled(True)
            self.stopSniffingButton.setEnabled(False)
        except serial.SerialException as e:
            print('Error opening port: ' + str(e))

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
        except serial.SerialException as e:
            print('Error closing port: ' + str(e))

    def scanPorts(self):
        comPorts = serial.tools.list_ports.comports()
        nameList = list(port.device for port in comPorts)
        for name in nameList:
            self.portSelectorComboBox.addItem(name)


def exception_hook(exctype, value, traceback):
    print(exctype, value, traceback)
    sys._excepthook(exctype, value, traceback)
    sys.exit(1)

def main():
    # excepthook redirect
    sys._excepthook = sys.excepthook
    sys.excepthook = exception_hook

    # creating app
    app = QApplication(sys.argv)
    gui = canSnifferGUI()

    #applying dark theme
    qtmodern.styles.dark(app)
    darked_gui = qtmodern.windows.ModernWindow(gui)

    # adding a grip to the top left corner to make the frameless window resizable
    layout = QVBoxLayout()
    sizegrip = QSizeGrip(darked_gui)
    sizegrip.setMaximumSize(30, 30)
    layout.addWidget(sizegrip, 50, Qt.AlignBottom | Qt.AlignRight)
    darked_gui.setLayout(layout)

    #starting the app
    darked_gui.show()
    app.exec_()


if __name__ == "__main__":
    main()
