/***********************************************************************************/
/*******PROJETO DE MONITORAMENTO IOT PARA A DISCIPLINA DE MICROCONTROLADORES********/
/************************UNIVERSIDADE DE PERNAMBUCO*********************************/
/***************************Data:28/06/2019*****************************************/
/***********************************************************************************/

/************************* Inclusão das Bibliotecas *********************************/
#include <WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>

/************************* Conexão WiFi*********************************/
#define WIFI_SSID   "Nome da sua rede"
#define WIFI_PASS   "senha"

/********************* Credenciais Adafruit io *************************/
#define IO_USERNAME  "seu user Adafruit"
#define IO_KEY       "Key Adafruit"

/********************** Variaveis globais *******************************/
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, "io.adafruit.com", 1883 , IO_USERNAME, IO_KEY);
int led = 2;
#define umidade 32
#define luminosidade 35
#define nivel 33

/****************************** Declaração dos Feeds ***************************************/

/* feed responsavel por receber os dados da nossa dashboard */
Adafruit_MQTT_Subscribe _led = Adafruit_MQTT_Subscribe(&mqtt, "isabellagalvao/f/led", MQTT_QOS_1);

/* feed responsavel por enviar os dados do sensor para nossa dashboard */
Adafruit_MQTT_Publish   _umidade = Adafruit_MQTT_Publish(&mqtt, "isabellagalvao/f/umidade", MQTT_QOS_1);
Adafruit_MQTT_Publish   _luminosidade = Adafruit_MQTT_Publish(&mqtt, "isabellagalvao/f/luminosidade", MQTT_QOS_1);
Adafruit_MQTT_Publish   _nivel = Adafruit_MQTT_Publish(&mqtt, "isabellagalvao/f/nivel", MQTT_QOS_1);


void setup() {  
  /* Definindo Saídas e Entradas */
  pinMode(led, OUTPUT);
  pinMode(umidade, INPUT);
  pinMode(luminosidade, INPUT);
  pinMode(nivel, INPUT);

  /* Comunicação Serial*/
  Serial.begin(115200);
  
  /* Conexão Wi-Fi*/
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(250);
  }
  /*Compilar Wireless*/
  bootRemOtA();
  
  /*Inicializando o protocolo MQTT*/
  initMQTT();
}

void loop() {
  /* Chamando função que compila codigo remotamente */
  ArduinoOTA.handle();
  
  /*Chamando função que conecta ao Adafruit*/
  conectar_broker();
  
  /*carregandos pacotes*/
  mqtt.processPackets(5000);
  
  /* Chamando funçoes de envio de dados para o broker*/
  /* São necessários 2s de intervalo entre o envio/recebimento de dados para a contra FREE da Adafruit*/
  umidade_publish();
  luminosidade_publish();
  nivel_publish();
  delay(2000);
}

/* Função que inicia protocolo MQTT*/
void initMQTT() {
  _led.setCallback(led_callback);
  mqtt.subscribe(&_led);
}

/* Função que conecta ao broker */
void conectar_broker() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }
  Serial.println("Conectando - se ao CloudMQTT...");
  uint8_t num_tentativas = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Falha. Tentando se reconectar em 1 segundos.");
    mqtt.disconnect();
    delay(1000);
    num_tentativas--;
  }
}

/************************** Função que recebe informações do Broker***********************************************/
void led_callback(char *data, uint16_t len) {
  String prinToSerial = data;

  if (prinToSerial == "ON") {
    digitalWrite(led, HIGH);
  }
  else
  {
    digitalWrite(led, LOW);
  }

  Serial.print("Servidor: ");
  Serial.println(prinToSerial);
  Serial.println("valor metodo");
  Serial.println(prinToSerial.toInt());
}

/************************** Funções de envio de informações para o Broker***********************************************/
void umidade_publish(void) {
  int valor = map(analogRead(umidade), 4095, 0, 10, 100); //traz a leitura do sensor para o range de 0 a 100

Serial.print("\n valor umidade: ");
Serial.print(valor);
 
  if (!_umidade.publish(valor)) {                         //Envia e verifica o recebimento do dado
    Serial.print(F("\n Falha ao enviar umidade!"));
  }
  else{
    Serial.println(F("\n umidade enviado!"));             //Tratamento de erro
  }
}

void luminosidade_publish(void) {
  int valor = analogRead(luminosidade);
  char iconSun[] = "w:day-sunny";                         
  char iconCloud[] = "cloud";

Serial.print("\n valor luminosidade: ");
Serial.print(valor);

  if (valor >= 3500) {
    if(_luminosidade.publish(iconSun)){
      Serial.print("\n luminosidade enviada - ");
      Serial.print("ensolarado!");
    }
    else{
      Serial.print("\nluminosidade NÃO enviada");
    }
  }
  else{
    if(_luminosidade.publish(iconCloud)){
      Serial.print("\nluminosidade enviada - ");
      Serial.print(" nublado!");
    }
    else{
      Serial.print("luminosidade NÃO enviada");
    }
  }
}

void nivel_publish(void) {
  int valorAnalog = analogRead(nivel);
  Serial.print("\n ANALOG NIVEL: ");
  Serial.print(valorAnalog);
  
  int valor = map(analogRead(nivel), 2230, 0, 100, 0);

  Serial.print("\n valor nivel: ");
  Serial.print(valor);
 
  if (!_nivel.publish(valor)) {
    Serial.print(F("\n Falha ao enviar nivel!"));
//    delay(1000);
  }
  else{
    Serial.println(F("\n nivel enviado!"));
  }
}

/***************************** Função que faz compilação por meio da rede Wi-Fi ************************************/
void bootRemOtA(){
//  Serial.println("Booting");
//  WiFi.mode(WIFI_STA);
//  WiFi.begin(WIFI_SSID, WIFI_PASS);
//  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
//    Serial.println("Connection Failed! Rebooting...");
//    delay(5000);
//    ESP.restart();
//  }

  // Porta padrão 3232
   ArduinoOTA.setPort(3232);

  // Hostname padrão esp3232-[MAC]
  ArduinoOTA.setHostname("myesp32");

  ArduinoOTA.setPassword("admin");

  // A senha também pode ser definida por MD5
  // Gerador de chave MD5: https://www.md5hashgenerator.com/
  //MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  ArduinoOTA.setPasswordHash("dc7161be3dbf2250c8954e560cc35060");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
