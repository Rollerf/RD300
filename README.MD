# Automatic door
## Description
This is a door control with ESP8266.

## Configuration
You'll need your own variables for MQTT and WIFI connection. I use in this format:

WIFI connection file:
const char *ssid = "";
const char *password = "";

MQTT connection file:
const char *mqtt_broker = "";
const char *topicState = "";
const char *topicCommand = "";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = ;
const char *client_name = "";

## Connections
| Name                | Name in board | GPIO/IDE Arduino name |
|---------------------|---------------|-----------------------|
| Open limit switch   | D2            | 5                     |
| Closed limit switch | D1            | 4                     |
| Engine close        | D5            | 14                    |
| Engine open         | D6            | 12                    |
| Engine stop         | D7            | 13                    |
| Engine strength     | A0            | A0                    |
