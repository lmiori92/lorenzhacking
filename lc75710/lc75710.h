/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Lorenzo Miori (C) 2015 [ 3M4|L: memoryS60<at>gmail.com ]

    Version History
        * 1.0 initial

*/

/**
 * @file lc75710.h
 * @author Lorenzo Miori
 * @date 7 Sept 2015
 * @brief Header of LC75710 driver functions
 */

#include <stdint.h>
#include <stdbool.h>

#define ADDRESS     B11100110   /**< Chip address */

/* Modes of operation */
#define NO_MDATA_NOR_ADATA      0x0     /**< Command does not affect MDATA nor ADATA */
#define ADATA_ONLY              0x1     /**< Command does affect ADATA only */
#define MDATA_ONLY              0x2     /**< Command does affect MDATA only */
#define MDATA_AND_ADATA         0x3     /**< Command does affect both ADATA and MDATA */

void lc75710_write(uint32_t data);
void lc75710_write_56bits(uint64_t data);
void lc75710_blink(uint8_t operation, uint8_t period, uint16_t digits);
void lc75710_on_off(uint8_t operation, bool mode, uint16_t grids);
void lc75710_shift(uint8_t operation, bool direction);
void lc75710_grid_register_load(uint8_t grids);
void lc75710_set_ac_address(uint8_t dcram, uint8_t adram);
void lc75710_intensity(uint8_t intensity);
void lc75710_dcram_write(uint8_t addr, uint8_t data);
void lc75710_adram_write(uint8_t addr, uint8_t data);
void lc75710_cgram_write(uint8_t addr, uint64_t data);
void lc75710_init(void);
