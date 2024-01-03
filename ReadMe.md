This repo is a fork of the original mast rotation compensator from RandelO, to enable the code to be used with Garmin wind instruments. The Garmin GNX Wind display does not allow user configuration of its N2K source, so if the GND10 (Nexus<>N2K translator) and the ESP32 (mast rotation corrector) are on the same network, the GNX Wind will always revert to the GND10 and ignore the corrected data.

Details page.

Acknowledgements. 
All of the people below provided support for this project and answered many annoying noob questions.
randelO wrote the original code and figured out how to solve this problem initially for far less than the cost of a commercially-available rotation-compensated masthead wind instrument.
ttlappalainen wrote the N2K libraries.
mairas develops the Sailor Hat ESP32 hardware.
buhhe got me started on parsing raw N2K data from the 2nd CAN bus.
lkarsten wrote a library to parse Nexus FDX data from the masthead wind transducer.
