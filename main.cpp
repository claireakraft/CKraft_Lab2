


#include "mbed.h"
#include "CKraft_binaryutils.h"
#include "USBSerial.h"
#include <math.h>
#include <I2C.h>

#define PRESSURE (uint32_t)(1 << 0)
#define TEMPERATURE (uint32_t)(1 << 9)
// addresses for the calibration numbers
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
int16_t AC1, AC2, AC3, B1, B2, MB, MC, MD;
uint16_t AC4, AC5, AC6;
char Radd = 0xEF;
char Wadd = 0xEE;
int32_t B5;

// digital out pins 
DigitalOut ledg(LED3);
DigitalOut ledb(LED4);
DigitalOut TurnOn(p32);

EventFlags event_flags;
Ticker flipper;
Thread thread1;
Thread thread2;
USBSerial SERIAL;

I2C i2c(p31, p2);
int j = 1;

// this function is used by the ticker to flip back and forth whether the temperture
// pressure event flag was set
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

// this function takes in the two addresses that hold 8 bits and reads the information from 
// them, and then combinds them into a 16 bit number
int reading(const uint8_t add1, const uint8_t add2){
    char Radd = 0xEF;
    char Wadd = 0xEE;
    char outp1[1], outp2[1];

    // reading from the MSB address
    outp1[0] = add1;
    i2c.write(Wadd, outp1, 1, true);
    i2c.read(Radd, outp1, 1, false);

    //reading from the least significant bit address
    outp2[0] = add2;
    i2c.write(Wadd, outp2, 1, true);
    i2c.read(Radd, outp2, 1, false);

    // returning the 16 bit number 
    return (outp1[0] << 8) | outp2[0];

}

void read_pressure(){
    char input1[2], output0[1], output1[1], output2[1];
    int oss = 0;
    input1[0]= 0xF4; 
    input1[1]= 0x34 + (oss<<6);

    //Calculation variables initialized
    int32_t B6, X1, X2, X3, B3, p; 
    uint32_t  B4, B7;

    while(true){
        event_flags.wait_any(PRESSURE);
        SERIAL.printf("reading pressure\r\n");
        ledg = 0; // green led on
        thread_sleep_for(500);
        ledg = !ledg; // green led off 
        thread_sleep_for(500);

        // reading uncompensated pressure value 
        
        // writing 0x34 + (oss<<6) to the 0xF4 address
        i2c.write(Wadd, input1, 2, false);

        thread_sleep_for(4.5);
        
        // reading from the 0xF6, 0xF7, and 0xF8 addresses
        output0[0] = 0xF6;
        output1[0] = 0xF7;
        output2[0] = 0xF8;
        i2c.write(Wadd, output0, 1, true);
        i2c.read(Radd, output0, 1, false);
        i2c.write(Wadd, output1, 1, true);
        i2c.read(Radd, output1, 1, false);
        i2c.write(Wadd, output2, 1, true);
        i2c.read(Radd, output2, 1, false);
        // uncompensated temperature value
        int32_t UP = ((output0[0] << 16) + (output1[0] << 8) + output2[0]) >> (8 - oss);
        //SERIAL.printf("The UP pressure is = %i \r\n", UP);

        //Calculations for the true pressure
        B6 = B5 - 4000;
        //SERIAL.printf("B6 = %i\r\n", B6);
        X1 = (B2 *(B6*B6/pow(2,12)))/ pow(2, 11);
        //SERIAL.printf("X1 = %i\r\n", X1);
        X2 = AC2 * B6 / pow(2,11);
        //SERIAL.printf("X2 = %i\r\n", X2);
        X3 = X1 + X2;
        //SERIAL.printf("X3 = %i\r\n", X3);
        B3 = (((AC1*4 + X3) << oss) + 2) / 4;
        //SERIAL.printf("B3 = %i\r\n", B3);
        X1 = AC3 * B6/ pow(2,13);
        //SERIAL.printf("X1 = %i\r\n", X1);
        X2 = (B1*(B6*B6/pow(2,12))) /pow(2,16);
        //SERIAL.printf("X2 = %i\r\n", X2);
        X3 = ((X1+X2) +2) /pow(2,2) ;
        //SERIAL.printf("X3 = %i\r\n", X3);
        B4 = AC4 * (uint32_t)(X3 + 32768)/pow(2,15);
        //SERIAL.printf("B4 = %i\r\n", B4);
        B7 = ((uint32_t)UP - B3) * (50000 >> oss);
        //SERIAL.printf("B7 = %i\r\n", B7);
        if (B7 < 0x80000000){
            p = (B7 *2) / B4;
        }
        else{
            p = (B7/B4)*2;
        }
        X1 = (p/pow(2,8)) * (p/pow(2,8));
        //SERIAL.printf("X1 = %i\r\n", X1);
        X1 = (X1*3038) / pow(2,16);
        //SERIAL.printf("X1 = %i\r\n", X1);
        X2 = (-7357 *p) / pow(2,16);
        //SERIAL.printf("X2 = %i\r\n", X2);
        p = p + (X1 + X2 + 3791)/pow(2,4);
        //SERIAL.printf("the pressure is %i Pa\r\n", p);
    }    
}


