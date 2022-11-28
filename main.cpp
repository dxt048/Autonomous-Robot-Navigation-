#include "mbed.h"
#include "nRF24L01P.h"

BusOut display1(PC_4, PB_13, PB_14, PB_15, PB_1, PB_2, PB_12, PA_11);
BusOut display2(PC_3, PC_2, PB_7, PA_14, PC_12, PC_10, PD_2, PC_11);

nRF24L01P my_nrf24l01p(D11, D12, D13, D8, D7);    // mosi, miso, sck, csn, ce

BusOut motorControl(D2, D3, D4, D5);

PwmOut ENA(D6);
PwmOut ENB(D9);

//Timers and Threads
Thread displayingCoords;
Thread updateCoords;
Timer timerController;
Timer displayUpdateTimer;

//Cordinates and direction
int x;
int y;
int direction;

//Global variables
bool inMotion; //Tracks if robot is currently in the middle of moving
bool isTurning; //Tracks if robot is currently rotating
int motionSteps; //Tracks what step into its movement the robot is in (This is just for hardcoded movements)

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
        default:
            display = 0x79;
            break;
    }
    display = 0x6f;
    return display;
}

//Display coordinates on both 7-segment displays
void displaySegments(){
    while(true){
        display1 = displayHexValue(y);
        display2 = displayHexValue(x);
    }
}

//Update x,y coordinates depending on direction in motion
void updateCoordinates(){
    switch(direction){
        case 1:
            y = y + 1;
            break;
        case 2:
            x = x + 1;
            break;
        case 3:
            y-=1;
            break;
        case 4:
            x-=1;
            break;
    }
}

//Stops any robot movement and changes global bool variable to be false
void robotStopMovement() {
    motorControl = 0x00;
    inMotion = false;
    isTurning = false;
}

//Main robot movement function
void robotMotorMovements(int motorMovement){
    switch(motorMovement){
        case 1: //Stop 
            motorControl = 0x00;
            break;
        case 2: //Left
            motorControl = 0x09;
            if(direction == 1) {
                direction = 4;
                break;
            }
            direction-=1;
            break;
        case 3: //Right
            motorControl = 0x06;
            if(direction == 4) {
                direction = 1;
                break;
            }
            direction+=1;
            break;
        case 4: //Forward
            motorControl = 0x0A;
            break;
        case 5: //Reverse
            motorControl = 0x05;
            break;
    }
}

// main() runs in its own thread in the OS
int main()
{
    //Setting up reciever
    #define TRANSFER_SIZE   24
    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
    int txDataCnt = 0;
    int rxDataCnt = 0;
    my_nrf24l01p.powerUp();
    my_nrf24l01p.setRfOutputPower(-6);
    my_nrf24l01p.setTxAddress(DEFAULT_NRF24L01P_ADDRESS,DEFAULT_NRF24L01P_ADDRESS_WIDTH);
    my_nrf24l01p.setRxAddress(DEFAULT_NRF24L01P_ADDRESS,DEFAULT_NRF24L01P_ADDRESS_WIDTH);
    my_nrf24l01p.setAirDataRate(2000);
    // Display the (default) setup of the nRF24L01+ chip
    printf( "nRF24L01+ Frequency    : %d MHz\r\n",  my_nrf24l01p.getRfFrequency() );
    printf( "nRF24L01+ Output power : %d dBm\r\n",  my_nrf24l01p.getRfOutputPower() );
    printf( "nRF24L01+ Data Rate    : %d kbps\r\n", my_nrf24l01p.getAirDataRate() );
    printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", my_nrf24l01p.getTxAddress() );
    printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", my_nrf24l01p.getRxAddress() );
    my_nrf24l01p.setTransferSize( TRANSFER_SIZE );
    my_nrf24l01p.setReceiveMode();
    my_nrf24l01p.enable();

    //PWM for motors
    ENA.period(0.05);
    ENA=0.5;

    ENB.period(0.05);
    ENB=0.5;

    //Movement time at 50% duty cycle for 8 inches + 90 degree-ish turn
    double movementTime = 1.1;
    double turnTime = 0.25;

    //Initialize coords
    x = 1;
    y = 1;
    direction = 1;

    //Initialize global variables
    inMotion = false;
    isTurning = false;
    motionSteps = 1;

    //Start displaying segments continuously
    displayingCoords.start(displaySegments);

    //Initialize bool to end while loop
    bool endOfProgram = false;

    timerController.start();
    //While loop
    while (true) {
        //Check if enough time has pass to update coordinates
        if(inMotion && displayUpdateTimer.read() > movementTime/2){
            updateCoordinates();
            displayUpdateTimer.reset();
            displayUpdateTimer.stop();
        }
        //Check if enough time to have moved 8 inches has passed
        if(inMotion && timerController.read() > movementTime){
            robotStopMovement();
        }
        //Check if enough time to turn 90 degrees has passed
        if(isTurning && timerController.read() > turnTime){
            robotStopMovement();
        }

        //If robot is not in motion or is not turning, check switch statement for next set of instructions
        if (!inMotion && !isTurning) {
            switch(motionSteps) {
                case 1: //First step, move forward from (1,1) to (1,2)
                    timerController.reset();
                    displayUpdateTimer.start();
                    robotMotorMovements(4);
                    inMotion = true;
                    motionSteps+=1;
                    break;
                case 2: //Second step, turn from north to east <direction goes from 1 -> 2>
                    timerController.reset();
                    robotMotorMovements(3);
                    inMotion = true;
                    motionSteps+=1;
                    break;
                case 3: //Third step, move forward from (1,2) to (2,2)
                    timerController.reset();
                    displayUpdateTimer.start();
                    robotMotorMovements(4);
                    inMotion = true;
                    motionSteps+=1;
                    break;
                case 4: //Fourth step, turn from east to south <direction goes from 2 -> 3>
                    timerController.reset();
                    robotMotorMovements(3);
                    inMotion = true;
                    motionSteps+=1;
                    break;
                case 5: //Fifth step, move forward from (2,2) to (2,1)
                    timerController.reset();
                    displayUpdateTimer.start();
                    robotMotorMovements(4);
                    inMotion = true;
                    motionSteps+=1;
                    break;
                case 26: //End of hardcoded path, exit loop
                    endOfProgram = true;
                    break;
            }
        }

        if(endOfProgram){
            break;
        }
    }
}
