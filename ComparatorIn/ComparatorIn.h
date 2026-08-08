/**************************************************************************************************
 *****                                                                                        *****
 *****  Name: ComparatorIn.h                                                                  *****
 *****  Date: 05/06/2013                                                                      *****
 *****  Auth: Frank Vannieuwkerke                                                             *****
 *****  Func: library for KL25Z Comparator                                                    *****
 *****                                                                                        *****
 *****  05/08/2026: Modified by I. Cserny for easier handling of internal 1.0V Bandgap        *****
 *****  reference (IN6 / CMP_BANDGAP) and internal 6-bit DAC (IN7 / CMP_DAC6) with pseudo-pin *****
 *****  definitions.                                                                          *****
 **************************************************************************************************/

#ifndef COMPARATORIN_H
#define COMPARATORIN_H
 
/*
 * Includes
 */
#include "mbed.h"
#include "pinmap.h"
 
#ifndef TARGET_KL25Z
    #error "Target not supported"
#endif

// Pseudo-pin definition for internal non-GPIO channels
#define CMP_BANDGAP ((PinName)0xE6) // IN6: Internal 1.0V PMC Bandgap Reference
#define CMP_DAC6    ((PinName)0xE7) // IN7: Internal 6-bit Reference DAC

/** ComparatorIn library
*
* INP/INM connection selection :
*     PTC6        (IN0)    CMP0_IN0
*     PTC7        (IN1)    CMP0_IN1
*     PTC8        (IN2)    CMP0_IN2
*     PTC9        (IN3)    CMP0_IN3
*     PTE30       (IN4)    CMP0_IN4 (External 12-bit DAC0, auto connect to IN4)
*     PTE29       (IN5)    CMP0_IN5
*     CMP_BANDGAP (IN6)    Internal 1.0V PMC Bandgap Reference
*     CMP_DAC6    (IN7)    Internal 6-bit DAC Reference
*
* Example usage (Internal 1.0V Bandgap vs. 6-bit DAC Sweep):
* @code
* #include "mbed.h"
* #include "ComparatorIn.h"
*
* Serial pc(USBTX, USBRX);
* DigitalOut green_led(LED2);
* DigitalOut red_led(LED1);
*
* // IN6 (Bandgap) on (+) input, IN7 (6-bit DAC) on (-) input
* ComparatorIn cmp(CMP_BANDGAP, CMP_DAC6);
*
* int main() {
*     float threshold = 0.0f;
*     while(1) {
*         cmp.treshold(threshold);
*         wait_ms(10);
*         unsigned char status_val = cmp.status();
*         if (status_val == 1) {
*             green_led = 0; red_led = 1; // Bandgap > DAC
*         } else {
*             green_led = 1; red_led = 0; // Bandgap < DAC
*         }
*         pc.printf("DAC: %5.1f%% | CMP OUT: %d\r\n", threshold * 100.0f, status_val);
*         threshold += 0.015625f; // 1/64 step
*         if (threshold > 1.0f) threshold = 0.0f;
*         wait_ms(100);
*     }
* }
* @endcode
*/

typedef enum {
    CMP0_IN0  =  0,
    CMP0_IN1  =  1,
    CMP0_IN2  =  2,
    CMP0_IN3  =  3,
    CMP0_IN4  =  4,
    CMP0_IN5  =  5,
    CMP0_IN6  =  6, // Internal Bandgap 1.0V
    CMP0_IN7  =  7  // Internal 6-bit DAC
} CMPName;

/** Class to use KL25Z Comparator
*/ 
class ComparatorIn {
 
public:

    /** Create a ComparatorIn, connected to the specified pins.
    * @param pinP = positive ComparatorIn pin to connect to
    * @param pinM = negative ComparatorIn pin to connect to\n
    * @note Valid values for pinP/pinM:\n
    * PTC6, PTC7, PTC8, PTC9, PTE30, PTE29, CMP_BANDGAP, CMP_DAC6, NC\n
    * Special cases:\n
    * CMP_BANDGAP  Connects input to the internal 1.0V Bandgap reference (Auto-enables PMC BGBE)\n
    * CMP_DAC6     Connects input to the internal 6-bit DAC (Alias for NC)\n
    * PTE30        PTE30 is set as 12-bit DAC0 output and connected to IN4\n
    * @return none
    */
    ComparatorIn(PinName pinP, PinName pinM = CMP_DAC6);
    
    /** Set the number of consecutive threshold samples.
    * Represents the number of consecutive samples that must agree\n
    * prior to the comparator ouput filter accepting a new output state.\n
    * @param input Unsigned char - range : 1..7
    * @return none
    */
    void FilterCount(unsigned char fico);

    /** Set the hysteresis.
    * 0 : 5mV\n
    * 1 : 10mV\n
    * 2 : 20mV\n
    * 3 : 30mV\n
    * @param input Unsigned char
    * @return none
    */
    void hysteresis(unsigned char hyst);

    /** Sampling mode control.
    * This mode cannot be set when windowing mode is enabled.
    * @param input Unsigned char (0 = disable, 1 = enable)
    * @return none
    */
    void SampleMode(unsigned char samp_en);

    /** Windowing mode control.
    * @note Windowing mode is NOT implemented in KL25Z silicon. Keep disabled (0).
    * @param input Unsigned char (0 = disable, 1 = enable)
    * @return none
    */
    void WindowMode(unsigned char win_en);

