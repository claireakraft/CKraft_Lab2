


#include "mbed.h"
#include "CKraft_binaryutils.h"
#include "USBSerial.h"

#define PRESSURE (uint32_t)(1 << 0)
#define TEMPERATURE (uint32_t)(1 << 9)



//DigitalOut led(LED1);
//DigitalOut flash(LED_RED);

#define SET (uint32_t *)0x50000508
#define CLEAR (uint32_t *)0x5000050C
#define DIR (uint32_t *)0x50000514
#define ping (uint8_t)16
#define pinr (uint8_t)24
#define pinb (uint8_t)6


EventFlags event_flags;
Ticker flipper;
Thread thread1;
Thread thread2;
USBSerial SERIAL;

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


void read_pressure(){
    uint32_t PressureFlag = 0;
    //setbit(DIR, ping);
   
    while(true){
        event_flags.wait_any(PRESSURE);

            SERIAL.printf("reading pressure\r\n");
            
            // actually turn on and off the LED
            setbit(CLEAR, ping); // turning on
            //thread_sleep_for(100);
            setbit(SET, ping); // turning off
            thread_sleep_for(500);
    }    
}


void read_temperature(){
    uint32_t TemperatureFlag = 0;
    
    //setbit(CLEAR, pinb);
    while(true){
        event_flags.wait_any(TEMPERATURE);

            SERIAL.printf("reading temperature\r\n");
            // actually turn on and off the LED
            setbit(CLEAR, pinb); // turning on
            //thread_sleep_for(100);
            setbit(SET, pinb); // turning off
            thread_sleep_for(500);
         
    }
}



// main() runs in its own thread in the OS
int main() {
    //thread1.start(mbed::callback(read_temperature));

    thread1.start(read_pressure);
    thread2.start(read_temperature);

    setbit(DIR, ping);
    setbit(DIR, pinb);
    setbit(SET, pinb);


    flipper.attach(&flip, 3.0); // the address of the function to be attached (flip) and the interval (2 seconds)
    

    while (true) {
        SERIAL.printf("j is %i\r\n", j);
        thread_sleep_for(1000);

    }
    
}

