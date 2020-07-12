#include <Arduino.h>

//SALIDAS:
#define MC 8//Motor cerrar
#define MA 9//Motor abrir
#define MV 10//Motor velocidad

void setup_motor()
{
  for (byte i = 8; i <= 10; i++)
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
