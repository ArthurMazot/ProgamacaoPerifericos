#include <TinyGPS++.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

/*
 ***ATENÇÂO MÁXIMA***
 NADA FOI TESTADO

 Atenções:
  - Implementar as funções getLatDesntino() e getLngDestino() para retornar a latitude e 
  longitude do destino.
  - Função callback() -> Ler o comentário a cima da função.
  - Não entendi se o ESP pode funcionar com subcriber e publisher ao mesmo tempo, mas está 
  comentado ao lado de qual função é para o ESP funcionar como publisher e qual é para funcionar 
  como subscriber.
  - Falta implementar as funções dos componentes, e colocar elas no loop().
  - Dos slides do Terroso, falta a parte do BotTelegram e o "Configurando esp32 como ap ou sta".
*/

//(Começo) Defines
  //(Começo) GPS
  #define GPS_RX 20 //Tx do GPS
  #define GPS_TX 21 //Rx do GPS

  //Esses defines não devem existir, mas no momento eu não sei daonde vão vir essas informações
  #define LATITUDE -30.123456 // Latitude de referência (Santa Maria)
  #define LONGITUDE -51.654321 // Longitude de referência (Santa Maria)
  //(Fim) GPS

  //(Começo) MQTT
  #define DHTPIN 19 //Pino de dados do sensor DHT

  //Definições dos tópicos de publicação, provavelmente terão mais
  //AS de Artur e Sofia, mas se quiser colocar mais um S pra ficar ASS, pode colocar
  #define MQTT_ID "AS_esp32" //Nosso ID
  #define topico_pub_temp "AS/Temp"
  //(Fim) MQTT


  //(Começo) componentes
  #define pino_buzzer 18
  #define pino_botao_panico 17
  #define pino_botao_queda 16
  #define pino_gerador_de_sinais 15
  //(Fim) componentes
//(Fim) Defines



//(Começo) Definições de funções
  //(Começo) GPS
  double getLatDesntino(); //Retorna a latitude do destino
  double getLngDestino(); //Retorna a longitude do destino
  double getDistance(); //Retorna a distância entre a posição atual e o destino em metros
  //(Fim) GPS

  //(Começo) MQTT
  void conectaWifi();
  void conectaBroker();

  void publicaDados(); //ESP com Publisher
  void callback(char *topico, byte *msg, unsigned int tamanho); //ESP com Subscriber
  //(Fim) MQTT

  //(Começo) Componentes
  void buzzer();
  void botaoPanico();
  void botaoQueda();
  void geradorDeSinais();
  //(Fim) Componentes
//(Fim) Definições de funções

//(Começo) Variáveis globais
  //(Começo) GPS
  HardwareSerial GPSserial(1);
  TinyGPSPlus gps;
  //(Fim) GPS

  //(Começo) MQTT
  DHT dht(DHTPIN, DHT22);

  WiFiClient espClient;
  PubSubClient MQTT(espClient);

  static long long tempo=0;
  const char* ssid = "mototerroso"; //Mudar para o nosso wifi
  const char* password = "0123456789"; //Mudar para a senha do nosso wifi
  const char* mqtt_broker = "broker.emqx.io";
  const int mqtt_port = 1883;
  //(Fim) MQTT
//(Fim) Variáveis globais

void setup() {
  pinMode(pino_buzzer, OUTPUT);
  pinMode(pino_botao_panico, INPUT);
  pinMode(pino_botao_queda, INPUT);
  pinMode(pino_gerador_de_sinais, INPUT);

  Serial.begin(115200);
  GPSserial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  dht.begin();
  conectaWifi();
  MQTT.setServer(mqtt_broker, mqtt_port);
  MQTT.setCallback(callback); //ESP como Subscriber
  conectaBroker();
  delay(1000);
}

void loop() {
  //(Começo) GPS
    while (GPSserial.available() > 0){
      gps.encode(GPSserial.read());
    }

    double distance = 0;
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000){
      lastUpdate = millis();
      if(gps.location.isValid())
        distance = getDistance();
    }
  //(Fim) GPS

  //(Começo) BotTelegram
    //Não sei o que vai vir aqui no momento
  //(Fim) BotTelegram

  //(Começo) MQTT
    static long long pooling = 0;
    if(WiFi.status() != WL_CONNECTED) conectaWifi();
    if(!MQTT.connected()) conectaBroker();
    if(millis() - pooling > 10000){
      pooling = millis();
      publicaDados();
    }
    MQTT.loop();
  //(Fim) MQTT

  //(Começo) Componentes

  //(Fim) Componentes
}

//(Começo) Funções
  //(Começo) GPS
  double getLatDesntino() {
    //return ????
  }

  double getLngDestino() {
    //return ????
  }

  double getDistance(){ //Retorna a distância entre a posição atual e o destino em metros
    double LatDestino = LATITUDE;  //Trocar pela função getLatDesntino()
    double LngDestino = LONGITUDE; //Trocar pela função getLngDestino()
    return TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), LatDestino, LngDestino);
  }
  //(Fim) GPS

  //(Começo) MQTT
  //Funções fornecidas pelo Terroso, não mexer
  void conectaWifi(){
    Serial.println("Conectando a rede wifi!");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.println("Conectando a rede wifi....");
    }
  Serial.println("Conectado a rede wifi com sucesso!!!");
  }

  void conectaBroker(){
    while(!MQTT.connected()) {
      if (MQTT.connect(MQTT_ID))
        Serial.println("Conectado ao Broker!");

      else {
        Serial.print("Falha na conexão. O status é: ");
        Serial.print(MQTT.state());
        delay(2000);
      }
    }
  }
  void publicaDados(){ //Todos os topicos de publicação devem ser chamados aqui dentro
    float temp = dht.readTemperature(); 
    MQTT.publish(topico_pub_temp, String(temp).c_str());
  }

  //(callBack) Não entendi se essa função é só para printar no terminal ou se tem 
  //que retornar alguma coisa, então por enquanto só vai printar no terminal
  void callback(char *topico, byte *msg, unsigned int tamanho){
    Serial.print("Mensagem recebida do topico ");
    Serial.println(topico);
    Serial.print("Mensagem: ");
    for (int i = 0; i < tamanho; i++) 
      Serial.print((char) msg[i]);

    Serial.println();
    Serial.println("-----------------------");
  }
  //(Fim) MQTT
//(Fim) Funções