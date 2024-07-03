# Bulk-Push-Thingspeak

## Overview
This project generates random float values and stores them in the SPIFFS of an ESP32 every 30 seconds (the delay can be changed). After storing four values, it bulk writes the data to ThingSpeak.

## Features
- Generates random float values
- Stores data in SPIFFS of ESP32
- Writes data to ThingSpeak in bulk
- Adjustable delay between data generation

## Functionality
- The code generates random float values every 30 seconds.
- The generated values are stored in the SPIFFS of the ESP32.
- After four values are stored, the code bulk writes the data to ThingSpeak.

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.


