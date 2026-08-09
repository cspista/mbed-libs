# ComparatorIn - Archival & Enhanced Copy for FRDM-KL25Z

> **Archival & Modification Notice**
> This repository contains an enhanced version of the **ComparatorIn** library originally written and published by **Frank Vannieuwkerke** for the NXP FRDM-KL25Z development board.
> 
> Following the deprecation and retirement of the legacy Mbed OS platform, this repository preserves the original code with ongoing maintenance and usability enhancements.
> 
> - **Original Author:** Frank Vannieuwkerke
> - **Original URL:** https://developer.mbed.org/users/frankvnk/code/ComparatorIn/
> - **Target Microcontroller / Board:** FRDM-KL25Z (MKL25Z128VLK4 / ARM Cortex-M0+)
> - **05/08/2026 Modification (I. Cserny):** Added explicit CMP_BANDGAP and CMP_DAC6 pseudo-pin definitions for easier access to internal non-GPIO channels (1.0V Bandgap reference and 6-bit internal DAC).

---

## Overview

The ComparatorIn C++ class provides a high-level driver interface for configuring and controlling the analog comparator (CMP0) hardware block on the NXP FRDM-KL25Z development board using Mbed OS. 

It allows hardware-level voltage comparison with configurable hysteresis, input multiplexing, integrated DAC reference sources, interrupt callbacks on rising/falling edges, and optional output routing to external physical pins.

---

## Pin Mapping & Input Channels

The analog comparator on the FRDM-KL25Z supports multiple input sources, including external physical microcontroller pins, an integrated 12-bit DAC output, internal bandgap references, and an internal 6-bit DAC ladder network.

| Pin / Source | Channel | Signal Name | Description / Notes |
| :--- | :--- | :--- | :--- |
| PTC6 | IN0 | CMP0_IN0 | Non-inverting or Inverting pin input |
| PTC7 | IN1 | CMP0_IN1 | Non-inverting or Inverting pin input |
| PTC8 | IN2 | CMP0_IN2 | Non-inverting or Inverting pin input |
| PTC9 | IN3 | CMP0_IN3 | Non-inverting or Inverting pin input |
| PTE30 | IN4 | CMP0_IN4 | External 12-bit DAC output (DAC0) auto-connected to IN4 |
| PTE29 | IN5 | CMP0_IN5 | Non-inverting or Inverting pin input |
| CMP_BANDGAP | IN6 | Bandgap | Internal 1.0V PMC Bandgap reference voltage (Auto-enables PMC BGBE buffer) |
| CMP_DAC6 / NC | IN7 | Internal DAC | Connects channel input to the internal 6-bit DAC reference source |

### Special Pin Considerations:
* CMP_BANDGAP: Pseudo-pin (0xE6) connecting the input directly to the internal PMC 1.0V Bandgap reference. Automatically enables the PMC Bandgap buffer (PMC->REGSC |= PMC_REGSC_BGBE_MASK).
* CMP_DAC6 / NC: Pseudo-pin (0xE7) connecting the input to the internal 6-bit DAC reference source. Passing CMP_DAC6 or NC assigns this internal DAC ladder.
* PTE30: Selecting pin PTE30 automatically instantiates and initializes the onboard 12-bit hardware DAC (AnalogOut), routing its output directly to comparator channel 4.

---

## Key Member Functions Overview

| Function | Signature / Usage | Description |
| :--- | :--- | :--- |
| Constructor | ComparatorIn(PinName pinP, PinName pinM = CMP_DAC6) | Instantiates a ComparatorIn object, mapping pinP to (+) and pinM to (-). Defaults pinM to CMP_DAC6. |
| OutputPin | void OutputPin(PinName ope) | Routes the comparator output signal (CMPO) to an external physical pin (PTC0, PTC5, or PTE0). Pass NC to disconnect output routing. |
| treshold | void treshold(float vo_pct) | Sets the reference threshold voltage for either the internal 6-bit or 12-bit DAC as a fraction of full scale (0.0 to 1.0, where 1.0 corresponds to VREFH / VDD, typically 3.3V). |
| rising | void rising(void(*fptr)(void)) | Enables rising edge detection interrupts and assigns a user-defined C-style function callback (fptr). Pass NULL to disable. |
| falling | void falling(void(*fptr)(void)) | Enables falling edge detection interrupts and assigns a user-defined C-style function callback (fptr). Pass NULL to disable. |
| status | unsigned char status(void) | Queries the immediate digital state of the comparator output (0: VinP < VinM, 1: VinP > VinM). |

---

## Detailed API Reference

### Initialization & Configuration

#### ComparatorIn(PinName pinP, PinName pinM = CMP_DAC6)
Constructor to setup the comparator hardware module (CMP0). 
* Sets initial high-speed operation mode, continuous conversion, and enables the internal 6-bit DAC reference initialized at VDD/2 (1.65V).
* Valid Pin Arguments: PTC6, PTC7, PTC8, PTC9, PTE30, PTE29, CMP_BANDGAP, CMP_DAC6, NC.

#### void hysteresis(unsigned char hyst)
Configures the hardware digital hysteresis level to prevent high-frequency chatter near threshold transitions.
* 0: 5 mV
* 1: 10 mV
* 2: 20 mV
* 3: 30 mV

#### void PowerMode(unsigned char pmode)
Configures current consumption versus propagation speed.
* 0: Low-Speed (LS) mode (lower power consumption, slower output propagation delay)
* 1: High-Speed (HS) mode (higher power consumption, faster response speed)

