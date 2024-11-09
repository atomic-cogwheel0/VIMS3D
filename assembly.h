#ifndef ASSEMBLY_H
#define ASSEMBLY_H

// get number of 1/128 ticks since midnight (according to the calc's RTC)
unsigned int RTC_GetTicks(void);

// draws a line to VRAM between the specified points, probably unchecked but faster than the one in the library
int Bdisp_SYS_FastDrawLineVRAM(int x1, int y1, int x2, int y2);

// clears the keyboard input buffer (useful if GetKey() isn't called so it's not emptied and the keypresses pile up)
void Keyboard_ClrBuffer(void);

#endif