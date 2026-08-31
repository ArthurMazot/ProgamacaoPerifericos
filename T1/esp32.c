#include <Arduino.h>

char msg_tx[8] = {};

void setup(){
    Serial.begin(9600);
    delay(1000);
}

void loop(){
    uint8_t resp = 0;
    while(resp == 0) resp = getResp();

    switch(resp){
        case 0:
        case 1:
        case 2:
        default:;
    }


}

uint8_t getResp(){
    uint8_t dado = 0;
    if(Serial.available()>0)
        dado = Serial.read();

    delay(1000);
    return dado;
}

void sendResp(){

}