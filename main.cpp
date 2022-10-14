


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

I2C i2c(p14, p15);
//const int addr = 0x55;

int j = 1;

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
    char outp1[1], outp2[1];

    outp1[0] = add1;
    i2c.write(Wadd, outp1, 1, true);
    i2c.read(Radd, outp1, 1, false);

    outp2[0] = add2;
    i2c.write(Wadd, outp2, 1, true);
    i2c.read(Radd, outp2, 1, false);

    return (outp1[0] << 8) | outp2[0];

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
        int oss = 0;
        char input1[1], input2[1], output0[1], output1[1], output2[1];
        input1[0]= 0xF4;
        input2[0]= 0x34 + (oss<<6);
        i2c.write(Wadd, input1, 1, true);
        i2c.write(Wadd, input2, 1, false);

        thread_sleep_for(4.5);

        output0[0] = 0xF6;
        output1[0] = 0xF7;
        output2[0] = 0xF8;

        i2c.write(Wadd, output0, 1, true);
        i2c.read(Radd, output0, 1), false;

        i2c.write(Wadd, output1, 1, true);
        i2c.read(Radd, output1, 1, false);

        i2c.write(Wadd, output2, 1, true);
        i2c.read(Radd, output2, 1, false);

        int32_t UP = ((output0[0] << 16) + (output1[0] << 8) + output2[0]) >> (8 - oss);

        SERIAL.printf("pressure = %i \r\n", UP);
      
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
        char input3[1], input4[1], out1[1], out2[1];

        input3[0]= 0xF4;
        input4[0]= 0x2E;
        i2c.write(Wadd, input3, 1, true);
        i2c.write(Wadd, input4, 1, false);

        thread_sleep_for(4.5);

        out1[0] = 0xF6;
        out2[0] = 0xF7;
        i2c.write(Wadd, out1, 1, true);
        i2c.read(Radd, out1, 1, false);
        i2c.write(Wadd, out2, 1, true);
        i2c.read(Radd, out2, 1, false);

        uint32_t UT = (out1[0] << 8) + out2[0];
        SERIAL.printf("temp = %i \r\n", UT);

         
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
    if(out[0] == 0x55){
        SERIAL.printf("RIGHT sensor\r\n");
    }
    else{
        SERIAL.printf("WRONG sensor\r\n");
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

