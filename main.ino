#include "motor.h"
#include "entradas.h"
#include "tiempos.h"
#include "WIFIconfig.h"
#include "MQTTconfig.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <Timer.h>
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient client(espClient);

// ESTACIONES:
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

// TIMERS:
TON *tPublishInfo;
TON *tSerial;
TON *tCheckConnection;

// CONSTANTES
char *CERRADO = "cerrado";
char *ABIERTO = "abierto";
char *INTERMEDIO = "intermedio";
char *ERROR_FINALES_CARRERA = "Error finales carrera";

char *ABRIR = "abrir";
char *CERRAR = "cerrar";
char *PARAR = "parar";

const boolean START = true;
const boolean RESET = false;

// VARIABLES
static bool recorridoGuardado = false;
char *estado = "";

void setup()
{
  Serial.begin(115200);

  WIFIConnection();

  setup_motor();

  setup_entradas();

  // Init limit switchs
  Serial.print("antireboteFC: ");
  Serial.println(antireboteFC());

  Serial.print("antireboteFA: ");
  Serial.println(antireboteFA());

  E0 = true;

  OTAConfig();

  MQTTConnection();

  Serial.println("Connected to the WiFi network");

  tPublishInfo = new TON(1000);
  tSerial = new TON(5000);
  tCheckConnection = new TON(52000);
}

char *getEstado()
{
  if (antireboteFC() && !antireboteFA())
  {
    return CERRADO;
  }
  if (antireboteFA() && !antireboteFC())
  {
    return ABIERTO;
  }
  if (!antireboteFA() && !antireboteFC())
  {
    return INTERMEDIO;
  }
  if (antireboteFC() && antireboteFA())
  {
    return ERROR_FINALES_CARRERA;
  }
}

void publishInfo()
{
  if (tPublishInfo->IN(START))
  {
    StaticJsonDocument<192> jsonDoc;
    JsonObject recorrido = jsonDoc.createNestedObject("recorrido");

    String payload = "";

    if ((estado == CERRADO || estado == ABIERTO || estado == ERROR_FINALES_CARRERA) && estado == getEstado())
    {
      tPublishInfo->IN(RESET);
      return;
    }

    estado = getEstado();

    jsonDoc["estado"] = estado;
    jsonDoc["consumo"] = getConsumo();
    jsonDoc["consumoLimite"] = getConsumoLimite();

    recorrido["recorridoGuardado"] = recorridoGuardado;
    recorrido["t_PosicionFinal"] = getPosicionFinal();

    serializeJson(jsonDoc, payload);
    client.publish(topicState, (char *)payload.c_str());

    tPublishInfo->IN(RESET);
  }
}

void checkMqttConnection()
{
  if (tCheckConnection->IN(START))
  {
    if (!client.connected())
    {
      ESP.restart();
    }

    tCheckConnection->IN(RESET);
  }
}

