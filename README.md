
# Alimentador de Animais Domésticos


# Descrição
Este projeto foi desenvolvido com o objetivo de auxiliar os cidadãos, que cada vez mais, por conta de necessidades da sociedade atual, precisam ficar fora de suas casas durante longas horas, logo, sempre havera uma preocupação para aqueles donos de pets, na questão alimentar. 
O Dispositivo Alimentador de Animais domésticos veio com o objetivo de minimizar tal preocupação, já que dispõe da função de alimentar o seu animal de estimação através de um "clique", ou até mesmo, apenas decidindo um certo horário para seu pet, que ao chegar perto de sua vasilha de ração, será detectado pelo sensor ultrassônico que se encontra nesse projeto, e através dos dispositivos usados, uma porção de ração irá ser despejada em sua vasilha de comida.

## Projeto Final
![enter image description here](https://i.imgur.com/8e0oO4m.jpg)
## Hardware Utilizado
 - NodeMcu ESP8266
 - Servo Motor SG5010
 - Sensor Ultrassônico HC-SR04
 - Buzzer Contínuo
 - MicroSwitch
 - Motor Vibratório
 - Protoboard
 - Jumpers

## Interface MQTT
![Interface MQTT](https://i.imgur.com/hBCYZUy.jpg)

Interface gerada pelo aplicativo  IoTMQTT-Panel
## Representação do Projeto
![Representação de projeto físico](https://i.imgur.com/WBsSLUD.jpg)

Representação do projeto feito através da plataforma Fritzing.

## Vídeo explicativo do projeto
Vídeo explicativo do Hardware e do Software utilizados em projeto:
[![enter image description here](https://media.discordapp.net/attachments/705567563324850177/846561307368226836/unknown.png?width=682&height=427)](https://www.youtube.com/watch?v=GjlpPZVdYmk)
## Descrição do Código do Projeto

Para o desenvolvimento do código foram usadas as bibliotecas EEPROM.h, NTPClient.h, WiFiUdp.h, ESP8266WiFi.h e PubSubClient.h.
- EEPROM.h: permite ler e escrever dados na memória do Arduino.
- NTPClient.h: libera o uso do protocolo NTP, permitindo a automação e a contagem correta do tempo.
- WiFiUdp.h: biblioteca que permite o uso do protocolo UDP.
- PubSubClient.h: a partir dela conseguimos fazer a conexão com o mqtt.
- ESP8266WiFi.h: a partir dela conseguimos criar a conexão do WIFI com o NodeMCU.

A IDE usada para fazer essa programação foi a Arduino IDE.

No começo do código a partir das definições da variáveis e da importação das bibliotecas, o resto foi só definir as principais configurações do mqtt e nodeMCU, além de configurar o funcionamento do sensor ultrassônico, o movimento do servo para despejar a comida e as configurações de timer e som a partir do buzzer.

[Clique aqui para ver o código](https://github.com/VDFerreira/Alimentador-de-Animais-OIC/blob/main/codigoAlimentador.ino).

# Feito por

 - Igor de Holanda Chagas
 - Vitor Delsin Ferreira
