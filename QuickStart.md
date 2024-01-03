Purchase a Sailor Hat ESP32 board with the accessories required to build mairas' NMEA-USB gateway.
https://docs.hatlabs.fi/sh-esp32/pages/tutorials/nmea2000-gateway/
Purchase a Honeywell position sensor. I used the 100 degree but in retrospect I would use the 180 degree sensor.
https://prod-edam.honeywell.com/content/dam/honeywell-edam/sps/siot/en-us/products/sensors/motion-position-sensors/magnetic-position-sensors/smart-position-sensors-arc/documents/sps-siot-smart-position-sensor-100-180degrees-arc-product-sheet-000732-3-en-ciid-142139.pdf
Purchase components needed to build the voltage divider as per RandelO's instructions. Note that RandelO used an external ADC while I used one of the ADCs available on the ESP32 (TBD: diagram).
My Honeywell sensor runs at 12V so I don't need a voltage regulator. I connected the analog output from the sensor to pin 36 on the Sailor Hat header.
Flash RandelO's code to test before proceeding.
Purchase the MCP2515 CAN controller with SPI interface.
https://ww1.microchip.com/downloads/en/DeviceDoc/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf
Wire the CAN controller to the ESP32 header (TBD: diagram)
Purchase a 78L05 voltage regulator to provide 5V power from the 12V CAN bus, and wire it to the MCP2515.
Both of your N2K connectors on the ESP32 will be connected to power on the CAN bus connector on the Sailor Hat. One of the N2K sockets will be connected to CAN H and CAN L on the same CAN bus connector. This will be the interface for your boat's N2K network (where all of your instruments/transducers *except* wind will be plugged in). This network should be terminated as usual.
The other N2K connector in the box will get power from the CAN bus, but its CAN L and CAN H will be connected to the MCP2515. This will be the isolated network for the wind transducer/GNX10. This network will have only 2 nodes and may not need to have external termination resistors depending on how you wire it. There's a lot of information on the Internet about terminating N2K, but the short answer is that you can put a jumper on the MCP2515 at J1 to connect a termination resistor on the Sailor Hat side.
Flash the code in this repository. When it's running, the display will show the range of values observed by the ADC (the "Sens:" line on the display). You may need to adjust your code in the readAnalogRotationValue() function to change the lowset and highset constants to reflect what your boat shows as the range of rotation.
If you did everything correctly, you will see *uncorrected* AWS and AWA on the Sailor Hat display, and corrected AWS and AWA on your Garmin wind display.
