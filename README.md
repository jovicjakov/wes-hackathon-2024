# ESP32-Powered Tic Tac Toe Game

## Introduction
This project is a part of our hackathon, where we developed an interactive Tic Tac Toe game that runs on a custom-developed ESP32 devkit. Players can engage with the game via an MQTT topic subscription. Additionally, the GUI provides two main screens – one for playing the game and another for displaying real-time sensor data, including temperature, humidity, and current time. The project also features LED signalization on button presses and a Morse code SOS signal using a buzzer.

## Hardware Requirements
- Custom ESP32 development kit
- Joystick module for game navigation
- Temperature and humidity sensor 
- LEDs for signalization
- Buzzer for Morse code output
- Button for initiating SOS signal
- Screen for GUI display 

## Software Requirements
- MQTT broker (e.g., Mosquitto)
- FreeRTOS (for task management and scheduling)
- LVGL library for GUI rendering

## Setup Instructions
1. Flash the provided firmware onto the ESP32 devkit using the ESP-IDF.
2. Connect the joystick, temperature/humidity sensor, LEDs, buzzer, and button to the designated pins on the ESP32 devkit.
3. Ensure the ESP32 is connected to the same network as the MQTT broker.
4. Adjust the `sdkconfig` file if necessary to match your network and MQTT settings.

## How to Play
1. Navigate the Tic Tac Toe grid using the joystick.
2. Select a cell with a button press on the joystick.
3. The game state is updated and published to the MQTT topic, allowing for remote play.
4. An LED indicator will show the current player's turn and winning status.
5. Press the designated button to send an SOS signal via the buzzer in Morse code.

## GUI Screens
- **Game Screen**: Displays the Tic Tac Toe grid and game status.
- **Sensor Data Screen**: Shows the temperature, humidity, and current time fetched from onboard sensors and the SNTP server.

## LED Signalization
- A sequence of LED flashes indicates button press or sensor data publishing.

## Morse Code SOS Signal
- Pressing the SOS button will trigger the buzzer to emit an SOS signal in Morse code.

