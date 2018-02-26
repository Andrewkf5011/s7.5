#include <mbed.h>
#include <EthernetInterface.h>
#include <rtos.h>
#include <mbed_events.h>
#include <FXOS8700Q.h>

Thread eventhandler;
EventQueue eventqueue;

/* YOU will have to hardwire the IP address in here */
SocketAddress server("192.168.1.136",65500);
EthernetInterface eth;
UDPSocket udp;
char buffer[512];
char line[80];
void send(char *m) {
    nsapi_size_or_error_t r = udp.sendto( server, m, strlen(m));
}
/* Set up i2c bus and accelerometer */
I2C i2c(PTE25, PTE24);
FXOS8700QAccelerometer acc(i2c, FXOS8700CQ_SLAVE_ADDR1);
FXOS8700QMagnetometer  mag(i2c, FXOS8700CQ_SLAVE_ADDR1);

/* Input from Potentiometers */
AnalogIn  left(A0);
AnalogIn right(A1);

void sensor_poll(void) {
    motion_data_units_t data;
    buffer[0]='\0';

    sprintf(line,"POT 1:%f\n",left.read());
    strcat(buffer,line);
    sprintf(line,"POT 2:%f\n",right.read());
    strcat(buffer,line);

    acc.getAxis(data);
    sprintf(line, "X:%f\nY:%f\nZ:%f\n", data.x, data.y, data.z);
    strcat(buffer,line);
    send(buffer);
}

int main() {
    acc.enable();
    mag.enable();

    printf("conecting \n");
    eth.connect();
    const char *ip = eth.get_ip_address();
    printf("IP address is: %s\n", ip ? ip : "No IP");

    udp.open( &eth);
    SocketAddress source;
        printf("sending messages to %s/%d\n",
                server.get_ip_address(),  server.get_port() );

    eventhandler.start(callback(&eventqueue, &EventQueue::dispatch_forever));

    eventqueue.call_every(20,sensor_poll);

    while(1){}
}
