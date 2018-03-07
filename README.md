# GreenRobot
Code for a DIY robot with lots of buttons and lights. Hardware setup is a Raspberry Pi connected through USB to an arduino mega with lots of buttons for input and some RGB-lights. The eyes, mouth and heart all have their own ESP8266 and everything is communicating using MQTT. This code as well as node-red and mosquitto (mqtt server) runs in docker containers but code should be executable without this. 

The robotheart and robotmouth folders are ESP8266 code based on a mashup of code: http://www.instructables.com/id/Wearable-Message-Board/ and code for mqtt communication.

The roboteyes code is just a copy the bruhautomation code with my specific pinouts etc. https://github.com/bruhautomation/ESP-MQTT-JSON-Digital-LEDs

All robot logic is configurable using node-red and other services/things in the home automation system can also be controlled through mqtt communication.

 

 
