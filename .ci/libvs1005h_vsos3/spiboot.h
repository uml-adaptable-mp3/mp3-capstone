#ifndef SPI_BOOT_H
#define SPI_BOOT_H

#ifndef ASM

/* Gets called when SPI boot encounters record 7 */
void BootSetKey(register __b0 s_int16 bytes);

#endif /* !ASM */

#endif /* !SPI_BOOT_H */
