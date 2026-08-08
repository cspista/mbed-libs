# tsi_sensor library for Freescale FRDM-KL25Z

**Author:** Martin Kojtal  
**Source:** [http://mbed.org/users/Kojto/code/tsi_sensor](http://mbed.org/users/Kojto/code/tsi_sensor)

This library is an improved and optimized fork of the original Mbed TSI driver. It provides a more stable and accurate implementation of the **Touch Sensing Input (TSI)** peripheral specifically for Freescale/NXP Kinetis L-series MCUs, including the FRDM-KL25Z.

---

## 🌟 Key Improvements

Compared to the official Mbed TSI library, this version offers:
* **Better slider accuracy** and more reliable position calculation
* **Improved noise filtering** and signal stability
* **Bug fixes** present in the original implementation
* **Simplified API** for reading slider values
* **KL25Z-focused tuning**, making it the preferred choice for this board

> **Note:** This library is highly recommended when working with capacitive touch sliders or electrodes on the FRDM-KL25Z platform.

---

## 🚀 Usage

### 1. Include the library
```cpp
#include "tsi_sensor.h"