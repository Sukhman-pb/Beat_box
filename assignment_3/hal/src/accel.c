#include "hal/accel.h"
#include "hal/i2c_bus.h"
#include "hal/helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#define X_THRESHOLD 6000
#define Y_THRESHOLD 5000
#define Z_THRESHOLD 8000
static bool init = false;
static int fileDescriptor;
static int x_axis = 0, y_axis = 0, z_axis = 0;
static bool chX=false, chY = false, chZ = false, isfirst = true;
pthread_t accelerometer;
static pthread_mutex_t dataLock = PTHREAD_MUTEX_INITIALIZER;

void* accelThread(void* arg){
    assert(init);
    (void) arg;
    while(init){
    /*count = (count+1)%5000; //so that even though the count becomes so high the z axis can change
    count_z = (count_z+1)%1000;*/
    int16_t x = abs(read_accel(fileDescriptor, REG_OUT_X_L, REG_OUT_X_H));
    int16_t y = abs(read_accel(fileDescriptor, REG_OUT_Y_L, REG_OUT_Y_H));
    int16_t z = abs(read_accel(fileDescriptor, REG_OUT_Z_L, REG_OUT_Z_H));
    pthread_mutex_lock(&dataLock);
    x_axis = x;
    y_axis = y;
    z_axis = z;
    if(x > X_THRESHOLD){
        chX = true;
    }
    if(y > Y_THRESHOLD){
        chY = true;
    }
    if(z > Z_THRESHOLD){
        chZ = true;
    }
    pthread_mutex_unlock(&dataLock);
    isfirst = false;
    }
    return NULL;
}
void accel_init(){
    assert(!init);
    fileDescriptor = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);
    uint8_t config_value = 0x50;  // ODR = 50 Hz, XYZ enabled (Refer to datasheet for control register settings)
    write_register_8(fileDescriptor, REG_CTRL1, config_value);
    init = true;
    pthread_create(&accelerometer, NULL, accelThread, NULL);
}

int16_t read_accel(int fileDescriptor, uint8_t low, uint8_t high){
    assert(init);
    uint8_t lower = read_register_8(fileDescriptor, low);
    uint8_t higher = read_register_8(fileDescriptor, high);
    return (int16_t)(((higher << 8) | lower))>>2; 
}

bool get_accel(int choice){
    bool re;
    if (choice == 1){
        pthread_mutex_lock(&dataLock);
        re =  chX;
        chX = false;
        pthread_mutex_unlock(&dataLock);
        return re;
    }
    else if (choice == 2){
        pthread_mutex_lock(&dataLock);
        re =  chY;
        chY = false;
        pthread_mutex_unlock(&dataLock);
        return re;
    }
    else if (choice == 3){
        pthread_mutex_lock(&dataLock);
        re =  chZ;
        chZ = false;
        pthread_mutex_unlock(&dataLock);
        return re;
    }
    re = false;
    return re;
}

void accel_close(){
    assert(init);
    pthread_cancel(accelerometer);
    pthread_join(accelerometer, NULL);
    init = false;
    close_i2c_bus(fileDescriptor);
}