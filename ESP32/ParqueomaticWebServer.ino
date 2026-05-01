/*************************************************************************************************
  ESP32 Web Server - EJEMPLO
  Ejemplo de creación de Web server 
  Basándose en los ejemplos de: 
  https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/
  https://electropeak.com/learn
**************************************************************************************************/
//************************************************************************************************
// Librerías
//************************************************************************************************
#include <WiFi.h>
#include <WebServer.h>
//************************************************************************************************
// Variables globales
//************************************************************************************************
// SSID & Password
const char* ssid = "MarioGalaxyA22";  // Enter your SSID here
const char* password = "contraseña";  //Enter your Password here


WebServer server(80);  // Object of WebServer(HTTP port, 80 is defult)

// Variables de ESP32
uint8_t LED1pin = 2;
bool LED1status = LOW;

// Arreglo para parqueo
uint8_t availability[8] = {0,0,0,0,0,0,0,0};
uint8_t last_update_times[8] = {0,0,0,0,0,0,0,0};

// Definiciones internas
#define LEAVE_PARKING_SPOT  1  // Un parqueo libre es 1
#define ENTER_PARKING_SPOT  0  // Un parqueo ocupado es 0
#define PARKING_SPOT_FREE       1
#define PARKING_SPOT_OCCUPIED   0


//************************************************************************************************
// Configuración
//************************************************************************************************
void setup() {
  Serial.begin(115200);
  Serial.println("Intentando conectarse ");
  Serial.println(ssid);

  pinMode(LED1pin, OUTPUT);

  // Iniciar intento de conexión a modem wi-fi
  WiFi.begin(ssid, password);

  // Esperar hasta que haya una conexión
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // Imprimir dirección IP
  Serial.println("");
  Serial.println("Conexión WiFi Exitosa");
  Serial.print("Se obtuvo dirección IP: ");
  Serial.println(WiFi.localIP());  //Show ESP32 IP on serial

  // Inicializar handlers
  server.on("/", handle_OnConnect); // Directamente desde e.g. 192.168.0.8
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  
  // Handler en caso de que no se encuentre (¿Qué cosa?)
  server.onNotFound(handle_NotFound);

  // Iniciar servidor
  server.begin();
  Serial.println("Servidor HTTP Iniciado");
  delay(100);
}

//************************************************************************************************
// Loop Principal
//************************************************************************************************
void loop() {
  server.handleClient();
  if (LED1status)
  {
    digitalWrite(LED1pin, HIGH);
  }
  else
  {
    digitalWrite(LED1pin, LOW);
  }
}

/************************************************************************************************
 HANDLERS
 Estas funciones se ejecutan cuando uno de los botones se presiona en
 el sitio web. También hay un Handler por si la página no se encuentra.
 Por cada Handler se realizan las siguientes acciones: (1) se modifica una variable relacionada 
 al hardware, (2) se envía un mensaje a través del serial, (3) se actualiza el servidor.
*************************************************************************************************/

