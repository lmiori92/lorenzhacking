/* Load horizontal bars in the CGRAM of the chip */
void display_load_vertical_bars_in_ram()
{
    
    uint8_t i = 0;
    uint64_t c = 0;
    
    for (i = 0; i < 7; i++)
    {
        c |= (uint64_t)((uint64_t)0x1 << 4-i);
        c |= (uint64_t)((uint64_t)0x1 << (9-i));
        c |= (uint64_t)((uint64_t)0x1 << (14-i));
        c |= (uint64_t)((uint64_t)0x1 << (19-i));
        c |= (uint64_t)((uint64_t)0x1 << (24-i));
        c |= (uint64_t)((uint64_t)0x1 << (29-i));
        c |= (uint64_t)((uint64_t)0x1 << (34-i));
        lc75710_cgram_write(i, c);
    }
    
}

void display_load_vumeter_bars_in_ram()
{
    
    uint8_t i = 0;
    uint64_t c = 0;
    
    c = (uint64_t) 0xCC3;
    for (i = 0; i < 7; i++)
    {
        lc75710_cgram_write(i, c);//(uint64_t)0x7FFFFFFFFL << (64-35)) << (i * 5));
    }
    
}

void display_show_horizontal_bar(uint8_t level)
{
    
    uint8_t i = 0;

    display_clear(10);
    lc75710_set_ac_address(0, 0);

    /* Full bars */
    for (i = 0; i < level / 5; i++)
    {
        lc75710_dcram_write(i, 4);
    }

    lc75710_dcram_write(i, level%5-1);

}


void display_show_bars(uint8_t bar, uint8_t level)
{
    lc75710_dcram_write(bar%11, level%7);
}