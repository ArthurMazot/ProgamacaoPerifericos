#include <Arduino.h>

#define MAX485_DE_RE 4

void printHexMessage(unsigned char values[], int tamanho);
void printHexByte(unsigned char b);
void calculateCRC(unsigned char *frame, int tamanho);

// half second wait for a reply
int TIMEOUT = 500;

// Dado a ser transmitido
unsigned char msg_tx[8] = {0x01, 0x04, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00};
unsigned char msg_sem_crc[6] = {0x01, 0x04, 0x00, 0x01, 0x00, 0x02};
unsigned char crc_saida[2];

// Dado a ser recebido
unsigned char msg_rx[12];

void setup() {
  // Serial0 somente para debug
  Serial.begin(9600);

  calculateCRC(msg_sem_crc, 6);
  msg_tx[6] = crc_saida[0];
  msg_tx[7] = crc_saida[1];
   

  // Serial1 será usada com o adaptador MAX485
  Serial1.begin(9600, SERIAL_8N1, 16, 17);

  // Pino que fica alternando entre transmitir ou receber
  pinMode(MAX485_DE_RE, OUTPUT);
  digitalWrite(MAX485_DE_RE, LOW);
  delay(1000);
}

int j=0;

void loop() {
  uint32_t startTime = 0;

  // Imprimindo na Serial0 a mensagem que vai ser enviada ao slave.
  Serial.print("TX: ");
  printHexMessage(msg_tx, sizeof(msg_tx) );

  // Agora começa o processo de envio via serial1 para o módulo MAX485
  // O pino DE-RE deve ser colocado em nível lógico alto na transmissão master -> slave.
  digitalWrite(MAX485_DE_RE, HIGH);
  delay( 10 );

  // A função Serial1.write com dois parâmetros tem a seguinte finalidade:
  // o primeiro parâmetro é o vetor e o segundo o tamanho do vetor....seria como fazer
  // um for, mas desta forma já envia todos os bytes do vetor.
  Serial1.write(msg_tx, sizeof(msg_tx) );
  Serial1.flush();

  // Importante, depois da transmissão, colocar o bit DE-RE em nível lógico zero,
  // habilitando assim o envio do slave -> master.
  digitalWrite(MAX485_DE_RE, LOW);

  Serial.print("RX: ");
  
  startTime = millis();

  // Os dados estão sendo recebidos e armazenados num vetor e posteriormente serão impressos
  // na Serial0.
  while ( millis() - startTime <= TIMEOUT ) {
    if (Serial1.available()) {
      msg_rx[j] = Serial1.read();
      j++;
    }
  }

  // Uma rotina para imprimir os dados na Serial0. Começa na posicao até a 9. Na zero tem 0x00 e na 10 tem 0x00.
  for(j=1; j<10; j++)
    printHexByte(msg_rx[j]);

  Serial.println();
  delay(2000);
  j=0;
}

// Funções para imprimir os bytes enviados na Serial0.

void printHexMessage(unsigned char values[], int tamanho) {
  for (int i = 0; i < tamanho; i++) {
    printHexByte(values[i]);
  }
  Serial.println();
}

void printHexByte(unsigned char b)
{
  Serial.print((b >> 4) & 0xF, HEX);
  Serial.print(b & 0xF, HEX);
  Serial.print(' ');
}

void calculateCRC(unsigned char *frame, int tamanho) {
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
  crc_saida[1] = (crc >> 8) & 0xFF;     // Separando a parte alta do CRC  
  crc_saida[0] = crc & 0xFF;
}