#### void invert(unsigned char inv)
Controls the output signal polarity.
* 0: Normal polarity (active high when VinP > VinM)
* 1: Inverted polarity

---

### Reference DAC & Threshold Control

#### void treshold(float vo_pct)
Adjusts the analog threshold level generated by the internal DAC.
* vo_pct: A floating-point multiplier between 0.0 (0V) and 1.0 (3.3V).
* If CMP_DAC6 or NC was chosen for pinP or pinM, this function computes and sets the 6-bit DAC register (Vout = Vin * val / 64).
* If PTE30 was chosen, it controls the 12-bit DAC reference (Vout = Vin * val).

#### void RefSource(unsigned char res)
Selects the ladder network reference voltage for the 6-bit internal DAC.
* 0: Vin1 = VREFH
* 1: Vin2 = VDD (Recommended for optimal operation)

---

### Interrupt Handling & Callbacks

#### void rising(void(*fptr)(void))
Attaches an ISR callback function executed automatically when the comparator output transitions from LOW to HIGH.

#### void falling(void(*fptr)(void))
Attaches an ISR callback function executed automatically when the comparator output transitions from HIGH to LOW.

---

### Advanced Hardware Features

| Function | Parameter / Range | Description |
| :--- | :--- | :--- |
| FilterCount | fico: 1 .. 7 | Sets the required number of consecutive agreement samples before an output filter state change is accepted. |
| FilterPeriod | fipe: 0 .. 255 | Sets the sampling period of the output filter in bus clock cycles (0 disables filter). |
| OutputSelect | cos: 0 or 1 | Selects between filtered output (0 = COUT) and unfiltered output (1 = COUTA). |
| SampleMode | samp_en: 0 or 1 | Controls clock-sampled operation mode (cannot be enabled simultaneously with Window Mode). |
| WindowMode | win_en: 0 or 1 | Enables windowing mode (cannot be enabled simultaneously with Sample Mode). |
| TrigMode | trig_en: 0 or 1 | Enables external timer trigger compare mode. |
| dma | dmaen: 0 or 1 | Enables or disables hardware DMA request assertion on edge flags (CFR / CFF). |
| enable | en: 0 or 1 | Enables (1) or powers down (0) the comparator peripheral module. |

---

## Code Examples

### Example 1: Basic Analog Threshold Comparison

Compares an analog signal on PTC6 against the internal 6-bit DAC set to 1.25V.
```cpp
    #include "mbed.h"
    #include "ComparatorIn.h"

    // Non-inverting input on PTC6, Inverting input connected to Internal 6-bit DAC (CMP_DAC6)
    ComparatorIn cmp(PTC6, CMP_DAC6);
    DigitalOut led(LED1);

    int main() {
        // Set internal threshold to ~1.25V (3.3V * 0.3788)
        cmp.treshold(1.25f / 3.3f);
        
        while(1) {
            if (cmp.status() == 1) {
                led = 0; // Turn on LED if V_PTC6 > 1.25V
            } else {
                led = 1; // Turn off LED
            }
            wait_ms(100);
        }
    }
```
### Example 2: Interrupt-Driven Edge Detection

Triggers an interrupt callback routine whenever an input voltage crosses a specified threshold.
```cpp
    #include "mbed.h"
    #include "ComparatorIn.h"

    ComparatorIn cmp(PTC7, CMP_DAC6);
    DigitalOut red_led(LED1);

    void on_rising_edge() {
        red_led = 0; // High voltage detected
    }

    void on_falling_edge() {
        red_led = 1; // Normal voltage restored
    }

    int main() {
        // Set hysteresis to 20mV for noise immunity
        cmp.hysteresis(2);
        
        // Set 50% VDD threshold (1.65V)
        cmp.treshold(0.5f);
        
        // Attach ISR callbacks
        cmp.rising(&on_rising_edge);
        cmp.falling(&on_falling_edge);
        
        while(1) {
            // Sleep or execute main loop task
            sleep();
        }
    }
```
### Example 3: Comparing Internal Bandgap Reference vs. Sweeping 6-bit DAC

Compares the internal 1.0V PMC Bandgap reference (CMP_BANDGAP) against a sweeping internal 6-bit DAC voltage (CMP_DAC6).
```cpp
    #include "mbed.h"
    #include "ComparatorIn.h"

    Serial pc(USBTX, USBRX);
    DigitalOut green_led(LED2);
    DigitalOut red_led(LED1);

    // IN6 (Bandgap) on (+) input, IN7 (6-bit DAC) on (-) input
    ComparatorIn cmp(CMP_BANDGAP, CMP_DAC6);

    int main() {
        float threshold = 0.0f;
        while(1) {
            cmp.treshold(threshold);
            wait_ms(10);
            unsigned char status_val = cmp.status();
            if (status_val == 1) {
                green_led = 0; red_led = 1; // Bandgap > DAC
            } else {
                green_led = 1; red_led = 0; // Bandgap < DAC
            }
            pc.printf("DAC: %5.1f%% | CMP OUT: %d\r\n", threshold * 100.0f, status_val);
            threshold += 0.015625f; // 1/64 step
            if (threshold > 1.0f) threshold = 0.0f;
            wait_ms(100);
        }
    }
```
---

## License

This library is released under the original open-source terms provided by the original author (Frank Vannieuwkerke). Please refer to the source files for copyright details.