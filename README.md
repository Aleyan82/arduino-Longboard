# Arduino Plugin for [Proffieboard](https://fredrik.hubbe.net/lightsaber/v4/)
# Fork specifically made to support Longboard prototypes - WIP
## Installing

 1. [Download and install the Arduino IDE](https://www.arduino.cc/en/Main/Software) (at least version v1.6.8)
 2. Start the Arduino IDE
 3. Go into Preferences
 4. Add ```https://Aleyan82.github.io/arduino-longboard/package_proffieboard_index.json``` as an "Additional Board Manager URL"
 5. Open the Boards Manager from the Tools -> Board menu and install "Proffieboard Plugin"
 6. Select Proffieboard Tools -> Board menu

### OS Specific Setup

#### Linux

 1. Go to ~/.arduino15/packages/profezzorn/hardware/stm32l4/```<VERSION>```/drivers/linux/
 2. sudo cp *.rules /etc/udev/rules.d
 3. reboot

####  Windows

##### STM32 BOOTLOADER driver setup for STM32L4 based boards

 1. Download [Zadig](http://zadig.akeo.ie)
 2. Plugin the Proffieboard and toggle the RESET button while holding down the BOOT button
 3. Let Windows finish searching for drivers
 4. Start ```Zadig```
 5. Select ```Options -> List All Devices```
 6. Select ```STM32 BOOTLOADER``` from the device dropdown
 7. Select ```WinUSB (v6.1.7600.16385)``` as new driver
 8. Click ```Replace Driver```

##### USB Serial driver setup for STM32L4 based boards (Windows 7/XP only, not for Windows 8 or 10)

 1. Go to ~/AppData/Local/Arduino15/packages/profezzorn/hardware/stm32l4/```<VERSION>```/drivers/windows
 2. Right-click on ```dpinst_x86.exe``` (32 bit Windows) or ```dpinst_amd64.exe``` (64 bit Windows) and select ```Run as administrator```
 3. Click on ```Install this driver software anyway``` at the ```Windows Security``` popup as the driver is unsigned

## Troubleshooting

### How do I know if uploads are working?

 Look at the bottom section of the arduino program. The progress of the upload will show in red text. Unfortunately, arduino will not scroll down automatically as uploads are taking place, so if you want to see how it's doing you have to keep scrolling down while it's working.

### Recovering from a broken upload

 Sometimes a faulty sketch can render the normal USB Serial based integration into the Arduino IDE not working. In this case plug in the Proffieboard and toggle the RESET button while holding down the BOOT button and program a known to be working sketch to go back to a working USB Serial setup.

### Connection issues

#### Windows 10
 Go to the control panel and click on Bluetooth & other devices. It should either show "Proffieboard" or "STM32 BOOTLOADER". If you hold BOOT and click RESET, it should show "STM32 BOOTLOADER". If neither show up, try a different USB port or cable.

#### Linux
 Running ```sudo tail -f /var/log/kern.log``` will show you when things connect and disconnect, the lsusb command is also helpful.

## Alternative installation method for developers

 1. ```cd <SKETCHBOOK>```, where ```<SKETCHBOOK>``` is your Arduino Sketch folder:
  * OS X: ```~/Documents/Arduino```
  * Linux: ```~/Arduino```
  * Windows: ```~/Documents/Arduino```
 2. Create a folder named ```hardware```, if it does not exist, and change directories to it
 3. Clone this repo: ```git clone https://github.com/profezzorn/arduino-proffieboard.git profezzorn/stm32l4```
 4. Restart the Arduino IDE
 5. Make sure you install any drivers you need from the instructions at the top.

## Credits

This core is based on the [Arduino STM32L4 Core](https://github.com/GrumpyOldPizza/arduino-STM32L4). For now, the boards from the original core are still supported by this core.
