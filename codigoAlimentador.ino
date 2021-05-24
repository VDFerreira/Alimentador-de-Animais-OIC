//vibra
#define VIBRA 16
#define LIGA 1
#define DESLIGA 0
#define MICROSWITCH 5
// PINOS SENSOR SONICO
const int trigPin = 2;  //D4
const int echoPin = 0;  //D3
#include <Servo.h>
Servo myservo;  //inicial objeto do servo

#include <EEPROM.h>
#include <NTPClient.h>//bibilioteca que cuida de obter o horario do ntp
#include <WiFiUdp.h>
#include <ESP8266WiFi.h> // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
// define o id do mqtt e topicos para publicação e inscrição

#define TOPICO_SUBSCRIBE   "sendArnieAGORA"     //tópico MQTT de escuta
#define TOPICO_SUBSCRIBE_FLAGA "sendArnieLIGAA"     //tópico MQTT de escuta
#define TOPICO_SUBSCRIBE_FLAGB "sendArnieLIGAB"     //tópico MQTT de escuta
#define TOPICO_SUBSCRIBE_FLAGC "sendArnieLIGAC"     //tópico MQTT de escuta
#define TOPICO_SUBSCRIBE_A "sendArnieHORAA"     //tópico MQTT de escuta
#define TOPICO_SUBSCRIBE_B "sendArnieHORAB"     //tópico MQTT de escuta
#define TOPICO_SUBSCRIBE_C "sendArnieHORAC"     //tópico MQTT de escuta
#define TOPICO_PUBLISH     "reciArniePETfeed"    //tópico MQTT de envio de informações para Broker
#define ID_MQTT  "arniepetfeed"     //id mqtt (para identificação de sessão)
int last_hour;
byte time_A;
byte time_B;
byte time_C;
byte updateCycle = 0;
byte time_A_flag = 0;
byte time_B_flag = 0;
byte time_C_flag = 0;
bool DOSAR_A_FLAG = 0;
bool DOSAR_B_FLAG = 0;
bool DOSAR_C_FLAG = 0;
const char* SSID = "nomeWiFi"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "senhaWiFi"; // Senha da rede WI-FI que deseja se conectar
const char* BROKER_MQTT = "broker.emqx.io"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = -10800;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);//iniciliza o cliente com um offset de -3H(60seg*60min*3hora = -10800) NPT

void setup() {
  myservo.attach(12); 
  myservo.write(180); //posicao inicial do servo 180 ou seja horizontal
  pinMode(MICROSWITCH, INPUT_PULLUP); 
  pinMode(VIBRA, OUTPUT); //vibrador
  digitalWrite(VIBRA, DESLIGA);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
  Serial.begin(115200);//comunicaçao serial
  EEPROM.begin(20);
  time_A = EEPROM.read(1);
  time_B = EEPROM.read(2);
  time_C = EEPROM.read(3);
  time_A_flag = EEPROM.read(4);
  time_B_flag = EEPROM.read(5);
  time_C_flag = EEPROM.read(6);
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");
  reconectWiFi();
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
  MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
  timeClient.begin();//começa accesar cliente ntp
}

void loop() {
  updateCycle++;
  if (updateCycle == 11)updateCycle = 0;
  delay(1000);
  //se não há conexão com o WiFI, a conexão é refeita
  reconectWiFi();
  //se não há conexão com o Broker, a conexão é refeita
  reconnectMQTT();
  //envia o status de todos os outputs para o Broker no protocolo esperado
  EnviaEstadoOutputMQTT();
  //mantem vivo a comunicação com broker MQTT
  MQTT.loop();
  timeClient.update();

  Serial.print("time from ntp ->");  Serial.println(timeClient.getHours());
  // caso tempo esteja habilitado resetar status prar dosar no dia seguinte no mesmo horario
//  caso nao tenha dosado ainda, e esteja habilitado para fazer isso, continua em 1 , na reseta
  if (timeClient.getHours() != last_hour) {
    if (time_A_flag == 2)time_A_flag = 1;
    if (time_B_flag == 2)time_B_flag = 1;
    if (time_C_flag == 2)time_C_flag = 1;
  }
  last_hour = timeClient.getHours();

  //quando um horario programado bate, verificamos se ja foi atendido:
  // cas ja foi atendio o pedido de alimentaçao  mudamos o status para 2 ate a hora mudar e ter um reset
  // caso nao tenha um pedido de alimentaça o status fica em 0
  // caso e hora de alimentar o status muda para 1 ate terminar de alimentar

  Serial.print("A flag:");  Serial.print(time_A_flag);
  Serial.print(" time A:"); Serial.println(time_A);
  Serial.print("B flag:");  Serial.print(time_B_flag);
  Serial.print(" time B:"); Serial.println(time_B);
  Serial.print("C flag:");  Serial.print(time_C_flag);
  Serial.print(" time C:"); Serial.println(time_C);

  //caso esteja habilitado
  if (time_A_flag != 0)
    //libera para dosagem, agora espera animal se aproximar
    if (timeClient.getHours() == time_A && time_A_flag == 1 )  time_A_flag = alimentarQuandoProximo("HORARIO A");


  //caso esteja habilitado
  if (time_B_flag != 0)
    //libera para dosagem, agora espera animal se aproximar
    if (timeClient.getHours() == time_B && time_B_flag == 1 ) time_B_flag = alimentarQuandoProximo("HORARIO B");


  //caso esteja habilitado
  if (time_C_flag != 0)
    //libera para dosagem, agora espera animal se aproximar
    if (timeClient.getHours() == time_C && time_C_flag == 1 ) time_C_flag = alimentarQuandoProximo("HORARIO C");




}