void loop()
{
  ArduinoOTA.handle();
  client.loop();
  yield();

  publishInfo();

  if (E0) // Posicion standby
  {
    if (tSerial->IN(START))
    {
      Serial.println("E0");

      tSerial->IN(RESET);
    }

    parar();
    checkMqttConnection();

    if (recibir())
    {
      E1 = true;
      E0 = false;
    }
  }

  if (E1)
  {
    if (tSerial->IN(START))
    {
      Serial.println("E1");

      tSerial->IN(RESET);
    }

    if (!antireboteFC() && !antireboteFA())
    {
      Serial.println("E1");
      E2 = true;
      E1 = false;
    }

    if (antireboteFC() && !antireboteFA())
    {
      E3 = true;
      C_Tiempos(1); // Tiempo inicial
      E1 = false;
    }

    if (!antireboteFC() && antireboteFA())
    {
      E5 = true;
      C_Tiempos(1); // Tiempo inicial
      E1 = false;
    }

    if (antireboteFC() && antireboteFA())
    {
      E0 = true;
    }

    T1(0, 0);                 // Reseteo el temporizador
    antiaplastamiento(false); // Reseteamos valores de antiaplastamiento
    // Serial.println("E1");
  }

  if (E2) // POSICION INTERMEDIA
  {
    if (tSerial->IN(START))
    {
      Serial.println("E2");

      tSerial->IN(RESET);
    }

    lenta();
    cerrar();

    if (antireboteFC() || antireboteFA() || recibir() || antiaplastamiento(true))
    {

      E0 = true;
      E2 = false;
      Serial.println("E2");
    }
  }

  if (E3) // ABRIR
  {
    if (tSerial->IN(START))
    {
      Serial.println("E3");
      //
      //      Serial.print("Wifi: ");
      //      Serial.println(WiFi.waitForConnectResult() == WL_CONNECTED);

      tSerial->IN(RESET);
    }

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
      //      Serial.print("Wifi: ");
      //      Serial.println(WiFi.waitForConnectResult() == WL_CONNECTED);

      E4 = true;
      E3 = false;
    }
    if ((antireboteFA() && antireboteFC()) || recibir() || antiaplastamiento(true))
    {
      E0 = true;
      E3 = false;
      // Serial.println("E3");
    }
    C_Tiempos(2);
  }

  if (E5) // CERRAR
  {
    if (tSerial->IN(START))
    {
      Serial.println("E5");

      tSerial->IN(RESET);
    }

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
      // Serial.println("E5");
    }
    C_Tiempos(2);
  }

  if (E4) // Posicion standby
  {
    if (tSerial->IN(START))
    {
      Serial.println("E4");
      //      Serial.print("Recorrido guardado: ");
      //      Serial.println(recorridoGuardado);
      //
      //      Serial.print("Wifi: ");
      //      Serial.println(WiFi.waitForConnectResult() == WL_CONNECTED);
      //
      //      Serial.println("IP address: ");
      //      Serial.println(WiFi.localIP());

      tSerial->IN(RESET);
    }

    if (!recorridoGuardado)
    {
      C_Tiempos(3); // Hago el calculo de los tiempos Rapida y Seguridad
      recorridoGuardado = true;
    }

    parar();
    checkMqttConnection();

    T1(0, 0);                 // Reseteo T1
    antiaplastamiento(false); // Reseteamos valores de antiaplastamiento
    if (recibir())
    {
      E6 = true;
      E4 = false;
      // Serial.println("E4");
    }
  }

  if (E6)
  {
    if (tSerial->IN(START))
    {
      Serial.println("E6");

      tSerial->IN(RESET);
    }

    static bool e_abrir = false;
    static bool e_cerrar = false;

    if (!antireboteFC() && !antireboteFA()) // INTERMEDIO
    {

      if (e_abrir) // Estaba abriendo asi que cerramos
      {
        C_Tiempos(1);
        E7_c = true;
        e_cerrar = true;
        e_abrir = false;
        E6 = false;
      }

      else if (e_cerrar) // Estaba cerrando asi que abrimos
      {
        C_Tiempos(1);
        E7_a = true;
        e_abrir = true;
        e_cerrar = false;
        E6 = false;
      }
    }

    if (antireboteFC() && !antireboteFA()) // ABRIR
    {
      e_cerrar = false;
      e_abrir = true;
      E8 = true;
      C_Tiempos(1); // Tiempo inicial
      E6 = false;
      // Serial.println("E6 ABRIR");
    }

    if (!antireboteFC() && antireboteFA()) // CERRAR
    {
      e_abrir = false;
      e_cerrar = true;
      E9 = true;
      C_Tiempos(1); // Tiempo inicial
      E6 = false;
      // Serial.println("E6 CERRAR");
    }
  }

  if (E7_a) // INTERMEDIO ABRIR
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

  if (E7_c) // INTERMEDIO CERRAR
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

  if (E8) // ABRIR
  {
    if (tSerial->IN(START))
    {
      Serial.println("E8");

      tSerial->IN(RESET);
    }

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

  if (E9) // CERRAR
  {
    if (tSerial->IN(START))
    {
      Serial.println("E9");

      tSerial->IN(RESET);
    }

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

void WIFIConnection()
{
  // connecting to a WiFi network
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi..");
    delay(5000);
    ESP.restart();
  }

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

void OTAConfig()
{
  ArduinoOTA.setHostname(client_name);
  ArduinoOTA.onStart([]()
                     {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    } });
  ArduinoOTA.begin();
}

void MQTTConnection()
{
  // connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  while (!client.connected())
  {
    String client_id = client_name;
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("Public emqx mqtt broker connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  client.subscribe(topicCommand);
}

void callback(char *topicCommand, byte *payload, unsigned int length)
{
  //  Serial.print("Message arrived in topic: ");
  //  Serial.println(topicCommand);
  //  Serial.print("Message:");
  String payload_n;

  for (int i = 0; i < length; i++)
  {
    payload_n += (char)payload[i];
  }

  //  Serial.println(payload_n);
  //  Serial.println("-----------------------");

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, payload_n);
  if (error)
    return;
  String orden = doc["orden"];
  int consumoLimite = doc["consumoLimite"];
  boolean recorridoGuardadoRecibido = doc["recorrido"]["recorridoGuardado"];

  if (orden == ABRIR || orden == CERRAR || orden == PARAR)
  {
    setAccionar(true);
  }

  if (consumoLimite != 0)
  {
    setConsumoLimite(consumoLimite);
  }

  recorridoGuardado = recorridoGuardadoRecibido;
  setPosicionFinal(doc["recorrido"]["t_PosicionFinal"]);

  if (recorridoGuardado && getPosicionFinal() > 0)
  {
    calcularTiempos();
    E4 = true;
    E0 = false;
  }
}
