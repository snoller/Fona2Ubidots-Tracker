Fona2Ubidots-Tracker
====================
With this code you can post sensor-values to Ubidots with Adafruit Fona over GPRS. It will read RGB-Values from an I2C RGB-Sensor 
(Sparkfun ISL29125 https://www.sparkfun.com/products/12829) and post them to three different variables in one single call. And it will 
pull out GPS information from the GPRS-Network and post them as well.
Works without any usage of String() Object in Arduino which makes the code memory-efficient and stable. Nevetheless - to run autonomously i activated a watchdog for both routines.
Can easily be modified to carry other sensor information.

Sleep-Mode: comes with a custom sleep-function that enables the AVR-Sleep-Functionality with a handy call. If you combine this with a 
Breadboard-Arduino you can have a cool low-power tracker that runs on batteries for weeks.

Please be aware that some functions from the Adafruit Fona library have to be made public in order to compile this code. Just move the private: statement to the bottom of the function block in Adafruit_FONA.h
