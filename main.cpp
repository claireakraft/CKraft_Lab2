


#include "mbed.h"
#include "CKraft_binaryutils.h"
#include "USBSerial.h"

#define PRESSURE (uint32_t)(1 << 0)
#define TEMPERATURE (uint32_t)(1 << 9)
const uint8_t AC1_REG_MSB = 0xAA;
const uint8_t AC1_REG = 0xAB;
const uint8_t AC2_REG_MSB = 0xAC;
const uint8_t AC2_REG = 0xAD;
const uint8_t AC3_REG_MSB = 0xAE;
const uint8_t AC3_REG = 0xAF;
const uint8_t AC4_REG_MSB = 0xB0;
const uint8_t AC4_REG = 0xB1;
const uint8_t AC5_REG_MSB = 0xB2;
const uint8_t AC5_REG = 0xB3;
const uint8_t AC6_REG_MSB = 0xB4;
const uint8_t AC6_REG = 0xB5;
const uint8_t B1_REG_MSB = 0xB6;
const uint8_t B1_REG = 0xB7;
const uint8_t B2_REG_MSB = 0xB8;
const uint8_t B2_REG = 0xB9;
const uint8_t MB_REG_MSB = 0xBa;
const uint8_t MB_REG = 0xBB;
const uint8_t MC_REG_MSB = 0xBC;
const uint8_t MC_REG = 0xBD;
const uint8_t MD_REG_MSB = 0xBE;
const uint8_t MD_REG = 0xBF;
int16_t AC1, AC2, AC3, AC4, AC5, AC6, B1, B2, MB, MC, MD;
char Radd = 0xEF;
char Wadd = 0xEE;


DigitalOut ledg(LED3);
DigitalOut ledb(LED4);


EventFlags event_flags;
Ticker flipper;
Thread thread1;
Thread thread2;
USBSerial SERIAL;

I2C i2c(p31, p2);
const int addr = 0x55;

int j = 0;

void flip(){
    
    if(j == 0){
        event_flags.set(PRESSURE);
        j = 1;
    }
    else if(j == 1){
        event_flags.set(TEMPERATURE);
        j = 0;
    }  
}

int reading(const uint8_t add1, const uint8_t add2){
    char Radd = 0xEF;
    char Wadd = 0xEE;
    char outp1[2];
    i2c.write(Wadd, add1, 1);
    i2c.read(Radd, outp1[0], 1);

    i2c.write(Wadd, add2, 1);
    i2c.read(Radd, outp1[1], 1);

    return (outp1[0] << 8) | outp1[1];


}

void read_pressure(){
   
    while(true){
        event_flags.wait_any(PRESSURE);

        //SERIAL.printf("reading pressure\r\n");
            
        // actually turn on and off the LED
        ledg = 0;
        thread_sleep_for(500);
        ledg = !ledg;
        thread_sleep_for(500);

        // reading uncompensated pressure value 
        i2c.write(0xF4, 0x2E, 1);
        char output0[1];
        char output1[1];
        char output2[1];
        i2c.write(Wadd, 0xF6, 1);
        i2c.read(Radd, output0, 1);

        i2c.write(Wadd, 0xF7, 1);
        i2c.read(Radd, output1, 1);

        i2c.write(Wadd, 0xF8, 1);
        i2c.read(Radd, output2, 1);

        int32_t UP = ((output[0] << 16) + (output1[0] << 8) + output2[0]) >> 8;

        SERIAL.printf("pressure = %i", UP);
      
    }    
}


void read_temperature(){
    
    while(true){
        event_flags.wait_any(TEMPERATURE);

        //SERIAL.printf("reading temperature\r\n");
        // actually turn on and off the LED
        ledb = 0;
        thread_sleep_for(500);
        ledb = !ledb;
        thread_sleep_for(500);

        char output1[1];
        uint8_t Var1 = 0x2E;
        uint8_t Var2 = 0xF4;
        i2c.write(Var2, Var1, 1);

        wait(4.5);

        uint8_t Var3 = 0xF6;
        char output3[1];
        uint8_t Var4 = 0xF7;
        char output4[1];
        i2c.write(Wadd, VAR3, 1);
        i2c.read(Radd, output3, 1);

        i2c.write(Wadd, VAR4, 1);
        i2c.read(Radd, output3, 1);

        uint32_t UT = (output3[0] << 8) + output4[0];
        SERIAL.printf("temp = %i", UT);

         
    }
}



// main() runs in its own thread in the OS
int main() {

    thread1.start(read_pressure);
    thread2.start(read_temperature);
    flipper.attach(&flip, 3.0); // the address of the function to be attached (flip) and the interval (2 seconds)
    
    char Radd = 0xEF;
    char Wadd = 0xEE;
    char value[10];
    char out[10];
    value[0] = 0x00;
    out[0] = 0x00;
    i2c.write(Wadd, value, 1);
    i2c.read(Radd, out, 1);
    if(out[0] = 0x55){
        SERIAL.printf("RIGHT sensor");
    }
    else{
        SERIAL.printf("WRONG sensor");
    }
        
    AC1 = reading(AC1_REG_MSB, AC1_REG);
    AC2 = reading(AC2_REG_MSB, AC2_REG);
    AC3 = reading(AC3_REG_MSB, AC3_REG);
    AC4 = reading(AC4_REG_MSB, AC4_REG);
    AC5 = reading(AC5_REG_MSB, AC5_REG);
    AC6 = reading(AC6_REG_MSB, AC6_REG);
    B1 = reading(B1_REG_MSB, B1_REG);
    B2 = reading(B2_REG_MSB, B2_REG);
    MB = reading(MB_REG_MSB, MB_REG);
    MC = reading(MC_REG_MSB, MC_REG);
    MD = reading(MD_REG_MSB, MD_REG);

    

    while (true) {

        thread_sleep_for(1000);
    }
    
}

