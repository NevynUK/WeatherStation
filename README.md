# Modular Weather Station
Weather station based on the [Digistump Oak](http://digistump.com "Digistump Hom Page") (ESP8266 based system) and STM8S microcontrollers.

The mind map for this project shows the initial goals:

![Modular Weather Station Mind Map](http://blog.mark-stevens.co.uk/wp-content/uploads/2016/03/WeatherStationMindMap.jpg "Modular Weather Station Mind Map")

This software is currently in the proof of concept state and is more than a little rough around the edges.

## Build System
The application requires the following devlopment environments in order to build correctly:

- IAR STM8S IDE
- Visual Studio Community Edition
- Arduino IDE

[IARs](https://iar.com "IAR Home Page") development environment is normally a chargeable item but there is a free version.  This is restricted to 8K of code but this is adequete for this project.

[Visual Studio](https://www.visualstudio.com "Visual Studio") has been free since 2015.  The IDE is combined with [Visual Micro](http://www.visualmicro.com "Visual Micro addin for Visual Studio") addin for Visual Studio.  This addin allows the development of embedded code using the Visual Studio IDE.  Unfortuneatly this is not a free addin.

The [Arduino](https://www.arduino.cc "Arduino Home Page") IDE can be used to edit the Oak part of the application removing the need for Visual Studio.  This download is required to managed the libraries for Visual Studio even if it is not used for editing.

## Libraries
This project requires a number of libraries to be installed in order to compile and run:

1. [NtpClientLib](https://github.com/gmag11/NtpClient "NTP Client") - No licence file in repository
2. [Adafruit Unified Sensor](https://github.com/adafruit/Adafruit_Sensor "Adafruit Unified Sensor Library") - No licence file in repository
3. [Adafruit BME280](https://github.com/adafruit/Adafruit_BME280_Library "Adafruit BME280 Temperature, Humidity and Air Pressure Sensor") - No licence file in repository.
4. [Adafruit MQTT](https://github.com/adafruit/Adafruit_MQTT_Library "Adafruit MQTT Library") - MIT licence
5. [SparkFunTSL2561](https://github.com/sparkfun/SparkFun_TSL2561_Arduino_Library "Sprkfun TSL2561 Luminosity Sensor") - Open source (buy the developer a beer if you meet them)

### Licence
All of the libraries used are freely available on GitHub as open source or MIT licenced code.  The code for this project is released under the MIT Licence.

## Build Logs
Build logs are available showing how this project is progressing can be found in the following locations:

- [Authors web site](http://blog.mark-stevens.co.uk/weather-station/ "Mark Stevens Blog")
- [Hackaday IO Project Page](https://hackaday.io/project/12397-modular-weather-station "Modular Weasther Station on Hackaday.io")