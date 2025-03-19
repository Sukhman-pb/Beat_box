#ifndef _ACCEL_H
#define _ACCEL_H

#include <stdint.h>
#include <stdbool.h>

void accel_init(void);

int16_t read_accel(int fileDescriptor, uint8_t low, uint8_t high);

bool get_accel(int choice);

void accel_close(void);
#endif