//Função: função de callback
//        esta função é chamada toda vez que uma informação de
//        um dos tópicos subescritos chega)
//Parâmetros: nenhum
//Retorno: nenhum
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String hdr;
  for (int i = 0; i < 14; i++)
  {
    char c = (char)topic[i];
    hdr += c;//    Serial.print(c);
  }

  String msg;
  //obtem a string do payload recebido
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    msg += c;//    Serial.print(c);
  }

  //definir o que fazer com os valores recebidos
  //caso o hdr (header) substring != -1 , entao e esse o variavel que esta presente
  if (hdr.indexOf("AGORA") != -1) {
    alimentarAgora();
    MQTT.publish(TOPICO_PUBLISH, "DOSAR AGORA");
  }
  if (hdr.indexOf("HORAA") != -1) {
    time_A = msg.toInt();   //  Serial.print("HORAA "); Serial.println(time_A);
    EEPROM.write(1, time_A);
    EEPROM.commit();
  }
  if (hdr.indexOf("HORAB") != -1) {
    time_B = msg.toInt();   //  Serial.print("HORAB "); Serial.println(time_B);
    EEPROM.write(2, time_B);
    EEPROM.commit();
  }
  if (hdr.indexOf("HORAC") != -1) {
    time_C = msg.toInt();   //  Serial.print("HORAC "); Serial.println(time_C);
    EEPROM.write(3, time_C);
    EEPROM.commit();
  }
  if (hdr.indexOf("LIGAA") != -1) {
    time_A_flag = msg.toInt();   //  Serial.print("HORAA "); Serial.println(time_A);
    EEPROM.write(4, time_A_flag);
    EEPROM.commit();
  }
  if (hdr.indexOf("LIGAB") != -1) {
    time_B_flag = msg.toInt();   //  Serial.print("HORAB "); Serial.println(time_B);
    EEPROM.write(5, time_B_flag);
    EEPROM.commit();
  }
  if (hdr.indexOf("LIGAC") != -1) {
    time_C_flag = msg.toInt();   //  Serial.print("HORAC "); Serial.println(time_C);
    EEPROM.write(6, time_C_flag);
    EEPROM.commit();
  }

}

//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parâmetros: nenhum
//Retorno: nenhum
void reconnectMQTT() {
  while (!MQTT.connected())
  {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT))
    {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      //publica para estes pra synchonizar
      // puxar o que foi salvo
      String a = String( time_A);
      String b = String(time_B);
      String c = String(time_C);
      String flagA = String(time_A_flag);
      String flagB = String(time_B_flag);
      String flagC =  String(time_C_flag);

      Serial.println("valores salvos");
      Serial.print("A flag:");  Serial.print(time_A_flag);
      Serial.print(" time A:"); Serial.println(time_A);
      Serial.print("B flag:");  Serial.print(time_B_flag);
      Serial.print(" time B:"); Serial.println(time_B);
      Serial.print("C flag:");  Serial.print(time_C_flag);
      Serial.print(" time C:"); Serial.println(time_C);

      //atualizas horarios
      MQTT.publish(TOPICO_SUBSCRIBE_A, &a[0] );
      MQTT.publish(TOPICO_SUBSCRIBE_B, &b[0]);
      MQTT.publish(TOPICO_SUBSCRIBE_C, &c[0]);
      MQTT.publish(TOPICO_SUBSCRIBE_FLAGA, &flagA[0]);
      MQTT.publish(TOPICO_SUBSCRIBE_FLAGB, &flagB[0]);
      MQTT.publish(TOPICO_SUBSCRIBE_FLAGC, &flagC[0]);

      MQTT.subscribe(TOPICO_SUBSCRIBE);
      MQTT.subscribe(TOPICO_SUBSCRIBE_FLAGA);
      MQTT.subscribe(TOPICO_SUBSCRIBE_FLAGB);
      MQTT.subscribe(TOPICO_SUBSCRIBE_FLAGC);
      MQTT.subscribe(TOPICO_SUBSCRIBE_A);
      MQTT.subscribe(TOPICO_SUBSCRIBE_B);
      MQTT.subscribe(TOPICO_SUBSCRIBE_C);
    }
    else
    {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Haverá nova tentativa de conexao em 2s");
      delay(1000);
    }
  }
}

