# Hotboards Keypad Library (with Key helper class)

This repository contains the **Hotboards_keypad** library together with its
associated **Key** helper class.  
Both components originate from the Hotboards project and are published on the
official Mbed website.

The **Key** class is *not* intended to be used as a standalone library.
It is an internal helper module required by the Hotboards_keypad driver.
For this reason, the two libraries should always be kept together as a single
package.

---

## Source and original author

**Author:** Hotboards  
**Original Mbed repository:**  
https://os.mbed.com/users/hotboards/code/Hotboards_keypad/

The original repository is still publicly available, but it targets the
classic Mbed 2.x API.  
This package preserves the original structure so that existing Mbed 2.x
projects can continue to use the keypad driver without modification.

---

## Compatibility

- Designed for **Mbed 2.x (Classic)**  
- Not compatible with Mbed OS 5/6 without manual porting  
- Works with offline Mbed 2.x toolchains (e.g., gcc4mbed)

---

## Usage

Simply include the library folder in your project's Makefile:
USER_LIBS := ../../external/mbed-libs/Hotboards_keypad


Then include the keypad driver in the main.cpp file of yor project:

#include "Hotboards_keypad.h"


