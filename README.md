# RP2040 OOK48 LCD

## Description

This Project is an experimental test for a new Synchronous On Off Keying mode for use with weak signal microwave contacts 

## Features

Stand alone Device with LCD touch screen display. 

No special programming hardware or software required. 

Programming using the RP2040s built in standard boot loader. 

10 Preset Messages are saved to EEPROM for automatic load on power on. 

Requires a GPS 1 Pulse per second signal for accurate timing. 

## Operation Description

### Display
The display is split into 4 main areas. On the left are the Spectrum display and Waterfall. These are used to tune the reciever to the correct frequency. 

The Spectrum and Waterfall span from 300 Hz to 2200 Hz. Underneath the spectrum display there is an orange band which indicates the correct frequency for decoding (1 KHz) . The receiver needs to be tuned such that the received tone falls withing this orange band. 

When a signal is being received the orange band will turn green to indicate a tone is being detected.

Received messages will appear at one character per second on the right hand side of the display.

When Transmitting the Spectrum display is replaced with a RED box and the Text 'TX'. The transmitted message appears on the right hand side in red as it is sent. 

### Controls

At the bottom of the screen there are 6 touch buttons. Only 3 of these are currently in use. 

Clear Screen.   Clears the Message display.

Set Tx Text.  This shows a menu of the 10 stored messages to be used for transmit. Selecting a messsage allows it to be edited on the next screen. Pressing the Enter Button saves the message.

Tx / Rx   Starts and stops the transmission of the currently selected Message. 

## Hardware Requirements

This code is designed to work with the Elecrow CrowPanel Pico-3.5 inch 480x320 TFT LCD HMI Module. https://www.aliexpress.com/item/1005007250778536.html 

![IMG_20250128_153213](https://github.com/user-attachments/assets/226fca54-6678-45e9-b151-74a64b49ffde)

Note:- similar HMI Panels are available using the ESP32 processor chip. Make sure that you are purchasing the RP2040 version. 

A GPS module is also essential and must have a 1 Pulse per second output. This pulse is used to synchronise the start of each character. 

![GPS](https://github.com/user-attachments/assets/53ceb650-0525-4138-a7ff-bff1f450c409)

## Connecting

The receiver is connected to the HMI module via a simple level shifting interface. Details are shown in this document [Connections](Documents/Schematic.pdf)

The  GPS module can be connected the Connector on the top of the module as per the schematic. Alternatively it can be connected to the UART1 connector on the lower edge of the MHI Module. However the 1PPS signal will still need to be connected to the top connector. 

The power is provided by the USB-C connector marked 'USB' on the end of the HMI module, or optionally by a 3.7V lithium cell connected to the BAT connector. 


## Programming or updating the HMI Module (quick method) 

1. Locate the latest compiled firmware file 'RP2040_OOK48_LCD_9600.uf2' or 'RP2040_OOK48_LCD_38400.uf2' which will be found here https://github.com/g4eml/RP2040_OOK48_LCD/releases and save it to your desktop. The two files allow for two different default baud rates for the GPS module. 

2. Connect the HMI Module to your PC using the USB-C port on the side. 

3. Hold down the BOOT button on the back of the HMI module and briefly press the Reset button. The RP2040 should appear as a USB disk drive on your PC.

3. Copy the .uf2 file onto the USB drive. The RP2040 will recognise the file and immediately update its firmware and reboot.

## Building your own version of the firmware (longer method and not normally required)

The RP2040 is programmed using the Arduino IDE with the Earl F. Philhower, III  RP2040 core. 

#### Installing the Arduino IDE

1. Download and Install the Arduino IDE 2.3.0 from here https://www.arduino.cc/en/software

2. Open the Arduino IDE and go to File/Preferences.

3. in the dialog enter the following URL in the 'Additional Boards Manager URLs' field: https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

4. Hit OK to close the Dialog.

5. Go to Tools->Board->Board Manager in the IDE.

6. Type “RP2040” in the search box.

7. Locate the entry for 'Raspberry Pi Pico/RP2040 by Earle F. Philhower, III' and click 'Install'

### Installing the required libraries

1. From the Arduino IDE select (Tools > Manage Libraries)
2. Search for 'arduinoFFT' Scroll down to find the arduinoFFT library by Enrique Condes.
3. Click Install
4. Now search for 'TFT_eSPI' and find the TFT graphics library by Bodmer.
5. Click Install

#### Downloading the Software.

1. Download the latest released source code .zip file from https://github.com/g4eml/RP2040_OOK48_LCD/releases

2. Save it to a convenient location and then unzip it. 

The TFT_eSPI Library is unusual in that it needs to be configured to the TFT display in use by modifying library files. 
The required modified versions of the files are located in the 'LCD-eSPI Settings' folder of this repository. 
Copy the files 'User_Setup.h' and 'User_Setup_Select.h' from  the downloaded 'LCD-eSPI Settings' folder to your Arduino libraries directory.
This will normaly be found at 'Documents/Arduino/libraries/TFT_eSPI'

#### Programming the RP2040

1. Open the Arduino IDE and click File/Open

2. Navigate to the File RP2404Synth/RP2040_OOK48_LCD.ino (downloaded in the previous step) and click Open.

3. Select Tools and make the following settings.

   Board: "Raspberry Pi Pico"
   
   Debug Level: "None"

   Debug Port: "Disabled"

   C++ Exceptions: "Disabled"

   Flash Size: "2Mb (no FS)"

   CPU Speed: "133MHz"

   IP/Bluetooth Stack: "IPV4 Only"

   Optimise: "Small (-Os)(standard)"

   RTTI: "Disabled"

   Stack Protection: "Disabled"

   Upload Method: "Default (UF2)"

   USB Stack: "Pico SDK"  

5. Connect the HMI Module to the USB port, hold down the BOOT button and briefly press the reset Button. 

6. Click Sketch/Upload.

The Sketch should compile and upload automatically to the Pico. If the upload fails you may need to disconnect the module and then hold down the BOOT button while reconnecting. 

## Connections

The receiver audio is connected using a simple CR network to GPIO Pins GND, 3V£ and 28 on the top edge of the HMI Module. 
Details of this interface are in this file.  ![Interface](Documents/Schematic.pdf)

The firmware requires the connection of a GPS module. This is used to accurately set the time and to generate the 1 Pulse per second signal used to synchromise the satrt of each character. Any GPS module with a 3V3 output and a 1PPS output can be used. It needs to output NMEA data at 9600 Baud or 38400 Baud. One of the low cost NEO6M modules was used for development. 