void read_temperature(){
    char input3[2], input4[1], out1[1], out2[1];
    input3[0]= 0xF4;
    input3[1]= 0x2E;

    // calculation variables
    int32_t X1 = 0, X2 = 0, T = 0;
    while(true){
        event_flags.wait_any(TEMPERATURE);

        SERIAL.printf("reading temperature\r\n");
        ledb = 0; // blue led on 
        thread_sleep_for(500);
        ledb = !ledb; // blue led off
        thread_sleep_for(500);

        // reading uncompensated temperature value 
        i2c.write(Wadd, input3, 2, false);
        // writing 0x2E to the 0xF4 address
        thread_sleep_for(4.5);
        // reading from the 0xF6 and 0xF7 adresses
        out1[0] = 0xF6;
        out2[0] = 0xF7;
        i2c.write(Wadd, out1, 1, true);
        i2c.read(Radd, out1, 1, false);
        i2c.write(Wadd, out2, 1, true);
        i2c.read(Radd, out2, 1, false);
        // uncompensated temperature value
        uint32_t UT = (out1[0] << 8) + out2[0];
        //SERIAL.printf("the UT temperature is = %i \r\n", UT);

        // Calculations of true temperature
        X1 = (UT - AC6) * (AC5/ pow(2, 15));
        X2 = (MC *pow(2,11)) / (X1 + MD);
        B5 = X1 + X2;
        T = (B5 +8) / pow(2,4);
        //SERIAL.printf("X1 = %i \r\n", X1); 
        //SERIAL.printf("X2 = %i \r\n", X2);     
        //SERIAL.printf("B5 = %i \r\n", B5);     
        //SERIAL.printf("the temperature is = %i *0.1 C \r\n", T);              
    }
} 



// main() runs in its own thread in the OS
int main() {

    // setting high the SDA and SCL pins 
    TurnOn = 1;
    //checking is the controller is communicating to the right sensor
    char value[1];
    char out[1];
    value[0] = 0xD0;
    out[0] = 0x00;
    i2c.write(Wadd, value, 1, true);
    i2c.read(Radd, out, 1, false);
    if(out[0] == 0x55){
        SERIAL.printf("RIGHT sensor\r\n");
    }
    else{
        SERIAL.printf("WRONG sensor\r\n");
    }

    // reading all the calibration numbers    
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
    //printing to confirm them 
    SERIAL.printf("AC1 = %i \r\n", AC1);
    SERIAL.printf("AC2 = %i \r\n", AC2);
    SERIAL.printf("AC3 = %i \r\n", AC3);
    SERIAL.printf("AC4 = %i \r\n", AC4);  
    SERIAL.printf("AC5 = %i \r\n", AC5);   
    SERIAL.printf("AC6 = %i \r\n", AC6);
    SERIAL.printf("B1 = %i \r\n", B1);
    SERIAL.printf("B2 = %i \r\n", B2);
    SERIAL.printf("MB = %i \r\n", MB);
    SERIAL.printf("MC = %i \r\n", MC);
    SERIAL.printf("MD = %i \r\n", MD);
        

    // initializing threads and ticker
    thread1.start(read_pressure);
    thread2.start(read_temperature);
    flipper.attach(&flip, 3.0); // the address of the function to be attached (flip) and the interval (2 seconds)

    while (true) {

        thread_sleep_for(1000);
    }
    
}

