class MainTableData:
    def __init__(self, timestamp, ID, RTR, IDE, DLC, DATA_ARRAY):
        self.timestamp = timestamp
        self.ID = ID
        self.RTR = RTR
        self.IDE = IDE
        self.DLC = DLC
        self.DATA_ARRAY = DATA_ARRAY

    def getRowData(self):
        return [self.timestamp, self.ID, self.RTR, self.IDE, str("{:02X}".format(self.DLC)), *self.DATA_ARRAY]