//************************************************************************************************
// Handler de Inicio página
//************************************************************************************************
// Handler que se ejecuta cuando un cliente se conecta
void handle_OnConnect() {
  // Apagar LED1 - Enviar al server la configuración inicial
  LED1status = LOW;
  Serial.println("GPIO2 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status));
}
//************************************************************************************************
// Handler de led1on
//************************************************************************************************
// Handler que se ejecuta cuando se presiona el botón para encender LED1
void handle_led1on() {
  
  LED1status = HIGH;
  Serial.println("GPIO2 Status: ON");
  server.send(200, "text/html", SendHTML(LED1status));
}
//************************************************************************************************
// Handler de led1off
//************************************************************************************************
void handle_led1off() {
  LED1status = LOW;
  Serial.println("GPIO2 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status));
}

/************************************************************************************************
-- Procesador de HTML -- 
Este genera una cadena de texto con un archivo HTML que genera la página del servidor.
Para accionar un Handler se debe colocar un widget con un hipervímculo (href), con el 
primer argumento que se utiliza en la función server.on("/accion",handler) para establecer el handler.

Ejemplo:
En la siguiente línea de código se define un botón con el texto "ON"
con un hipervínculo a la misma página con la terminación "\led1on.
Al presionarlo se activa el handler handle_led1on. El widget a la izquierda es un texto
que dice Estado LED OFF.

ptr += "<p>Estado LED: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>\n";

Este código sirve para indicar que el LED está apagado y poner un botón para encenderlo.
*************************************************************************************************/
String SendHTML(uint8_t led1stat) {
  String ptr += "<!DOCTYPE html>\n";
  ptr += "<html lang=es>\n";
  ptr += "<head>\n";
  ptr += "<meta charset=UTF-8>\n";
  ptr += "<meta name=viewport content=\"width=device-width, initial-scale=1.0\">\n";
  ptr += "<title>Parqueomatic</title>\n";
  ptr += "<link href=https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css rel=stylesheet>\n";
  ptr += "</head>\n";
  ptr += "<body class=bg-light>\n";
  ptr += "<div class=\"container mt-5\">\n";
  ptr += "<div class=\"row mb-4\">\n";
  ptr += "<div class=\"col text-center\">\n";
  ptr += "<h1 class=\"display-4 text-primary fw-bold\">Parqueomatic</h1>\n";
  ptr += "<p class=\"lead text-muted\">Panel de control de estacionamiento en tiempo real</p>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "<div class=\"card shadow-sm\">\n";
  ptr += "<div class=\"card-body p-0\">\n";
  ptr += "<div class=table-responsive>\n";
  ptr += "<table class=\"table table-striped table-hover mb-0 text-center align-middle\">\n";
  ptr += "<thead class=table-dark>\n";
  ptr += "<tr>\n";
  ptr += "<th scope=col>Número de Parqueo</th>\n";
  ptr += "<th scope=col>Estado</th>\n";
  ptr += "<th scope=col>Última actualización</th>\n";
  ptr += "</tr>\n";
  ptr += "</thead>\n";
  ptr += "<tbody id=tabla-parqueos>\n";

  for(i = 0; i < 8; i++)
  {
    String PrintParkingSpotRow(uint8_t availability[8], i)
  }

  ptr += "</tbody>\n";
  ptr += "</table>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "<script src=https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js></script>\n";
  ptr += "</body>\n";
  ptr += "</html>";
  return ptr;
}

//************************************************************************************************
// Handler de not found
//************************************************************************************************
void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

//************************************************************************************************
// Funciones Auxiliares
//************************************************************************************************

// Actualizar el vector de disponibilidad
void UpdateParkingSpot(uint8_t *availability[8], uint8_t spot_id, uint8_t event){
  switch(event)
  {
    case LEAVE_PARKING_SPOT:
    *availability[spot_id] = PARKING_SPOT_FREE;
    break;

    case ENTER_PARKING_SPOT:
    *availability[spot_id] = PARKING_SPOT_OCCUPIED;
    break;
  
    default:
    break;
  }
}

// Imprimir texto HTML para fila
String PrintParkingSpotRow(uint8_t availability[8], uint8_t spot_id){
  // ID de parqueo
  String ptr  = "<tr id=parqueo-"+ to_string(spot_id+1) +">\n";
  ptr += "<td class=fw-bold>" + to_string(spot_id+1) "</td>\n";

  // Disponibilidad
  if(availability[spot_id] == PARKING_SPOT_FREE)
  {
    ptr += "<td><span class=\"badge bg-success\">Libre</span></td>\n";
  }
  else if(availability[spot_id] == PARKING_SPOT_OCCUPIED)
  {
    ptr += "<td><span class=\"badge bg-danger\">Ocupado</span></td>\n";
  }
  
  // Guardar el tiempo en milisegundos
  ptr += "<td>" + to_string(millis())+"</td>\n";
  ptr += "</tr>\n";
  return ptr
}