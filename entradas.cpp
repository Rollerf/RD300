#include <Arduino.h>
#include <Switches.h>
#include <PubSubClient.h>
#include "MQTTconfig.h"

//TODO: Se quita todo lo que tenga que ver con centralita
//Switches* switchCentralita;

//ENTRADAS DIGITALES:
#define FC 3//Portal cerrado
#define FA 4//Portal abierto

//TODO: Esta entrada se quita y es sustituida por la orden wifi
//#define centralita 5//Centralita rolling code

//ENTRADA ANALOGICA:
#define consumo A0//Lectura analógica

//Variable orden recibida
static bool accionar = false;

void callback(char *topicCommand, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topicCommand);
  Serial.print("Message:");
//  for (int i = 0; i < length; i++) {
//    Serial.print((char) payload[i]);
//  }

  String myString = String((char*)payload);
  Serial.println(myString);
  Serial.println("-----------------------");
  accionar = true;
}

void MQTTConnection(PubSubClient client) {
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  while (!client.connected()) {
//    String client_id = "esp8266-client-";
//    client_id += String(WiFi.macAddress());
    String client_id = "portal-trasero";
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  // publish and subscribe
  client.subscribe(topicCommand);
}

void setup_entradas(PubSubClient client)
{
  pinMode(FC, INPUT_PULLUP);
  pinMode(FA, INPUT_PULLUP);

  MQTTConnection(client);

  
  //TODO: Se quita todo lo que tenga que ver con centralita
//  pinMode(centralita, INPUT_PULLUP);

//  switchCentralita = new Switches(60, centralita);
}

//TODO: Se quita todo lo que tenga que ver con centralita
//Puedo hacer que use una variable y devuelva y resete la variable
//bool recibir()
//{
//  static unsigned long tInicial = 0;
//  if (millis() >= tInicial + 1500)
//  {
//    if (switchCentralita->buttonMode(true)) {
//      tInicial = millis();
//      return true;
//
//    }
//  }
//  return false;
//}

bool recibir()
{
  if(accionar){
      accionar = false;
      return !accionar;
  }

  return accionar;
}

byte antireboteFC()
{
  byte PM = digitalRead(FC);
  static byte estadoFC = 0; //En esta variable guardo el estado actual del pulsador
  static unsigned long tInicial_FC = millis();
  const unsigned long antiRebote = 60; //Tiempo antirebote del pulsador. AJUSTAR SEGUN NECESIDAD.
  if (PM != estadoFC)
  {
    PM = digitalRead(FC);
    if (millis() >= tInicial_FC + antiRebote)
    {
      estadoFC = PM;
    }
  }
  else
  {
    tInicial_FC = millis();
  }
  return estadoFC;
}

byte antireboteFA()
{
  byte PM = digitalRead(FA);
  static byte estadoFA = 0; //En esta variable guardo el estado actual del pulsador
  static unsigned long tInicial_FA = millis();
  const unsigned long antiRebote = 60; //Tiempo antirebote del pulsador. AJUSTAR SEGUN NECESIDAD.
  if (PM != estadoFA)
  {
    PM = digitalRead(FA);
    if (millis() >= tInicial_FA + antiRebote)
    {
      estadoFA = PM;
    }
  }
  else
  {
    tInicial_FA = millis();
  }

  return estadoFA;
}

bool antiaplastamiento(bool arranque)
{
  static unsigned long tInicialAplastamiento = millis();
  int fuerza = analogRead(consumo);
  const int fuerza_limite = 150; //Anterior valor 200
  static int fuerza_max;
  static bool listo = false;
  //Serial.println(fuerza);
  if (arranque && !listo)
  {
    if (tInicialAplastamiento + 2000 <= millis())
    {
      listo = true;
    }
  }
  else if (!arranque) //Se resetean las variables
  {
    listo = false;
    tInicialAplastamiento = millis();
    fuerza_max = 0;
  }
  if (listo)
  {
    if (fuerza > fuerza_max) //Actualizamos el valor de fuerza_max al más alto
    {
      fuerza_max = fuerza;
    }

    if (fuerza_max > fuerza_limite) //Si la fuerza maxima supera el limite
    {
      //Serial.println(fuerza_max);
      return true;//CAMBIAR POR TRUE
    }
    else
    {
      return false;
      //Serial.println("false");
    }
  }
  else
  {
    return false;
  }
}
