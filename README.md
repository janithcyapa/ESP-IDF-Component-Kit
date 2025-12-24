# ESP-IDF Component Kit 🛠️

[![ESP-IDF Version](https://img.shields.io/badge/ESP--IDF-v5.0%2B-blue.svg)](https://docs.espressif.com/projects/esp-idf/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Essential building blocks for ESP32 development. Features optimized drivers and utilities for ESP-IDF that you can drop into any project to accelerate your firmware development.

## 🚀 Getting Started

### Prerequisites
* **ESP-IDF** (v5.0 or later recommended)
* **Git** installed on your system

### Installation as a Submodule
The recommended way to use this kit is as a **Git Submodule**. This allows you to lock the library to a specific version while keeping it easily updatable.

Navigate to your project's root directory and run:

```bash
# Add the kit to your components directory
git submodule add https://github.com/janithcyapa/ESP-IDF-Component-Kit.git components/ESP-IDF-Component-Kit

# Initialize and fetch the content
git submodule update --init --recursive
```

### Usage
The ESP-IDF build system (CMake) will automatically detect the components inside the kit. To use a specific component in your code, simply include its header and add it to your project's idf_component.yml or CMakeLists.txt if required.

> Example:
```c
#include "esp_log.h"
#include "relevant_component_header.h"

void app_main(void) {
    // Initialize your component here
    component_init();
}
```

## 📂 Project Structure

```text
.
├── components/          # Individual modular components
│   ├── driver_abc/      # Example driver
│   └── utility_xyz/     # Example utility
├── examples/            # Example projects for each component
├── LICENSE              # MIT License
└── README.md
```

## 🤝 Contributing

Contributions are welcome! If you have a component that would be a great addition to this kit:

- Fork the Project

- Create your Feature Branch (git checkout -b feature/AmazingFeature)

- Commit your Changes (git commit -m 'Add some AmazingFeature')

- Push to the Branch (git push origin feature/AmazingFeature)

- Open a Pull Request

## 📜 License

Distributed under the MIT License. See LICENSE for more information.

Maintained by: [Janith Yapa](github.com/janithcyapa)