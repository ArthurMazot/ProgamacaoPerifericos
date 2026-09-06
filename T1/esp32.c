#include <Arduino.h>

const char comm[3][6] = {0x01, 0x04, 0x00, 0x01, 0x00, 0x01,  // Temperatura [0]
                         0x01, 0x04, 0x00, 0x02, 0x00, 0x01,  // Umidade [1]
                         0x01, 0x04, 0x00, 0x01, 0x00, 0x02}; // Temperatura e umidade [2]

const uint8_t resp_size[3] = {7, 7, 9}; // tamanho das mensagens recebidas do sensor

char msg_tx[8] = {};
char msg_rx[9] = {};

//==========================================//

void setup(){
    Serial.begin(9600);
    delay(1000);
}

//==========================================//

void loop(){
    uint8_t resp = 0;
    while(resp == 0) resp = getResp(); 

    if(resp < 0 && resp > 2){ // verifica se a resposta é válida
        // colocar alguma coisa aqui
    }

    for(int i = 0; i < 6; i++) // coloca a mensagem de resposta no buffer de envio
            msg_tx[i] = comm[resp][i];
        
    calculateCRC((unsigned char *)msg_tx, 6); // calcula o CRC da mensagem de resposta

    // enviar a mensagem para o sensor
    // receber a mensagem do sensor
    // Verificar o CRC da mensagem recebida (Descobrir se tem que fazer, se sim como fazer)

    uint16_t high_low = 0;
    //Provavelmente tem que mudar
    if(resp == 1 || resp == 0){
      high_low = (msg_rx[3] << 8) | msg_rx[4];//Temperatura ou Umidade
      Serial.write(high_low); // Dividir por 10 no front
    }

    else if(resp == 2){
      high_low = (msg_rx[3] << 8) | msg_rx[4]; //Temperatura
      Serial.write(high_low); // Dividir por 10 no front
      high_low = (msg_rx[5] << 8) | msg_rx[6]; //Umidade
      Serial.write(high_low); // Dividir por 10 no front
    }

}

//==========================================//

uint8_t getResp(){ // pega a resposta do front pela serial
    uint8_t dado = 0;
    if(Serial.available()>0)
        dado = Serial.read();

    delay(1000);
    return dado - '0'; // converte o char recebido para int
}

//==========================================//

void calculateCRC(unsigned char *frame, int tamanho) { // Função pega do aquivo .c disponibilizado pelo professor
  unsigned int crc = 0xFFFF;            // Initialize CRC to 0xFFFF
  for (int n = 0; n < tamanho; n++) {
    crc ^= frame[n];                    // XOR the frame byte with the CRC
    for (int m = 0; m < 8; m++) {
      if (crc & 0x0001) {               // Check if the LSB of the CRC is 1
        crc >>= 1;                      // Right shift the CRC
        crc ^= 0xA001;                  // XOR the CRC with the polynomial 0xA001
      } else {
        crc >>= 1;                      // Right shift the CRC
      }
    }
  }
  msg_tx[7] = (crc >> 8) & 0xFF;     // Separando a parte alta do CRC  
  msg_tx[6] = crc & 0xFF;
}
