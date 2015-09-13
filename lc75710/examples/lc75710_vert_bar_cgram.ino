
#include "lc75710.h"

/* Load vertical bars in the CGRAM of the chip */
void display_load_bars_in_ram()
{
    
    uint8_t i = 0;
    uint64_t c = 0;
    
    for (i = 0; i < 7; i++)
    {
        c |= (uint64_t)0x1F << (30 - (i*5));
        lc75710_cgram_write(i, c);
    }
    
}

void setup()
{
    /* Clear internal RAM */
    display_clear(64);
    /* Load bar data into the CGRAM that will be used for displaying
       bars */
    display_load_bars_in_ram();
    /* Turn all the display digits on */
    lc75710_on_off(MDATA_AND_ADATA, true, 0xFFFF);
}

void loop()
{

    static uint16_t i = 0;

    /* Create a fake FFT visualization
     * Note that CGRAM character code (0..7) is being used
     */
    lc75710_dcram_write(0, i%1);    /* digit 0 */
    lc75710_dcram_write(1, i%2);    /* digit 1 */
    lc75710_dcram_write(2, i%3);    /* digit 2 */
    lc75710_dcram_write(3, i%4);    /* digit 3 */
    lc75710_dcram_write(4, i%5);    /* digit 4 */
    lc75710_dcram_write(5, i%6);    /* digit 5 */
    lc75710_dcram_write(6, i%4);    /* digit 6 */
    lc75710_dcram_write(7, i%5);    /* digit 7 */
    lc75710_dcram_write(8, i%2);    /* digit 8 */
    lc75710_dcram_write(9, i%1);    /* digit 9 */

    i++;

    delay(200);

}
