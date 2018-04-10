#include <mbed.h>
#include <EthernetInterface.h>
#include <rtos.h>
#include <mbed_events.h>
#include <FXOS8700Q.h>
#include <string>
#include "communications.h"

//Thread handlers.
Thread eventhandler;
EventQueue eventqueue;

//[server] This is the server being communicated with.
//The IP address must be inputted manually.
SocketAddress server("192.168.70.15",65500);
//[serverCommunicator] Handles communications with the server.
ServerCommunicator serverCommunicator(server);
//[messageProcessor] Processes the messages to be sent to the server.
MessageProcessor messageProcessor;

/* Set up i2c bus and accelerometer */
I2C i2c(PTE25, PTE24);
FXOS8700QAccelerometer acc(i2c, FXOS8700CQ_SLAVE_ADDR1);
FXOS8700QMagnetometer  mag(i2c, FXOS8700CQ_SLAVE_ADDR1);

/* Input from Potentiometers */
AnalogIn  left(A0);
AnalogIn right(A1);

char buffer[512];
char line[80];

void sensor_poll(void)
{
    //Get the accelerometer data.
    motion_data_units_t data;
    acc.getAxis(data);


    buffer[0]='\0';

    sprintf(line,"POT 1:%f\n",left.read());
    strcat(buffer,line);
    sprintf(line,"POT 2:%f\n",right.read());
    strcat(buffer,line);

    sprintf(line, "X:%f\nY:%f\nZ:%f\n", data.x, data.y, data.z);
    strcat(buffer,line);

    serverCommunicator.sendMessage(buffer);
}

int main()
{
    acc.enable();
    mag.enable();

    eventhandler.start(callback(&eventqueue, &EventQueue::dispatch_forever));

    eventqueue.call_every(20,sensor_poll);

    while(1){}
}
