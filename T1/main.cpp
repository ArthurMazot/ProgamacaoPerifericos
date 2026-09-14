#include <Arduino.h>

#define RXD2 16
#define TXD2 17
#define DE_RE 4


/** 
  Sophia -> Corrigido Leitura, estava em TX

  !!!! Corrigir identação, veio esquisita do PC lab
*/
const uint8_t comm[3][6] = {
  {0x01, 0x04, 0x00, 0x01, 0x00, 0x01},
  {0x01, 0x04, 0x00, 0x02, 0x00, 0x01},
  {0x01, 0x04, 0x00, 0x01, 0x00, 0x02}
};

const uint8_t resp_size[3] = {7, 7, 9};

uint8_t comando[8];
uint8_t resposta[9];

uint16_t calculateCRC(uint8_t *frame, int tamanho);
unsigned char getResp();

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  pinMode(DE_RE, OUTPUT);
  digitalWrite(DE_RE, LOW);
  delay(1000);
}

void loop() {
  unsigned char resp = 0;

  while (resp < '0' || resp > '2'){
    resp = getResp();
  }
    
  resp -= '0';

  for (int i = 0; i < 6; i++){
    comando[i] = comm[resp][i];
  }
    

  uint16_t crc = calculateCRC(comando, 6);
  comando[6] = crc & 0xFF;
  comando[7] = crc >> 8;

  while (Serial2.available()){
    Serial2.read();
  }
    
  digitalWrite(DE_RE, HIGH);
  delay(2);

  Serial2.write(comando, sizeof(comando));
  Serial2.flush();

  digitalWrite(DE_RE, LOW);
  delay(200);

  int j = 0;
  while (Serial2.available() && j < resp_size[resp]) {
    resposta[j] = Serial2.read();
    j++;
  }

  if (j != resp_size[resp]) return;

  crc = calculateCRC(resposta, resp_size[resp] - 2);

  if (resposta[resp_size[resp] - 2] != (crc & 0xFF) ||
      resposta[resp_size[resp] - 1] != (crc >> 8)) return;

  if (resp == 0 || resp == 1) {
    uint16_t valor = (resposta[3] << 8) | resposta[4];
    Serial.println(valor);
  }

  else if (resp == 2) {
    uint16_t temperatura = (resposta[3] << 8) | resposta[4];
    uint16_t umidade = (resposta[5] << 8) | resposta[6];

    Serial.print(temperatura);
    Serial.print(":");
    Serial.println(umidade);
  }
}

unsigned char getResp() {
  unsigned char dado = 0;

  if (Serial.available() > 0)
    dado = Serial.read();

  delay(100);

  return dado;
}

uint16_t calculateCRC(uint8_t *frame, int tamanho) {
  uint16_t crc = 0xFFFF;

  for (int n = 0; n < tamanho; n++) {
    crc ^= frame[n];

    for (int m = 0; m < 8; m++) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      }

      else {
        crc >>= 1;
      }
    }
  }

  return crc;
}