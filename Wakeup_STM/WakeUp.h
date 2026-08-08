/*

    Original idea & program
    https://os.mbed.com/users/Sissors/code/WakeUp/
        by Erik
 */

/*
 *  Modified only for STM CPU
 *      by Kenji Arai / JH1PJL
 *
 *  http://www7b.biglobe.ne.jp/~kenjia/
 *  http://mbed.org/users/kenjiArai/
 *      Created:    September  21st, 2017
 *      Revised:    March      12th, 2020
 */

#include "mbed.h"

/**
 * Class to make wake up a STM CPU from deepsleep using a low-power timer. 
 *
 * @code
 * 
 * #include "mbed.h"
 * #include "WakeUp_STM.h"
 * 
 * DigitalOut myled(LED1);
 * 
 * int main() {
 *     uint32_t loop_count = 1;
 *     ThisThread::sleep_for(1000);
 *     while(true) {
 *         myled = !my_led;
 *         if (++count > 4) {
 *             WakeUp::standby_then_reset(30000);  // 30sec
 *             while(true) {;} // never executing this line
 *         }
 *     }
 * }
 * @endcode
 */
class WakeUp
{
public:
    /**
    * Enter Standby mode then Reset
    * @param ms required time in milliseconds
    */
    static void standby_then_reset(uint32_t ms);

private:
    static Callback<void()> callback;
    static void irq_handler(void);
    static void set_ms(uint32_t ms);
    static float cycles_per_ms;
};