// TLA2024 Sample Code
// THIS CODE IS FROM THE TLA2024 SAMPLE CODE from the solution section of the course
//https://opencoursehub.cs.sfu.ca/bfraser/solutions/433/guide-code/i2c_adc_tla2024/
// - Configure DAC to continuously read an input channel on the BeagleY-AI
#include "hal/i2c_bus.h"
#include <unistd.h>
#include "hal/helper.h"

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

void write_i2c_reg16(int i2c_file_desc, uint8_t reg_addr, uint16_t value)
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

uint16_t read_i2c_reg16(int i2c_file_desc, uint8_t reg_addr)
{
	// To read a register, must first write the address
	int bytes_written = write(i2c_file_desc, &reg_addr, sizeof(reg_addr));
	if (bytes_written != sizeof(reg_addr)) {
		perror("Unable to write i2c register.");
		exit(EXIT_FAILURE);
	}
	//need to put a delay here in order to have the data ready to be read
	//sleep_ms(50);  //50ms delay
	// Now read the value and return it
	uint16_t value = 0;
	int bytes_read = read(i2c_file_desc, &value, sizeof(value));
	if (bytes_read != sizeof(value)) {
		perror("Unable to read i2c register");
		exit(EXIT_FAILURE);
	}
	return value;
}

float getVoltage(int i2c_file_desc){
    uint16_t raw_y = read_i2c_reg16(i2c_file_desc, REG_DATA);
    
    // Swap bytes & extract 12-bit value
    uint16_t xy = ((raw_y >> 8) | (raw_y << 8)) >> 4;
    
    // Convert to voltage (assuming 3.3V reference)
    float voltage = xy * ADC_TO_VOLTAGE; 
    return voltage;
}

void close_i2c_bus(int address){
	close(address);
}
