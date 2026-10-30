#include <Arduino.h>

//==============================================================//
//                          Defines                             //
//==============================================================//

#define SAIDA   4      //Liga no pino 18 da ESP32 DevKit (e GND com GND)
#define BPM_MAX 600    //BPM máximo aceito

//==============================================================//
//                      Variaveis globais                       //
//==============================================================//

float bpm = 75;        //BPM atual (0 = sem sinal)

//==============================================================//
//                           setup                              //
//==============================================================//

void setup() {
  Serial.begin(115200);
  pinMode(SAIDA, OUTPUT);
  digitalWrite(SAIDA, LOW);
  Serial.println("Digite o BPM (ex.: 75) e Enter. 0 = sem sinal");
  Serial.println("Abaixo de 60 = bradicardia, 60 a 100 = normal, acima de 100 = taquicardia");
  Serial.println("Abaixo de 20 o monitor considera sem pulso");
  Serial.printf("BPM: %.0f (%.2f Hz)\n", bpm, bpm / 60);
}

//==============================================================//
//                            loop                              //
//==============================================================//

void loop() {
  //Lê o BPM digitado no terminal
  if (Serial.available()) {
    String s = Serial.readStringUntil('\n');
    s.trim();
    s.replace(",", ".");   //Aceita vírgula também
    if (s.length()) {
      float novo = s.toFloat();
      if ((isDigit(s[0]) || s[0] == '.') && novo >= 0 && novo <= BPM_MAX) {
        bpm = novo;
        Serial.printf("BPM: %.0f (%.2f Hz)\n", bpm, bpm / 60);
      } else Serial.printf("Valor invalido (0 a %d)\n", BPM_MAX);
    }
  }

  //Onda quadrada de 50% de duty: meio período = 60 s / bpm / 2 = 30000000 us / bpm
  static unsigned long ultimaMudanca = 0;
  static bool nivel = false;
  if (bpm <= 0) {
    nivel = false;
    digitalWrite(SAIDA, LOW);
  } else if (micros() - ultimaMudanca >= 30000000.0 / bpm) {
    ultimaMudanca = micros();
    nivel = !nivel;
    digitalWrite(SAIDA, nivel);
  }
}