    /** Trigger mode control.
    * CMP and DAC are configured to CMP Trigger mode when CMP_CR1[TRIGM] is set to 1.\n
    * @param input Unsigned char (0 = disable, 1 = enable)
    * @return none
    */
    void TrigMode(unsigned char trig_en);

    /** Power mode control.
    * 0 Low-Speed (LS) Comparison mode selected.\n
    * 1 High-Speed (HS) Comparison mode selected.\n
    * @param input Unsigned char - (0 = Low-Speed, 1 = high-Speed)
    * @return none
    */
    void PowerMode(unsigned char pmode);

    /** Invert mode control.
    * 0 Does not invert the comparator output.\n
    * 1 Inverts the comparator output.\n
    * @param input Unsigned char - (0 = not inverted, 1 = inverted)
    * @return none
    */
    void invert(unsigned char inv);

    /** Comparator Output Select.
    * 0 Set the filtered comparator output (CMPO) to equal COUT.\n
    * 1 Set the unfiltered comparator output (CMPO) to equal COUTA.\n
    * @param input Unsigned char - (0 : CMPO = COUT, 1 : CMPO = COUTA)
    * @return none
    */
    void OutputSelect(unsigned char cos);

    /** Connect the comparator Output Pin to an external pin.
    * @param input NC   disconnect CMPO from the associated CMPO output pin.
    * @param input PTC0 connect CMPO to PTC0.
    * @param input PTC5 connect CMPO to PTC5.
    * @param input PTE0 connect CMPO to PTE0.
    */
    void OutputPin(PinName ope);

    /** Comparator Module control.
    * 0 Analog Comparator is disabled.\n
    * 1 Analog Comparator is enabled.\n
    * @param input Unsigned char - (0 = disable, 1 = enable)
    * @return none
    */
    void enable(unsigned char en);

    /** Set the filter sample period.
    * @param input Unsigned char - range : 0..255
    * @return none
    */
    void FilterPeriod(unsigned char fipe);

    /** DMA Control.
    * @param input Unsigned char - (0 = disable, 1 = enable)
    * @return none
    */
    void dma(unsigned char dmaen);

    /** Analog Comparator Output.
    * @return comparator status (unsigned char)
    */
    unsigned char status(void);

    /** DAC Control.
    * 0 DAC is disabled.\n
    * 1 DAC is enabled.\n
    * @param input Unsigned char - 0 or 1
    * @return none
    */
    void dac(unsigned char den);

    /** Supply Voltage Reference Source Select.
    * 0 - Vin1 = VREFH\n
    * 1 - Vin2 = VDD\n
    * @param input Unsigned char - 0 or 1
    * @return none
    */
    void RefSource(unsigned char res);

    /** Set the detection threshold level (DAC Output Voltage Select).
    * Sets The 6-bit or 12-bit DAC output voltage.\n
    * @param input float - range 0.0 .. 1.0
    * @return none
    */
    void treshold(float vo_pct);

    /** Pass Through Mode Control.
    * @note Pass Through mode is NOT implemented in KL25Z silicon. Keep disabled (0).
    * @param input Unsigned char - (0 = disable, 1 = enable)
    * @return none
    */
    void PassThrough(unsigned char ptm);

    /** Plus Input Mux Control.
    * 0 : IN0   PTC6        CMP0_IN0\n
    * 1 : IN1   PTC7        CMP0_IN1\n
    * 2 : IN2   PTC8        CMP0_IN2\n
    * 3 : IN3   PTC9        CMP0_IN3\n
    * 4 : IN4   PTE30       CMP0_IN4 12-bit DAC0\n
    * 5 : IN5   PTE29       CMP0_IN5\n
    * 6 : IN6   CMP_BANDGAP Bandgap reference (1V)\n
    * 7 : IN7   CMP_DAC6    Internal 6-bit DAC0\n
    * @param input Unsigned char - range 0..7
    * @return none
    */
    void SwitchPlus(unsigned char pinP);

    /** Minus Input Mux Control.
    * 0 : IN0   PTC6        CMP0_IN0\n
    * 1 : IN1   PTC7        CMP0_IN1\n
    * 2 : IN2   PTC8        CMP0_IN2\n
    * 3 : IN3   PTC9        CMP0_IN3\n
    * 4 : IN4   PTE30       CMP0_IN4 12-bit DAC0\n
    * 5 : IN5   PTE29       CMP0_IN5\n
    * 6 : IN6   CMP_BANDGAP Bandgap reference (1V)\n
    * 7 : IN7   CMP_DAC6    Internal 6-bit DAC0\n
    * @param input Unsigned char - range 0..7
    * @return none
    */
    void SwitchMin(unsigned char pinM);

    /** Comparator rising interrupt callback.
    */
    void rising(void(*fptr)(void));

    /** Comparator falling interrupt callback.
    */
    void falling(void(*fptr)(void));

    static const PinMap PinMap_CMP[7];

private:
    static void _cmpISR(void);
    void hscmp_clear(void);
    PinName op_status(void);
    void op_enable(PinName pen, PinName pstat);
    void op_disable(PinName pdi);
    void dac6_write(unsigned int value);
    uint8_t CMPnumberP, CMPnumberM; // Changed to uint8_t to avoid type-limit warnings
};
 
#endif