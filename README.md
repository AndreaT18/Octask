# 🐙 Octask Firmware

**Octask** is a tangible interface that reimagines how we manage our daily productivity. 
Developed at **Politecnico di Milano** (Digital Interaction Design).

## 💡 The Concept
In a world full of digital distractions, checking off a to-do list on a screen often feels abstract and lacks real satisfaction. **Octask bridges the gap between digital workflows and physical interaction.** By turning digital tasks into physical "tentacles" (levers and glowing LEDs), Octask requires users to physically engage with their goals. Pulling a switch to complete a task provides tactile, satisfying feedback. It helps maintain focus, reduces screen time, and brings a sense of physical accomplishment back to our daily routines. 

This repository contains the Arduino/C++ firmware that brings the octopus to life, handling the physical inputs, the communication with the app, and the fluid, organic light animations.

## 🛠️ Hardware Requirements
* Arduino board (Uno/Nano)
* 4x WS2812B LED Strips (NeoPixels)
* 4x Physical Toggle Switches

## 🚀 How to Flash
1. Download this repository and open `Octask_Firmware.ino` in the Arduino IDE.
2. Install the **Adafruit NeoPixel** library via the Library Manager.
3. Connect your board and upload.
