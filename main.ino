#include "motor.h"
#include "entradas.h"
#include "tiempos.h"

//ESTACIONES:
bool E0 = false;
bool E1 = false;
bool E2 = false;
bool E3 = false;
bool E4 = false;
bool E5 = false;
bool E6 = false;
bool E7_a = false;
bool E7_c = false;
bool E8 = false;
bool E9 = false;
bool E10 = false;

void setup() {

  setup_motor();

  setup_entradas();

  //Serial.begin(9600);

  E0 = true;

}

void loop()
{
  if (E0)
  {
    parar();

    if (recibir())
    {
      //Serial.println("E0");
      //mando = true;
      E1 = true;
      E0 = false;
    }
  }

  if (E1)
  {
    if (!antireboteFC() && !antireboteFA())
    {
      E2 = true;
      E1 = false;
    }

    if (antireboteFC() && !antireboteFA())
    {
      E3 = true;
      C_Tiempos(1);//Tiempo inicial
      E1 = false;
    }

    if (!antireboteFC() && antireboteFA())
    {
      E5 = true;
      C_Tiempos(1);//Tiempo inicial
      E1 = false;
    }
    T1(0, 0); //Reseteo el temporizador
    antiaplastamiento(false);//Reseteamos valores de antiaplastamiento
    //Serial.println("E1");
  }

  if (E2) //POSICION INTERMEDIA
  {
    lenta();
    cerrar();

    if (antireboteFC() || antireboteFA() || recibir() || antiaplastamiento(true))
    {

      E0 = true;
      E2 = false;
      //Serial.println("E2");
    }

  }

  if (E3) //ABRIR
  {
    if (T1(1, 1000))
    {
      lenta();
      abrir();
    }
    else
    {
      rapida();
    }
    if (!antireboteFC() && antireboteFA())
    {
      E4 = true;
      E3 = false;
    }
    if ((antireboteFA() && antireboteFC()) || recibir() || antiaplastamiento(true))
    {
      E0 = true;
      E3 = false;
      //Serial.println("E3");
    }
    C_Tiempos(2);
  }

  if (E5) //CERRAR
  {
    if (T1(1, 1000))
    {
      lenta();
      cerrar();
    }
    else
    {
      rapida();
    }
    if (antireboteFC() && !antireboteFA())
    {
      E4 = true;
      E5 = false;
    }
    if ((antireboteFA() && antireboteFC()) || recibir() || antiaplastamiento(true))
    {
      E0 = true;
      E5 = false;
      //Serial.println("E5");
    }
    C_Tiempos(2);
  }
  if (E4)
  {

    static bool primeraVez = false;
    if (!primeraVez)
    {
      C_Tiempos(3);// Hago el calculo de los tiempos Rapida y Seguridad
      primeraVez = true;
      //Serial.println("primeraVez");
    }
    parar();
    T1(0, 0); //Reseteo T1
    antiaplastamiento(false);//Reseteamos valores de antiaplastamiento
    if (recibir())
    {
      E6 = true;
      E4 = false;
      //Serial.println("E4");
    }
  }

  if (E6)
  {
    static bool e_abrir = false;
    static bool e_cerrar = false;

    if (!antireboteFC() && !antireboteFA()) //INTERMEDIO
    {

      if (e_abrir) //Estaba abriendo asi que cerramos
      {
        C_Tiempos(1);
        E7_c = true;
        e_cerrar = true;
        e_abrir = false;
        E6 = false;
      }

      else if (e_cerrar) //Estaba cerrando asi que abrimos
      {
        C_Tiempos(1);
        E7_a = true;
        e_abrir = true;
        e_cerrar = false;
        E6 = false;
      }
    }

    if (antireboteFC() && !antireboteFA()) //ABRIR
    {
      e_cerrar = false;
      e_abrir = true;
      E8 = true;
      C_Tiempos(1);//Tiempo inicial
      E6 = false;
      //Serial.println("E6 ABRIR");
    }

    if (!antireboteFC() && antireboteFA()) //CERRAR
    {
      e_abrir = false;
      e_cerrar = true;
      E9 = true;
      C_Tiempos(1);//Tiempo inicial
      E6 = false;
      //Serial.println("E6 CERRAR");
    }
  }

  if (E7_a) //INTERMEDIO ABRIR
  {
    if (T1(1, 1000))
    {
      lenta();
      abrir();
    }
    else
    {
      if (C_Tiempos(7) == 1)
      {
        lenta();
      }
      else
      {
        rapida();
      }
    }

    if (antireboteFA() && !antireboteFC())
    {
      E4 = true;
      E7_a = false;
    }
    if ((antireboteFA() && antireboteFC()) || recibir() || antiaplastamiento(true) || C_Tiempos(7) == 2)
    {
      E4 = true;
      E7_a = false;
    }
  }

  if (E7_c) //INTERMEDIO CERRAR
  {
    if (T1(1, 1000))
    {
      lenta();
      cerrar();
    }
    else
    {
      if (C_Tiempos(6) == 1)
      {
        lenta();
      }
      else
      {
        rapida();
      }
    }
    if (!antireboteFA() && antireboteFC())
    {
      E4 = true;
      E7_c = false;
    }
    if ((antireboteFA() && antireboteFC()) || recibir() || antiaplastamiento(true) || C_Tiempos(6) == 2)
    {
      E4 = true;
      E7_c = false;
    }
  }

  if (E8) //ABRIR
  {
    if (T1(1, 1000))
    {
      lenta();
      abrir();
    }
    else
    {
      if (C_Tiempos(5) == 1)
      {
        lenta();
      }
      else
      {
        rapida();
      }
    }

    if (antireboteFA() && !antireboteFC())
    {
      E4 = true;
      E8 = false;
    }
    if ((antireboteFA() && antireboteFC()) || recibir() || antiaplastamiento(true) || C_Tiempos(5) == 2)
    {
      E4 = true;
      E8 = false;
    }
  }

  if (E9) //CERRAR
  {
    if (T1(1, 1000))
    {
      lenta();
      cerrar();
    }
    else
    {
      if (C_Tiempos(4) == 1)
      {
        lenta();
      }
      else
      {
        rapida();
      }
    }
    if (!antireboteFA() && antireboteFC())
    {
      E4 = true;
      E9 = false;
    }
    if ((antireboteFA() && antireboteFC()) || recibir() || antiaplastamiento(true) || C_Tiempos(4) == 2)
    {
      E4 = true;
      E9 = false;
    }

  }
}
