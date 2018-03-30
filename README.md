# esp8266DHTMQTT
Using an ESP8266 chip and DHT11 shield to send temperature, humidity and heat index info over MQTT.

This sketch sends the temperature, humidity and heat index info over MQTT every ten seconds if they changed by at least 0.1

It can be used together with [Home Assistant](http://www.home-assistant.io) or any other MQTT based platform.

Requires *PubSubClient* package to be installed on your Arduino environment.