#include <Arduino.h>

//SALIDAS:
#define MC 14//Motor cerrar
#define MA 12//Motor abrir
#define MV 13//Motor velocidad

void setup_motor()
{
  for (byte i = 12; i <= 14; i++)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH);
  }
}
void parar()
{
  digitalWrite(MV, HIGH);
  digitalWrite(MC, HIGH);
  digitalWrite(MA, HIGH);
}
void abrir()
{
  digitalWrite(MC, HIGH);
  digitalWrite(MA, LOW);
}

void cerrar()
{
  digitalWrite(MA, HIGH);
  digitalWrite(MC, LOW);
}

void rapida()
{
  digitalWrite(MV, LOW);
}

void lenta()
{
  digitalWrite(MV, HIGH);
}
