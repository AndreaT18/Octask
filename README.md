# # 🐙 Octask - Tangible Interface Firmware

**Octask** is a tangible interface designed to bridge the gap between digital productivity and physical interaction. By turning abstract tasks into physical "tentacles" that must be physically engaged, Octask helps users maintain focus and satisfaction in their workflow.

This repository contains the firmware (Arduino/C++) developed for the physical prototype.

---

## 🎓 Context
**Course:** Digital Interaction Design  
**Institution:** Politecnico di Milano  
**Project Type:** Physical Computing & Tangible Interface  

---

## 🕹️ System Architecture (The "Room" Logic)
To manage complex behaviors without conflicts, the firmware is built upon a **Finite State Machine (FSM)**. The system is divided into isolated logic "rooms":

* **🔒 Login Mode:** A waiting state with a gentle "breathing" animation. It waits for the app authentication.
* **🏠 Idle Mode:** The setup phase. Users can configure tasks via the companion app. The device listens for "Edit" commands and visualizes assignments.
* **⚡ Focus Mode:** The operational core. Physical switches control task completion.
* **🛠️ Pigi Mode:** A diagnostic state (Hardware Debug) that stress-tests LEDs and switches to verify physical connections in the tight assembly space.

---

## 🚀 Key Technical Challenges

### 1. Visual Debugging (Pigi Mode)
Given the compact design and fragile soldering, we needed a non-invasive way to check hardware health. **Pigi Mode** isn't just a screensaver; it's a diagnostic tool that simultaneously verifies:
* **Data Line Integrity:** By running full-spectrum waves.
* **Switch Responsiveness:** Changing color palettes instantly based on lever position.

### 2. "Liquid" Light Rendering
Standard LED control often looks "jumpy" (digital). To achieve an organic, liquid-like feel when a task is completed:
* We moved away from integer-based loops (`int i`).
* We implemented a **sub-pixel rendering algorithm** using floating-point math (`float`).
* The code calculates the exact "fraction of light" for LEDs between on and off states, creating a smooth, draining tail effect instead of a stepped progress bar.

### 3. Physical-Digital Sync Safety
The system implements a "Limbo State" (`MODE_ERROR_FIX`) to prevent desynchronization between the physical levers and the app state. If a user tries to exit Edit Mode while a lever is in the wrong position, the firmware intercepts the command, locks the system, and pulses a red warning until the physical constraint is resolved.

---

## 🛠️ Hardware Setup

* **Microcontroller:** Arduino Nano / Uno
* **Actuators:** 4x WS2812B LED Strips (NeoPixels) - 8 LEDs per strip
* **Sensors:** 4x SPST Toggle Switches (Pull-up configuration)
* **Power:** 5V External Power Supply

---

## 📦 How to Flash
1.  Open `Octask_Firmware.ino` in the Arduino IDE.
2.  Install the **Adafruit NeoPixel** library via the Library Manager.
3.  Connect your board and select the correct Port.
4.  Upload! 🐙

---

## 👥 Credits
Developed by [Tuonome], [NomeCompagno], [NomeCompagno].
