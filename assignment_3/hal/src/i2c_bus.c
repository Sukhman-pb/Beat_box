// TLA2024 Sample Code
// THIS CODE IS FROM THE TLA2024 SAMPLE CODE from the solution section of the course
//https://opencoursehub.cs.sfu.ca/bfraser/solutions/433/guide-code/i2c_adc_tla2024/
// - Configure DAC to continuously read an input channel on the BeagleY-AI
#include "hal/i2c_bus.h"
#include <unistd.h>
#include "hal/helper.h"
#include <errno.h>
//initialize the i2c bus
int init_i2c_bus(char* bus, int address)
{
	int i2c_file_desc = open(bus, O_RDWR);
	if (i2c_file_desc == -1) {
		printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
		perror("Error is:");
		exit(EXIT_FAILURE);
	}

	if (ioctl(i2c_file_desc, I2C_SLAVE, address) == -1) {
		perror("Unable to set I2C device to slave address.");
		exit(EXIT_FAILURE);
	}
	return i2c_file_desc;
}

void write_register_8(int i2c_file_desc, uint8_t reg_addr, uint16_t value)
{
	uint8_t buffer[2];  
    buffer[0] = reg_addr;  
    buffer[1] = value;  

    int bytes_written = write(i2c_file_desc, buffer, 2);

    // Check if the correct number of bytes were written
    if (bytes_written != 2) {
        // Print error message and terminate if write operation fails
        perror("I2C write failed");
        exit(EXIT_FAILURE);
    }
}

uint16_t read_register_8(int i2c_file_desc, uint8_t reg_addr)
{
    int bytes_written = write(i2c_file_desc, &reg_addr, 1);

    // Check if register address was successfully written
    if (bytes_written != 1) {
        perror("I2C write failed");
        exit(EXIT_FAILURE);
    }

    uint8_t value;
    int bytes_read = read(i2c_file_desc, &value, 1);
    if (bytes_read != 1) {
        perror("I2C read failed");
        exit(EXIT_FAILURE);
    }

    return value;
}

void write_register(int i2c_file_desc, uint8_t reg_addr, uint16_t value)
{
	int tx_size = 1 + sizeof(value);
	uint8_t buff[tx_size];
	buff[0] = reg_addr;
	buff[1] = (value & 0xFF);
	buff[2] = (value & 0xFF00) >> 8;
	int bytes_written = write(i2c_file_desc, buff, tx_size);
	if (bytes_written != tx_size) {
		perror("Unable to write i2c register");
		exit(EXIT_FAILURE);
	}
}

uint16_t read_register(int i2c_file_desc, uint8_t reg_addr)
{
	// To read a register, must first write the address
	int bytes_written = write(i2c_file_desc, &reg_addr, sizeof(reg_addr));
	if (bytes_written != sizeof(reg_addr)) {
		perror("Unable to write i2c register.");
		exit(EXIT_FAILURE);
	}
	//need to put a delay here in order to have the data ready to be read
	sleep_ms(50);  //50ms delay
	// Now read the value and return it
	uint16_t value = 0;
	int bytes_read = read(i2c_file_desc, &value, sizeof(value));
	if (bytes_read != sizeof(value)) {
		perror("Unable to read i2c register");
		exit(EXIT_FAILURE);
	} 
	return value;
}


void close_i2c_bus(int address){
	close(address);
}
