# canDrive
Tools for hacking your car. Please concider checking out the tutorials made about this project:
https://www.youtube.com/playlist?list=PLNiFaO8hU1z0o_6DSxk-jcVAM3UCUR-pY

I've created this google sheet, so everybody can freely add their decoded IDs and packets, so it's easier for the community to collaborate:
https://docs.google.com/spreadsheets/d/1eBKiTwEE6aPnfw2EfSHItLeMz00fJq0Y0L99Ym7fKwU/edit?usp=sharing

# Content
- 01_canSniffer_Arduino: This code runs on your arduino sniffer device and creates an interface between the car and the GUI.
- 02_canSniffer_GUI: Powerful and easy-to-use graphical sniffer application used for reverse engineering CAN packets. Written in Python.
- 03_canSniffer_HW: Hardware projects for the custom OBD2 module. Made in Altium Designer.
- 04_canSniffer_FW: Embedded code running on the custom OBD2 module. 
# Description
### 01_canSniffer_Arduino
This code creates the interface between the car and the canSniffer_GUI application. If the RANDOM_CAN define is set to 1, this code is generating random CAN packets in order to test the higher level code. The received packets will be echoed back. If the  RANDOM_CAN define is set to 0, the CAN_SPEED define  has to match the speed of the desired CAN channel in order to receive and transfer from and to the CAN bus.
 Required arduino packages: 
- CAN by Sandeep Mistry (https:github.com/sandeepmistry/arduino-CAN)

Required modifications: 
- MCP2515.h: 16e6 clock frequency reduced to 8e6 (depending on MCP2515 clock)
- MCP2515.cpp: extend CNF_MAPPER with your desired CAN speeds

### 02_canSniffer_GUI
Python 3 is required for this project, 3.8 is preferred. The GUI is based on pyQt. This project contains my decoded packets in save/decodedPackets.csv. The required python packages can be installed with:
```sh
$ pip install -r requirements.txt
```

### 03_canSniffer_HW
Altium Designer projects for the hardware projects. Currently version 2. The module consists of 2 PCBs (board1 & board2). The libraries of the used components can be found under the common folder. Schematics can also be found as PDF files.

### 04_canSniffer_FW
(BETA) Only board1 beta firmware is uploaded yet. It is a STM32CubeIDE project for the STM32F413. To be uploaded: nRF project for board2.

# To be uploaded
- Final version of the firmware.
- (at some point) Source code for the mobile application that is compatible with the v2 hardware.
- (potentially) New versions of the hardware module.
