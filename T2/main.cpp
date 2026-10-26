#include <Arduino.h>
#include <LiquidCrystal.h>
#include <DHT.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

//==============================================================//
//                          Defines                             //
//==============================================================//

#define WIFI_SSID      "Netvirtua801"                                       //Nome da rede
#define WIFI_SENHA     "pingoebombom"                                       //Senha da rede
#define MQTT_BROKER    "broker.emqx.io"
#define MQTT_ID        "AS_ESP32"                                           //AS de Artur e Sofia
#define TOPICO         "AS_Topicos"                                         //Se quiser colocar um S a mais pra ficar ASS, pode colocar
#define TELEGRAM_TOKEN "8890845312:AAEbZkFloXGsfQHHsYlCVUqlaOc-JHIR9TY"     //token do bot
#define TELEGRAM_CHAT  "8839309674"                                         //id do chat
#define INTERVALO_MQTT 5000                                                 //Tempo entre publicações no broker
#define TEMPO_AVISO    5000                                                 //Tempo que o aviso fica no LCD

#define LCD_RS          1
#define LCD_E           3
#define LCD_D4          4
#define LCD_D5          5
#define LCD_D6          6
#define LCD_D7          7
#define BOTAO_QUEDA     8
#define BOTAO_PANICO    9
#define DHT_DADOS       2
#define BUZZER          0
#define SINAL_BATIMENTO 10
#define GPS_RX          20
#define GPS_TX          21

//==============================================================//
//                      Variaveis globais                       //
//==============================================================//

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
DHT dht(DHT_DADOS, DHT22);
TinyGPSPlus gps;
WiFiClient wifi;
PubSubClient mqtt(wifi);

volatile unsigned long ultima = 0;      //Micros() da última batida
volatile unsigned long periodo = 0;     //Tempo entre as duas últimas batidas (us)
int bpm = 0;
int estado = 0;                         //0 = normal, 1 = bradicardia, 2 = taquicardia, 3 = sem pulso
const char *nomes[] = {"NORMAL", "BRADICARDIA", "TAQUICARDIA", "SEM PULSO"};
unsigned long tempoAviso = 0;           //Tempo em que o aviso está na tela

void IRAM_ATTR batida() { //Função que é chamada pela a interrupção
  unsigned long agora = micros();
  if (ultima) periodo = agora - ultima;
  ultima = agora;
}

void buzzerBip(int t) { //Ativa um ciclo do buzzer
  digitalWrite(BUZZER, HIGH);
  delay(t);
  digitalWrite(BUZZER, LOW);
  delay(t);
}

void telegram(const char *nome) { //Manda o evento e a posição do GPS para o bot do Telegram
  String url = String("https://api.telegram.org/bot" TELEGRAM_TOKEN "/sendMessage?chat_id=" TELEGRAM_CHAT "&text=") + nome + "%0A";
  if (gps.location.isValid()) url += String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
  else url += "GPS sem sinal";
  url.replace(" ", "+");
  WiFiClientSecure https;
  https.setInsecure();   //Não confere o certificado do Telegram
  HTTPClient http;
  http.begin(https, url);
  http.GET();
  http.end();
}

void evento(const char *nome) { //Publica o evento no broker e mostra no LCD
  mqtt.publish(TOPICO "/evento", nome);
  lcd.clear();
  lcd.print("EVENTO:");
  lcd.setCursor(0, 1);
  lcd.print(nome);
  telegram(nome);
  tempoAviso = millis();
}

//==============================================================//
//                           setup                              //
//==============================================================//

void setup() {
  //LCD
  lcd.begin(16, 2);

  //Botões e buzzer
  pinMode(BOTAO_QUEDA, INPUT_PULLUP);
  pinMode(BOTAO_PANICO, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  //DHT22
  dht.begin();

  //Define o pino que gera a interrupção, a função "batida" é chamada toda vez que o pino detectar um "RISING"
  pinMode(SINAL_BATIMENTO, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(SINAL_BATIMENTO), batida, RISING);

  //GPS
  Serial1.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  //WiFi (espera conectar) e broker MQTT
  lcd.print("Conectando WiFi");
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  mqtt.setServer(MQTT_BROKER, 1883);
  lcd.clear();
  lcd.print("Monitor ativo");
}

//==============================================================//
//                            loop                              //
//==============================================================//

void loop() {
  //Batimentos abaixo de 20 bpm ou 3s sem batida = sem pulso (bpm 0)
  bpm = periodo ? round(60000000.0 / periodo) : 0;   // periodo em us: 60 s / periodo = bpm
  if (bpm < 20 || micros() - ultima > 3000000UL) bpm = 0;
  estado = bpm == 0 ? 3 : bpm < 60 ? 1 : bpm > 100 ? 2 : 0;

  //Evento de batimento ao entrar em bradicardia, taquicardia ou sem pulso
  static int estadoAnterior = 3;  //Começa em "sem pulso" para não avisar ao ligar
  if (estado != estadoAnterior) { //Toda vez que muda para um estado diferente de "sem puslo", publica no broquer, telegram e ativa o buzzer
    estadoAnterior = estado;
    if (estado) {
      buzzerBip(200);
      evento(nomes[estado]);
    }
  }

  //Botões e buzzer
  if (!digitalRead(BOTAO_QUEDA)) {
    buzzerBip(300);
    evento("QUEDA");
  }
  if (!digitalRead(BOTAO_PANICO)){
    buzzerBip(100);
    evento("PANICO");
  }

  //Lê do GPS todos os bytes disponiveis
  while (Serial1.available()) gps.encode(Serial1.read());

  //Eventos e GPS ficam TEMPO_AVISO no LCD, no resto do tempo mostra o bpm
  static unsigned long ultimoGps = 0;
  if (millis() - tempoAviso > TEMPO_AVISO) {
    if (millis() - ultimoGps > 60000) {
      ultimoGps = tempoAviso = millis();
      lcd.clear();
      if (gps.location.isValid()) {
        lcd.printf("Lat: %.5f", gps.location.lat());
        lcd.setCursor(0, 1);
        lcd.printf("Lon: %.5f", gps.location.lng());
      } else lcd.print("GPS sem sinal");
    } else {
      lcd.setCursor(0, 0);
      lcd.printf("BPM: %-11d", bpm);
      lcd.setCursor(0, 1);
      lcd.printf("%-16s", nomes[estado]);
    }
  }

  //Tenta reconectar a cada 5s se caiu e/ou mantém a conexão ativa
  static unsigned long tentativa = 0;
  if (!mqtt.connected() && millis() - tentativa > 5000) {
    tentativa = millis();
    mqtt.connect(MQTT_ID);
  }
  mqtt.loop();

  //Envio periódico: bpm, temperatura e posição do GPS
  static unsigned long ultimoEnvio = 0;
  if (millis() - ultimoEnvio > INTERVALO_MQTT) {
    ultimoEnvio = millis();
    mqtt.publish(TOPICO "/bpm", String(bpm).c_str());
    mqtt.publish(TOPICO "/temperatura", String(dht.readTemperature(), 1).c_str());
    mqtt.publish(TOPICO "/gps", gps.location.isValid() ? (String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6)).c_str() : "sem sinal");
  }
}