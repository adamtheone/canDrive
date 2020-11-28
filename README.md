# canDrive
Tools for hacking your car. Please concider checking out the tutorials made about this project:
https://www.youtube.com/playlist?list=PLNiFaO8hU1z0o_6DSxk-jcVAM3UCUR-pY

I've created this google sheet, so everybody can freely add their decoded IDs and packets, so it's easier for the community to collaborate:
https://docs.google.com/spreadsheets/d/1eBKiTwEE6aPnfw2EfSHItLeMz00fJq0Y0L99Ym7fKwU/edit?usp=sharing

# Content
- 01_canSniffer_Arduino: This code runs on your arduino sniffer device and creates an interface between the car and the GUI.
- 02_canSniffer_GUI: Powerful and easy-to-use graphical sniffer application used for reverse engineering CAN packets. 
# Description
##### 01_canSniffer_Arduino
This code creates the interface between the car and the canSniffer_GUI application. If the RANDOM_CAN define is set to 1, this code is generating random CAN packets in order to test the higher level code. The received packets will be echoed back. If the  RANDOM_CAN define is set to 0, the CAN_SPEED define  has to match the speed of the desired CAN channel in order to receive and transfer from and to the CAN bus.
 Required arduino packages: 
- CAN by Sandeep Mistry (https:github.com/sandeepmistry/arduino-CAN)

Required modifications: 
- MCP2515.h: 16e6 clock frequency reduced to 8e6 (depending on MCP2515 clock)
- MCP2515.cpp: extend CNF_MAPPER with your desired CAN speeds

##### 02_canSniffer_GUI
Python 3 is required for this project, 3.8 is preferred. This project contains my decoded packets in save/decodedPackets.csv. The required python packages can be installed with:
```sh
$ pip install -r requirements.txt
```
# To be uploaded
- Hardware files for the custom OBD2 module.
- Firmware projects for the custom OBD2 module.
- Source code for the mobile application.
