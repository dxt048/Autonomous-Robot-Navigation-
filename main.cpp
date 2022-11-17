#include "mbed.h"
#include <map>

BusOut display1(PC_4, PB_13, PB_14, PB_15, PB_1, PB_2, PB_12, PA_11);
BusOut display2(PC_3, PC_2, PB_7, PA_15, PA_14, PA_13, PC_12, PC_10);

DigitalOut in1(D5);
DigitalOut in2(D4);
DigitalOut in3(D3);
DigitalOut in4(D2);

PwmOut ENA(D8);
PwmOut ENB(D13);

//Timers and Threads
Thread displayingCoords;
Timer controlTime;

int x;
int y;

//Return hex value to display on seven segment
int displayHexValue(int number){
    int display = 0x00;
    switch (number){
        case 0: 
            display = 0x3F;
            break;
        case 1:
            display = 0x06;
            break;
        case 2: 
            display = 0x5B;
            break;
        case 3: 
            display = 0x4F;
            break;
        case 4: 
            display = 0x66;
            break;
        case 5: 
            display = 0x6D;
            break;
        case 6: 
            display = 0x7D;
            break;
        case 7: 
            display = 0x07;
            break;
        case 8: 
            display = 0x7F;
            break;
        case 9: 
            display = 0x6F;
            break;
    }
    return display;
}

//Display coordinates on both 7-segment displays
void displaySegments(){
    display1 = displayHexValue(x);
    display2 = displayHexValue(y);
}

//Functions to move robot
void stop(){
    in1 = 0;
    in2 = 0;
    in3 = 0;
    in4 = 0;
}

void forward(){
    ENA=0.5;
    ENB=0.5;
    in1 = 1;
    in2 = 0;
    in3 = 1;        
    in4 = 0;
}

void reverse(){
    ENA=0.5;
    ENB=0.5;
    in1 = 0;
    in2 = 1;
    in3 = 0;        
    in4 = 1;
}

void left(){
    ENA=0.5;
    ENB=0.5;
    in1 = 1;
    in2 = 0;
    in3 = 0;
    in4 = 1;
}

void right(){
    ENA=0.5;
    ENB=0.5;
    in1 = 0;
    in2 = 1;
    in3 = 1;
    in4 = 0;
}

// main() runs in its own thread in the OS
int main()
{
    //PWM for motors
    ENA.period(0.05);
    ENA=0.5;

    ENB.period(0.05);
    ENB=0.5;

    double movementTime = 0.8;
    double spinTime = 0.56;

    controlTime.start();
    forward();

    x = 1;
    y = 1;

    displayingCoords.start(displaySegments);
    

    while (true) {
        //Turn stop moving forward and turn left after 4 seconds
        if(controlTime.read() > 0.8){
            stop();
            controlTime.stop();
        }
        //After 2 seconds of turning left, stop moving
        if(controlTime.read() > 2){
            stop();
            controlTime.stop();
        }
    }
}

