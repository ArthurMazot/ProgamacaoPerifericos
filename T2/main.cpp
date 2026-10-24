#include <Arduino.h>
#include <LiquidCrystal.h>
#include <DHT.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <PubSubClient.h>

/* Monitor de batimentos e queda - ESP32-C3-DevKitM-1
 * LCD: VSS, RW e K no GND; VDD e A (com 220 ohm) no 5V; V0 no cursor de um pot. de 10k; D0 a D3 soltos.
 * Botões e buzzer ativo: um lado no pino, o outro no GND. DHT22: pull-up de 10k entre DATA e 3.3V.
 * Gerador de sinais: onda quadrada de 0 a 3.3V no SINAL_BATIMENTO (resistor de 1k em série).
 * GPS NEO-6M: VCC em 3.3V, GND, TX do GPS no GPS_RX e RX do GPS no GPS_TX (cruzado). */

#define WIFI_SSID      "Netvirtua801"            // WiFi de 2.4 GHz
#define WIFI_SENHA     "pingoebombom"
#define MQTT_BROKER    "broker.emqx.io"          // broker público, porta 1883
#define MQTT_ID        "AS_ESP32"   // troque por algo seu: precisa ser único no broker
#define TOPICO         "AS_Topicos"   // prefixo dos tópicos: troque por algo seu
#define INTERVALO_MQTT 5000                      // ms entre os envios
#define TEMPO_AVISO    5000                     // ms que um evento ou o GPS ficam no LCD

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

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
DHT dht(DHT_DADOS, DHT22);
TinyGPSPlus gps;
WiFiClient wifi;
PubSubClient mqtt(wifi);

volatile unsigned long ultima = 0;    // micros() da última batida
volatile unsigned long periodo = 0;   // tempo entre as duas últimas batidas (us)
int bpm = 0;
int estado = 0;                       // 0 = normal, 1 = bradicardia, 2 = taquicardia, 3 = sem pulso
const char *nomes[] = {"NORMAL", "BRADICARDIA", "TAQUICARDIA", "SEM PULSO"};
unsigned long avisoEm = 0;            // millis() do último aviso no LCD (evento ou GPS)

void IRAM_ATTR batida() {
  unsigned long agora = micros();
  if (ultima) periodo = agora - ultima;   // a primeira batida só marca o tempo
  ultima = agora;
}

void buzzerBip(int t) {
  digitalWrite(BUZZER, HIGH);
  delay(t);
  digitalWrite(BUZZER, LOW);
  delay(t);
}

// Evento: publica no broker, mostra no LCD e apita
void evento(const char *nome, int bip) {
  mqtt.publish(TOPICO "/evento", nome);
  lcd.clear();
  lcd.print("EVENTO:");
  lcd.setCursor(0, 1);
  lcd.print(nome);
  buzzerBip(bip);
  avisoEm = millis();
}

void setup() {
  // LCD
  lcd.begin(16, 2);

  // Botões e buzzer
  pinMode(BOTAO_QUEDA, INPUT_PULLUP);
  pinMode(BOTAO_PANICO, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  // DHT22
  dht.begin();

  // Sinal dos batimentos (pull-down: pino solto = sem pulso)
  pinMode(SINAL_BATIMENTO, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(SINAL_BATIMENTO), batida, RISING);

  // GPS (o NEO-6M já transmite a 9600 baud)
  Serial1.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  // WiFi (espera conectar) e broker MQTT
  lcd.print("Conectando WiFi");
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  mqtt.setServer(MQTT_BROKER, 1883);
  lcd.clear();
  lcd.print("Monitor ativo");
}

void loop() {
  // Batimentos: abaixo de 20 bpm ou 3 s sem batida = sem pulso (bpm 0)
  bpm = periodo ? round(60000000.0 / periodo) : 0;   // periodo em us: 60 s / periodo = bpm
  if (bpm < 20 || micros() - ultima > 3000000UL) bpm = 0;
  estado = bpm == 0 ? 3 : bpm < 60 ? 1 : bpm > 100 ? 2 : 0;

  // Evento de batimento: ao entrar em bradicardia, taquicardia ou sem pulso
  static int estadoAnterior = 3;   // começa em "sem pulso" para não avisar ao ligar
  if (estado != estadoAnterior) {
    estadoAnterior = estado;
    if (estado) evento(nomes[estado], 200);
  }

  // Botões
  if (!digitalRead(BOTAO_QUEDA)) evento("QUEDA", 100);
  if (!digitalRead(BOTAO_PANICO)) evento("PANICO", 300);

  // GPS: alimenta o decodificador com o que chegou na serial
  while (Serial1.available()) gps.encode(Serial1.read());

  // LCD: evento e GPS (1 vez por minuto) ficam TEMPO_AVISO na tela; no resto do tempo mostra o bpm
  static unsigned long ultimoGps = 0;
  if (millis() - avisoEm > TEMPO_AVISO) {
    if (millis() - ultimoGps > 60000) {
      ultimoGps = avisoEm = millis();
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

  // MQTT: tenta reconectar a cada 5 s se caiu e mantém a conexão ativa
  static unsigned long tentativa = 0;
  if (!mqtt.connected() && millis() - tentativa > 5000) {
    tentativa = millis();
    mqtt.connect(MQTT_ID);
  }
  mqtt.loop();

  // Envio periódico: bpm, temperatura e posição do GPS
  static unsigned long ultimoEnvio = 0;
  if (millis() - ultimoEnvio > INTERVALO_MQTT) {
    ultimoEnvio = millis();
    mqtt.publish(TOPICO "/bpm", String(bpm).c_str());
    mqtt.publish(TOPICO "/temperatura", String(dht.readTemperature(), 1).c_str());
    mqtt.publish(TOPICO "/gps", gps.location.isValid() ?
                 (String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6)).c_str() : "sem sinal");
  }
}