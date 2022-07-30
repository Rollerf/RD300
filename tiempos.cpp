#include <Arduino.h>

//CONFIGURACION VELOCIDAD
static unsigned long Pf = 0;

static unsigned long Prapida_f = 0;
static unsigned long Pseguridad_f = 0;

static unsigned long Prapida_o = 0;
static unsigned long Pseguridad_o = 0;

const unsigned long Po = 10000;
//PROBLEMA: Al ser Po = 0 no puedo poner el tiempo de seguridad por que seria un numero negativo
// y par atrabajar con millis() necesito unsigned long
//SOLUCION: Offset de 1000 Po = 10000; //No influye por que todos los tiempos calculados incluyen este ofset, que se descuenta cuando se hacen operaciones entre ellos.

void setPosicionFinal(unsigned long posicionFinalGuardada) {
  Pf = posicionFinalGuardada;
}

unsigned long getPosicionFinal() {
  return Pf;
}

//TEMPORIZADOR 1:
bool T1(bool estado, long consignaTiempo) //Para activar
{
  static unsigned long tiempoInicial;
  bool salida;

  if  ( estado == 1 ) //Si la entrada está activada
  {

    if ( millis() - tiempoInicial >= consignaTiempo)
    {
      salida = 0;
      return salida;
    }

    salida = 1;

    return salida;

  }
  else
  {
    salida = 0;
    tiempoInicial = millis();

    return salida;

  }
}

void calcularTiempos() {
  Prapida_f = Pf - 1000;
  Pseguridad_f = Pf + 5000;
  ////Serial.println("Prapida_f:");
  //Serial.println(Prapida_f);

  //Serial.println("Pseguridad:_f");
  //Serial.println(Pseguridad_f);

  //POSICION INICIAL CERRADO
  Prapida_o = Po + 1000;
  Pseguridad_o = Po - 5000;
}

//CALCULO DE T_VRAPIDA Y T_SEGURIDAD:
// Esta funcion trabaja según la fase en la que este el portal. En cada
// sitio del programa donde cuadre se llama a esta función con una fase
byte C_Tiempos(byte fase) //Para activar
{
  //VARIABLES
  static unsigned long tiempoInicio;
  static unsigned long Pa;
  static unsigned long Pa_a;
  static unsigned long Pa_c;

  unsigned long Ta;


  switch (fase)
  {
    case 1: // Fase 1 cuando arranca desde abierto o cerrado estacion 3 y 5 o 8 y 9
      tiempoInicio = millis();
      break;

    case 2: // Fase 2 mientras esta cerrando o abriendo y aprendiendo estacion 3 y 5
      Pa = millis() - tiempoInicio;
      break;

    case 3: // Fase 3 cuando llega a un final de carrera despues de abrir
      // o cerrar. Es decir cuando llega a la estación 4

      //POSICION FINAL ABIERTO
      Pf = Pa + Po;

      Serial.println(Pf);

      calcularTiempos();

      break;

    case 4: //Fase 4 mientras esta cerrando
      Pa_a = 0;//Se resetea Pa_a por que parte de un final
      Pa_c = 0;//Se resetea Pa_c por que parte de un final
      Ta = millis() - tiempoInicio;
      Pa = Pf - Ta; //Parte de posicion final hacia posicion inicial

      if (Pa <= Prapida_o)
      {
        if (Pa <= Pseguridad_o)
        {
          return 2; //Se activa y para de funcionar
        }
        return 1; //Le digo de alguna forma que ponga la lenta
      }

      break;

    case 5: //Fase 5 mientras esta abriendo
      Pa_a = 0;//Se resetea Pa_a por que parte de un final
      Pa_c = 0;//Se resetea Pa_c por que parte de un final
      Ta = millis() - tiempoInicio;
      Pa = Po + Ta; //Parte de la posicion inicial hacia la final

      if (Pa >= Prapida_f)
      {
        if (Pa >= Pseguridad_f)
        {
          return 2; //Se activa y para de funcionar
        }
        return 1; //Le digo de alguna forma que ponga la lenta
      }

      break;

    case 6: //Fase 6 cuando esta en intermedia cerrando

      if (Pa_a > 0) //Si ya fue parado en el medio
      {
        Pa = Pa_a;
        Pa_a = 0;
        //Serial.println(Pa);
      }

      Ta = millis() - tiempoInicio;
      Pa_c = Pa - Ta;
      ////Serial.println(Pa_c);

      if (Pa_c <= Prapida_o)
      {
        if (Pa_c <= Pseguridad_o)
        {
          return 2; //Se activa y para de funcionar
        }
        return 1; //Le digo de alguna forma que ponga la lenta
      }

      break;

    case 7: //Fase 7 cuando esta en intermedia abriendo

      if (Pa_c > 0) //Si ya fue parado en el medio
      {
        Pa = Pa_c;
        Pa_c = 0;
        //Serial.println(Pa);
      }

      Ta = millis() - tiempoInicio;
      Pa_a = Pa + Ta;
      //Serial.println(Pa_a);

      if (Pa_a >= Prapida_f)
      {
        if (Pa_a >= Pseguridad_f)
        {
          return 2; //Se activa y para de funcionar
        }
        return 1; //Le digo de alguna forma que ponga la lenta
      }

      break;

  }
  return 0;
}
