# 🏦 Micro Market Simulator

A simple, experimental market simulator designed to explore market microstructure and algorithmic trading mechanics. It includes a basic TCP/UDP-based order book engine, desktop and ESP32 clients simulating trading behavior, and a web-based terminal for live market insights.

This project is a work in progress, built to deepen understanding of networking, trading systems, and market data infrastructure.

---

## 📁 Project Structure

- **`market/`**  
  Contains the main order book engine and matching logic. Acts as the central exchange server, accepting orders over TCP and broadcasting L1 market data over UDP multicast.

- **`shared/`**  
  Shared logic between desktop and embedded clients — including order types, strategy stubs, and message structures. Ensures consistent behavior across platforms.

- **`desktop-client/`**  
  A Windows-based trading client that can connect to the market server and execute basic strategies.

- **`esp32-client/`**  
  Embedded trader logic that runs on ESP32 microcontrollers using the ESP-IDF. Randomized order generation simulates distributed, latency-constrained market participants.

- **`terminal/`**  
  A minimal web-based UI displaying market data, providing a basic front-end view into the simulation.

---

## 🛠️ Build Instructions

### 🖥️ Windows (Desktop + Server)

1. Clone the repository.
2. Run `generate_projects.bat` to set up the Visual Studio solution.
3. Open the generated `.sln` file in Visual Studio.
4. Build the solution.

> ⚠️ Note: The project currently only supports Windows for desktop components.

### 📡 ESP32 (Embedded Trader)

1. Install [ESP-IDF v5+](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/).
2. Configure the environment as per ESP-IDF instructions.
3. Navigate to `esp32-client/` and run:
   ```bash
   idf.py build
   idf.py -p <PORT> flash

---

## 🚀 Run Instructions

1. Replace all placeholder IP addresses (XX-main.cpp files) with the actual IP of your server.
2. Open relevant ports and allow UDP multicast traffic in your firewall/router settings.
3. Start the market server first, then run the desktop or ESP32 clients
