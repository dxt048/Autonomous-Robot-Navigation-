//main.cpp file+++++++++++++++++++++++++++++++++++++++++
#include "mbed.h"
#include "nRF24L01P.h"
#include <cstdio>
 
nRF24L01P transmitter(D11, D12, D13,D2,D3);   // mosi, miso, sck, csn, ce, irq
bool awaitingNavigationTime; //boolean tracking when the mbed is open to recieving navigation time
bool sentStarting, sentEnding; //boolean tracking if starting and ending points have been sent

int main() {
 
// The nRF24L01+ supports transfers from 1 to 32 bytes, but Sparkfun's
//  "Nordic Serial Interface Board" (http://www.sparkfun.com/products/9019)
//  only handles 4 byte transfers in the ATMega code.
    
    #define TRANSFER_SIZE   24
    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
    int txDataCnt = 0;
    int rxDataCnt = 0;
    transmitter.powerUp();
    transmitter.setRfOutputPower(-6);
    transmitter.setTxAddress((0x1D21372D90),DEFAULT_NRF24L01P_ADDRESS_WIDTH);
    transmitter.setRxAddress((0x1D21372D90),DEFAULT_NRF24L01P_ADDRESS_WIDTH);
    transmitter.setAirDataRate(2000);

    // Display the (default) setup of the nRF24L01+ chip
    printf( "nRF24L01+ Frequency    : %d MHz\r\n",  transmitter.getRfFrequency() );
    printf( "nRF24L01+ Output power : %d dBm\r\n",  transmitter.getRfOutputPower() );
    printf( "nRF24L01+ Data Rate    : %d kbps\r\n", transmitter.getAirDataRate() );
    printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", transmitter.getTxAddress() );
    printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", transmitter.getRxAddress() );
    transmitter.setTransferSize( TRANSFER_SIZE );
    transmitter.setReceiveMode();
    transmitter.enable();

    //Initialize booleans
    awaitingNavigationTime = false;
    sentStarting = false;
    sentEnding = false;
     
    while (1) {
        char command[2];
 
        printf("\nType the 2 character command to use (Commands: START(ST), PAUSE(PA), START POINTS (SC), END POINTS(EC)): ");
        scanf("%s", command);
        printf("%s\n", command);

        //Command code
        if(strcmp(command, "ST") == 0){    //Start command
            if (sentStarting && sentEnding) {
                snprintf(txData, 3, "%s00000000", command);
                transmitter.write(NRF24L01P_PIPE_P0, txData, sizeof txData);
                awaitingNavigationTime = !awaitingNavigationTime;
                printf("The robot should have started moving. If not, ensure starting and end points commands have been used.\n");
            } else {
                printf("Missing starting or end points. Please use commands to set each points.\n");
            }


        } else if(strcmp(command, "PA") == 0){ //Pause command
                snprintf(txData, 3, "%s00000000", command);
                transmitter.write(NRF24L01P_PIPE_P0, txData, sizeof txData);
            printf("The robot should have stopped moving if it wasn't already.\n");


        } else if(strcmp(command, "SC") == 0){ //Starting points command
            printf("Enter starting x-axis point [0 - 7]:");
            int startx, starty, direction;
            int hits;
            hits = scanf("%d", &startx); //Accept x-axis starting position
            if(hits == 1){  //If matching, continue to check if numbers are valid
                printf("%d\n", startx);
                if( startx < 0 || startx > 7){
                    printf("Incorrect input. Expected an integer between values 0 to 7.\n");
                } else {
                    printf("Enter starting y-axis point [0 - 7]:");
                    hits = scanf("%d", &starty); //Accept y-axis starting position
                    if(hits == 1){  //If matching, continue to check if numbers are valid
                        printf("%d\n", starty);
                        if( starty < 0 || starty > 7){
                            printf("Incorrect input. Expected an integer between values 0 to 7.\n");
                        } else {
                            printf("Enter direction the robot is initially facing (1 - NORTH, 2 - EAST, 3 - SOUTH, 4 - WEST):");
                            hits = scanf("%d", &direction); //Accept y-axis starting position
                            if(hits == 1){
                                printf("%d\n", direction);
                                if(direction < 1 || direction > 4){
                                    printf("Invalid direction.\n");
                                } else {
                                    printf("Sending points (%d, %d) facing direction %d to robot.\n", startx, starty, direction);
                                    char data[24];
                                    snprintf(data, 6, "%s%d%d%dK", command, startx, starty, direction);
                                    transmitter.write(NRF24L01P_PIPE_P0, data, sizeof data);
                                    sentStarting = true;
                                }
                            } else {
                                printf("\nIncorrect input. Expected input was an integer.\n");
                            }
                        }
                    } else {
                        printf("\nIncorrect input. Expected input was an integer.\n");
                    }
                }
            } else {
                printf("\nIncorrect input. Expected input was an integer.\n");
            }


        } else if(strcmp(command, "EC") == 0){ //Ending points command
            printf("Enter end x-axis point [0 - 7]:");
            int endx, endy;
            int hits;
            hits = scanf("%d", &endx); //Accept x-axis ending position
            if(hits == 1){  //If matching, continue to check if numbers are valid
                printf("%d\n", endx);
                if( endx < 0 || endx > 7){
                    printf("Incorrect input. Expected an integer between values 0 to 7.\n");
                } else {
                    printf("Enter end y-axis point [0 - 7]:");
                    hits = scanf("%d", &endy); //Accept y-axis ending position
                    if(hits == 1){  //If matching, continue to check if numbers are valid
                        printf("%d\n", endy);
                        if( endy < 0 || endy > 7){
                            printf("Incorrect input. Expected an integer between values 0 to 7.\n");
                        } else {
                            printf("Sending end points (%d, %d) to robot.\n", endx, endy);
                            char data[24];
                            snprintf(data, 5, "%s%d%dk", command, endx, endy);
                            transmitter.write(NRF24L01P_PIPE_P0, data, sizeof data);
                            sentEnding = true;
                        }
                    } else {
                        printf("\nIncorrect input. Expected input was an integer.\n");
                    }
                }
            } else {
                printf("\nIncorrect input. Expected input was an integer.\n");
            }
        } else {
            printf("%s is not a valid command.\n", command);
        }

        if(transmitter.readable()){
            if(awaitingNavigationTime){
                awaitingNavigationTime = !awaitingNavigationTime;
                rxDataCnt = transmitter.read(NRF24L01P_PIPE_P0, rxData, sizeof(rxData));
                printf("Navigation time data from robot: %s seconds\n", rxData);
            }
        }
    }
}
