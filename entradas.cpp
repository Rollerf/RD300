#include <Arduino.h>
#include <Timer.h>
#include "InputsConfig.h"

//ENTRADA ANALOGICA:
#define entrada_consumo A0//Lectura analógica

//VARIABLES ESTADO
static bool accionar = false;
static int consumo = 0;
static int consumoLimite = 100;

//Constantes
const unsigned long TIEMPO_ANTIREBOTE = 7;

const byte numeroMuestras = 35;

const boolean START = true;
const boolean RESET = false;

//TIMERS
TON* tAnalogReadInterval;

void setAccionar(bool nuevaAccion) {
  accionar = nuevaAccion;
}

void setConsumo(int nuevoConsumo) {
  consumo = nuevoConsumo;
}

void setConsumoLimite(int nuevoConsumoLimite) {
  consumoLimite = nuevoConsumoLimite;
}

int getConsumo() {
  return consumo;
}

int getConsumoLimite() {
  return consumoLimite;
}

void setup_entradas()
{
  pinMode(FC, INPUT_PULLUP);
  pinMode(FA, INPUT_PULLUP);

  tAnalogReadInterval = new TON(7);
}

bool recibir()
{
  if (accionar) {
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

  if (PM != estadoFC)
  {
    PM = digitalRead(FC);
    if (millis() >= tInicial_FC + TIEMPO_ANTIREBOTE)
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

  if (PM != estadoFA)
  {
    PM = digitalRead(FA);
    if (millis() >= tInicial_FA + TIEMPO_ANTIREBOTE)
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
  static bool listo = false;
  static int fuerzaSum = 0;
  static byte contadorMedia = 0;
  static int consumoActual = 0;

  if (tAnalogReadInterval->IN(START)) {
    fuerzaSum += analogRead(entrada_consumo);

    contadorMedia++;

    tAnalogReadInterval->IN(RESET);
  }

  if (contadorMedia >= numeroMuestras) {
    consumoActual = fuerzaSum / contadorMedia;
    setConsumo(consumoActual);
    fuerzaSum = 0;
    contadorMedia = 0;
  }

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
    consumoActual = 0;
  }
  if (listo)
  {
    if (consumoActual > consumoLimite) {
      return true;
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