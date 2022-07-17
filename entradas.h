#include <PubSubClient.h>
void setup_entradas(PubSubClient client);
bool recibir();
byte antireboteFC();
byte antireboteFA();
bool antiaplastamiento(bool arranque);
void MQTTConnection();
