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
#include <Wire.h> // Librería para I2C


//************************************************************************************************
// Variables globales
//************************************************************************************************
// SSID & Password
const char* ssid = "MarioGalaxyA22";  // Enter your SSID here
const char* password = "contraseña";  //Enter your Password here

//const char* ssid = "Galaxy S21 FE 5G 4d0c";  // Enter your SSID here
//const char* password = "shxl0927";  //Enter your Password here

// Pines de salida
#define NUCLEO_1_ADDR 0x10  // Dirección de la primera Nucleo (Parqueos 1 al 4)
#define NUCLEO_2_ADDR 0x11  // Dirección de la segunda Nucleo (Parqueos 5 al 8)

// Pines de salida para feedback local en la ESP32 (8 pines en total ahora)
const uint8_t SENSOR_PINS[8] = {32, 33, 25, 26, 27, 14, 12, 13}; 
const uint8_t HEARTBEAT_LED = 4; 
bool heartbeatStatus = LOW;

WebServer server(80);  // Object of WebServer(HTTP port, 80 is defult)

// Variables de ESP32
uint8_t LED1pin = 2;
bool LED1status = LOW;

// Arreglo para parqueo (8 espacios)
uint8_t availability[8] = {0,0,0,0,0,0,0,0};

// Definiciones internas
#define PARKING_SPOT_FREE       1
#define PARKING_SPOT_OCCUPIED   0
#define LEAVE_PARKING_SPOT  1  // Un parqueo libre es 1
#define ENTER_PARKING_SPOT  0  // Un parqueo ocupado es 0

// Variables de estado de datos de las Nucleo
uint8_t i2c_data_n1 = 0; 
uint8_t i2c_data_n2 = 0;

//************************************************************************************************
// Configuración
//************************************************************************************************
void setup() {
  Serial.begin(115200);
  Serial.println("Intentando conectarse ");
  Serial.println(ssid);

  // Iniciar I2C (SDA = 21, SCL = 22)
  Wire.begin(); 
  
  // Configurar los 8 pines de salida para los sensores
  pinMode(LED1pin, OUTPUT);
  for(int i = 0; i < 8; i++) {
    pinMode(SENSOR_PINS[i], OUTPUT);
  }

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
  
  // 1. Solicitar datos a las Nucleos cada 200ms aproximadamente
  static unsigned long lastRequest = 0;
  if (millis() - lastRequest > 200) {
    readFromBothNucleos();
    lastRequest = millis();
  }

  // 2. Control del LED de estado (GPIO2)
  digitalWrite(LED1pin, LED1status);
}

//************************************************************************************************
// Función I2C Maestro con Detección de Esclavo y Toggle
//************************************************************************************************
void readFromBothNucleos() {
  bool anyResponse = false;

  // ----------------------------------------
  // LECTURA DE NUCLEO 1 (0x10)
  // ----------------------------------------
  uint8_t bytesN1 = Wire.requestFrom(NUCLEO_1_ADDR, 1);
  if (bytesN1 > 0) {
    i2c_data_n1 = Wire.read(); 
    anyResponse = true;
    
    for (int i = 0; i < 4; i++) {
      bool sensorActive = (i2c_data_n1 >> i) & 0x01;
      digitalWrite(SENSOR_PINS[i], sensorActive ? HIGH : LOW);
      availability[i] = sensorActive ? PARKING_SPOT_OCCUPIED : PARKING_SPOT_FREE;
    }
  } else {
    // ALERTA: Falla en la Nucleo 1
    Serial.println("[I2C WARNING] Falla de comunicación con Nucleo 1 (0x10) - ¿Cable desconectado?");
  }

  // ----------------------------------------
  // LECTURA DE NUCLEO 2 (0x11)
  // ----------------------------------------
  uint8_t bytesN2 = Wire.requestFrom(NUCLEO_2_ADDR, 1);
  if (bytesN2 > 0) {
    i2c_data_n2 = Wire.read(); 
    anyResponse = true;

    for (int i = 0; i < 4; i++) {
      bool sensorActive = (i2c_data_n2 >> i) & 0x01;
      digitalWrite(SENSOR_PINS[i + 4], sensorActive ? HIGH : LOW); 
      availability[i + 4] = sensorActive ? PARKING_SPOT_OCCUPIED : PARKING_SPOT_FREE;
    }
  } else {
    // ALERTA: Falla en la Nucleo 2
    Serial.println("[I2C WARNING] Falla de comunicación con Nucleo 2 (0x11) - ¿Cable desconectado?");
  }

  // Control del Heartbeat
  if (anyResponse) {
    heartbeatStatus = !heartbeatStatus;
  } else {
    digitalWrite(HEARTBEAT_LED, LOW);
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
  Serial.println("Conexión al servidor detectada");
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
  String ptr = "<!DOCTYPE html>\n";
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

  for(uint8_t i = 0; i < 8; i++) {
    ptr += PrintParkingSpotRow(availability, i);
  }

  ptr += "</tbody>\n";
  ptr += "</table>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "<script src=https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js></script>\n";
  
  // =========================================================================
  // SCRIPT DE AUTO-REFRESCO CADA 10 SEGUNDOS (10000 ms)
  // =========================================================================
  ptr += "<script>\n";
  ptr += "  setInterval(function() {\n";
  ptr += "    window.location.reload();\n";
  ptr += "  }, 10000);\n";
  ptr += "</script>\n";
  // =========================================================================

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
void UpdateParkingSpot(uint8_t availability[8], uint8_t spot_id, uint8_t event){
  switch(event)
  {
    case LEAVE_PARKING_SPOT:
    availability[spot_id] = PARKING_SPOT_FREE;
    break;

    case ENTER_PARKING_SPOT:
    availability[spot_id] = PARKING_SPOT_OCCUPIED;
    break;
  
    default:
    break;
  }
}

// Imprimir texto HTML para fila
String PrintParkingSpotRow(uint8_t availability[], uint8_t spot_id) {
  // ID de parqueo (Usando el constructor String() de Arduino)
  String ptr  = "<tr id=parqueo-" + String(spot_id + 1) + ">\n";
  ptr += "<td class=fw-bold>" + String(spot_id + 1) + "</td>\n"; // Se agregó el '+' faltante

  // Disponibilidad
  if(availability[spot_id] == PARKING_SPOT_FREE) {
    ptr += "<td><span class=\"badge bg-success\">Libre</span></td>\n";
  }
  else if(availability[spot_id] == PARKING_SPOT_OCCUPIED) {
    ptr += "<td><span class=\"badge bg-danger\">Ocupado</span></td>\n";
  }
  
  // Guardar el tiempo en milisegundos
  ptr += "<td>" + String(millis()) + "</td>\n";
  ptr += "</tr>\n";
  
  return ptr;
}