//Função: reconecta-se ao WiFi
//Parâmetros: nenhum
//Retorno: nenhum
void reconectWiFi() {
  //se já está conectado a rede WI-FI, nada é feito.
  //Caso contrário, são efetuadas tentativas de conexão
  if (WiFi.status() == WL_CONNECTED)    return;
  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("IP obtido: ");
  Serial.println(WiFi.localIP());
}

//Função: envia ao Broker o estado atual do output
//Parâmetros: nenhum
//Retorno: nenhum
void EnviaEstadoOutputMQTT(void) {
  if (updateCycle == 10) {
    //  Serial.print("A flag:");  Serial.print(time_A_flag);
    //  Serial.print(" time A:"); Serial.println(time_A);
    String s;
    if (time_A_flag == 2)       s = "HORARIO A DOSADO" ;
    else if (time_A_flag == 1)  s = "HORARIO A PROGRAMADO PARA " + String(time_A) + "H" ;
    else     s = "HORARIO A DES" ;
    MQTT.publish(TOPICO_PUBLISH, &s[0]);
    Serial.println(s);
    if (time_B_flag == 2)      s = "HORARIO B DOSADO" ;
    else if (time_B_flag == 1) s = "HORARIO B PROGRAMADO PARA " + String(time_B) + "H" ;
    else    s = "HORARIO B DES" ;
    MQTT.publish(TOPICO_PUBLISH, &s[0]);
    Serial.println(s);
    if (time_C_flag == 2)     s = "HORARIO C DOSADO" ;
    else if (time_C_flag == 1)s = "HORARIO C PROGRAMADO PARA " + String(time_C) + "H" ;
    else    s = "HORARIO C DES" ;
    MQTT.publish(TOPICO_PUBLISH, &s[0]);
    Serial.println(s);
  }
}

void SOLTAR_RACAO() {
  int pos;
  Serial.println("PA DESCENDO");            
  for (pos = 180; pos >= 70; pos -= 1) { // vai de 180 graus para 0 graus
    myservo.write(pos);              //  fala para o servo ir para a posição da varíavel 'pos'
    delay(5);                       // espera 15ms até o servo alcançar a posição
  }                                
  delay(2000);                      
  Serial.println("PA SUBINDO");             
  for (pos = 70; pos <= 180; pos += 1) { // vai de 0 graus para 180 graus
    // in steps of 1 degree             
    myservo.write(pos);              // fala para o servo para ir para a posição da variável 'pos'
    delay(5);                       // espera 15ms até o servo alcançar a posição
  }                                     
}

int lerDistancia() {
  long duration;
  int distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // COMEÇA ENVIO DE SINAL DE AUDIO DE 10 MICROSEGUNDOS.
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  // TERMINA SINAL DE 10 MICRO SEGUNDOS
  digitalWrite(trigPin, LOW);
  // LE A DURAÇAO DO PULSO ALTO
  duration = pulseIn(echoPin, HIGH);
  //PEGA A DURAÇAO DO PULSO, TRANSFORMA EM DISTANCIA
  distance = duration * 0.034 / 2;
  Serial.print("Distance: ");
  Serial.println(distance);
  return distance;
}

void alimentarAgora() {
  Serial.print("Alimentando");
  digitalWrite(VIBRA, LIGA);
  while (digitalRead(MICROSWITCH) == 1) {
    delay(10);
  }
  Serial.println("PA CHEIA");
  digitalWrite(VIBRA, DESLIGA);
  delay(1000);
  Serial.println("SOLATNDO RACAO");
  SOLTAR_RACAO();
  Serial.print("PRONTO");
}

byte alimentarQuandoProximo(String p ) {
  if (lerDistancia() < 16 && lerDistancia() < 16) {
    Serial.print("Alimentando");
    Serial.println(p);
    Serial.println("VIBRANDO-ENXENDO PA");
    digitalWrite(VIBRA, LIGA);
    while (digitalRead(MICROSWITCH) == 1) {
      delay(10);
    }
    Serial.println("PA CHEIA");
    digitalWrite(VIBRA, DESLIGA);
    delay(1000);
    Serial.println("SOLATNDO RACAO");
    SOLTAR_RACAO();
    Serial.print("PRONTO");
    Serial.println(p);
    return 2;//atualiza status do pedido de dosagem
  }
  else{
  return 1;
  }

}
