Firmware for ESP32 controllers.

## Strategy
wakeup callback -> wifi reconnect -> check messages (?) -> start measurment -> send results -> wait for commands -> disconnect and deep sleep

The device will periodically wake up from a deep sleep status and, by default, the main task will be executed.
Main task can carry out all the initializations needed for nominal operation: wifi setup, connection.

possible tasks:
- main
- read messages from bridge
- write results to bridge
- measure noise
- check for possible leakage
- perform inference with ml

precedence:
main -> {read messages, measure noise} -> {perform inference, check for leakage} -> write results

This task will then spawn two other tasks: one for measurments and the other for reading pending messages from the bridge.

# Events
idea is to define an event loop where tasks that will perform specific actions (like leak detection or inference) can post events when they finish: these tasks will post result data and the main task will be able to handle this data by registering event callbacks.

# MQTT
how to handle lifecycle of events? What happens when we disconnect?
MQTT_EVENT_DATA -> new data arrived. useful to check when bridge communicates something
how to check if connection is already present?
## Clean Session and Persistent Session
> From **MQTTv3.1.1 Specification** under *Conncect Flags*: The Client and Server can store Session state to enable reliable messaging to continue across a sequence of Network Connections. This (Clean Connection) bit is used to control the lifetime of the Session state. If CleanSession is set to 0, the Server MUST resume communications with the Client based on state from the current Session (as identified by the Client identifier). If there is no Session associated with the Client identifier the Server MUST create a new Session. The Client and Server MUST store the Session after the Client and Server are disconnected. If CleanSession is set to 1, the Client and Server MUST discard any previous Session and start a new one. This Session lasts as long as the Network Connection.

MQTT v3.1.1 CONNACK packet contains the "Session Present": it is set if the session is being restored following a connection request. In the connection configuration of ESP-MQTT we can specify the flag "disable clean session": if this flag is false, then at each connection the session for a specific client is cleared; setting the flag to false allows the persistence of teh session on the broker. When a client reconnects to the borker, its session will be restored automatically and all messages with QoS 1 and 2 will be delivered.
> It's the client that tells the broker if it wants to start a fresh session or if it wants a saved session by setting the Clean Session bit in the CONNECT packet.

We can check if the session was restored when receiving the CONNECTED event by checking the persistent_session flag.

ESP MQTT has an automatic reconnection mechanism. Maybe check for number of retries.

## Retained message
In the case of devices publishing measure messages or the bridge wanting to send a command to a device, a problem may arise: what if the deivce/bridge is not connected in that moment? For example, when the device reconnects after the bridge has already sent a command, it will not receive that latest sent command when it reconnects (even if it is a QoS > 0 message!). To be sure that a message is received by subscribers as the last published message (even if it was sent while they were not connected), the **Retain flag** must be set: this flags tells the broker to store the message and transmit it to everyone that subscribes to that topic. 

> Only the _last_ message is saved: any message with the retain flag set overwrites the previously saved message!

# ADC
Using only one adc channel to read the microphone data.
basic configuration:
- 12 bit quatization
- 20k Hz sampling (minimum sampling frequency on ESP23) -> how do we downsample to the preferred frequency?

# Error Handling
All fatal errors (those that would not permit the app to continue executing correctly) are handled with an abort through a macro given by ESP.
The system is configured to produce a core dump that will be saved in a dedicated flash memory partition after it receives an abort: this enables a detailed post-mortem analysis in case of malfunctions.


# Power management
Would like to keep wifi connection up while entering a power management mode

In Deep-sleep and Light-sleep modes, the wireless peripherals are powered down. Before entering Deep-sleep or Light-sleep modes, the application must disable Wi-Fi and Bluetooth using the appropriate calls (i.e., esp_bluedroid_disable(), esp_bt_controller_disable(), esp_wifi_stop()). Wi-Fi and Bluetooth connections will not be maintained in Deep-sleep or Light-sleep mode, even if these functions are not called.

If Wi-Fi/Bluetooth connections need to be maintained, enable Wi-Fi/Bluetooth Modem-sleep mode and automatic Light-sleep feature (see Power Management APIs). This will allow the system to wake up from sleep automatically when required by the Wi-Fi/Bluetooth driver, thereby maintaining the connection.

**Better to enter deep sleep and save energy or to just enter a light sleep mode?**
Need to consider the uptime of operations vs the downtime of just wating a timer wakeup for sensor reading tasks, ML inference and transmission: downtime is predominant in the normal flow, so it would be **better to enter deep sleep**. The downside of this approach is that the wireless connection with the bridge will be lost every time the controller enters deep sleep; for this reason, before starting operations after wakeup, we would need to restablish the lost connection.

## Deep sleep
> From [docs](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/low-power-mode.html#deep-sleep): The main application scenario of Deep-sleep mode is that the system will wake up only once in a long time, and then it will continue to enter Deep-sleep after completing its work, so its ideal current diagram is as follows.

## Enhancements
Use ULP coprocessor during sleep mode to monitor the ADC and detect possible leakage.

> From [docs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html#ulp-coprocessor-wakeup): ULP coprocessor can run while the chip is in sleep mode, and may be used to poll sensors, monitor ADC or touch sensor values, and wake up the chip when a specific event is detected.

# Provisioning
ESP Touch v2 (SmartConfig). Two pieces of data passed: 
- 3 digit identification number;
- longitude and latitude coordinates.
Message format: ID#sign000.00000,sign000.00000