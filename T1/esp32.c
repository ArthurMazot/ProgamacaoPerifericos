#include <Arduino.h>

const uint8_t comm[3][6] = {0x01, 0x04, 0x00, 0x01, 0x00, 0x01,  // Temperatura [0]
                            0x01, 0x04, 0x00, 0x02, 0x00, 0x01,  // Umidade [1]
                            0x01, 0x04, 0x00, 0x01, 0x00, 0x02}; // Temperatura e umidade [2]

const uint8_t resp_size[3] = {7, 7, 9}; // tamanho das mensagens recebidas do sensor

uint8_t msg_tx[8] = {};
uint8_t msg_rx[9] = {};

uint16_t calculateCRC(unsigned char *frame, int tamanho);
unsigned char getResp();

//==========================================//

void setup(){
  Serial.begin(9600);
  delay(1000);
}

//==========================================//

void loop(){
  uint16_t crc = 0;
  unsigned char resp = 0;
  while(resp < '0' || resp > '2') resp = getResp(); 
  resp -= '0';

  for(int i = 0; i < 6; i++) // coloca a mensagem de resposta no buffer de envio
    msg_tx[i] = comm[resp][i];
    
  crc = calculateCRC((unsigned char *)msg_tx, 6); // calcula o CRC da mensagem de resposta
  msg_tx[6] = crc & 0xFF; //Parte alta
  msg_tx[7] = crc >> 8; //Parte baixa

  // enviar a mensagem para o sensor
  // receber a mensagem do sensor

  // Apartir daqui não consigo testar, acima tudo fuinciona
  crc = calculateCRC((unsigned char *)msg_tx, resp_size[resp] - 2);// -2 porque são os 2 bytes do crc
    
  if(!((msg_tx[resp_size[resp] - 1] == (crc >> 8)) && (msg_tx[resp_size[resp] - 2] == (crc & 0xFF)))){ //Se a parte alta ou a parte baixa do crc forem diferentes das recebidas
    //Mensagem tem erro, repitir o envio ou mandar uma resposta dizendo que deu erro
  }

  //Provavelmente tem que mudar
  if(resp == 1 || resp == 0){
    Serial.println(msg_rx[3] | (msg_rx[4] << 8), HEX); //Temperatura ou Umidade, Dividir por 10 no front
  }

  else if(resp == 2){
    Serial.print(msg_rx[3] | (msg_rx[4] << 8));   //Temperatura, Dividir por 10 no front
    Serial.print(":"); //Separador para pegar no front
    Serial.println(msg_rx[5] | (msg_rx[6] << 8)); //Umidade, Dividir por 10 no front
  }
}

//==========================================//

unsigned char getResp(){ // pega a resposta do front pela serial
  unsigned char dado = 0;
  if(Serial.available()>0)
    dado = Serial.read();

  delay(1000);
  return dado; // converte o char recebido para int
}

//==========================================//

uint16_t calculateCRC(unsigned char *frame, int tamanho) { // Função pega do arquivo .c disponibilizado pelo professor
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
  return crc;
}
