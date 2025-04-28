//----------LIBRERIAS---------------//
#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

//----------PINES-------------------//
#define LED_BUILTIN_PIN 2   // D4
#define ESP_OFF 15          // D8 
#define SENSOR 13           // D7 
#define BATERIA_PIN A0      // A0
#define RESET 5             // D1

//-------VARIABLES DE ESTADO--------//
bool estadoSensorMagnetico = false;
const int intentosDeConexionWifi = 20;
bool modoAP = false;

// Ruta del archivo de credenciales JSON
const char* credentialsFile = "/credenciales.json";
const char* nombreSSIDAP = "Sensor-magnetico-wifi";
const char* passSSIDAP = "12345678";

// Variables para el parpadeo del LED
unsigned long previousMillis = 0;  // Almacena el último tiempo que se cambió el LED
int contadorSinInternet = 0;

// Variabled para el reset
bool estadoInicialBotonReset = false;

// Configuración del endpoint
const char* endpoint = "";
const char* apiKey = "";

// Estructura para almacenar SSID y contraseña
struct WiFiCredentials {
  String ssid;
  String password;
  String ip;
  String useStaticIp;
  String gateway;
  String subnet;
  String endpoint; 
};

ESP8266WebServer server(80);  // Crear un servidor web en el puerto 80

void handleRoot() {
  String html = "<!DOCTYPE html>";
  html += "<html lang='es'>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Configuración WiFi</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; background-color: #f4f4f9; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; }";
  html += ".container { background-color: #fff; border-radius: 10px; box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1); padding: 20px; width: 90%; max-width: 400px; text-align: center; }";
  html += "h1 { font-size: 24px; color: #333; }";
  html += "label { font-size: 14px; color: #666; display: block; margin: 10px 0 5px; text-align: left; }";
  html += "input { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ddd; border-radius: 5px; font-size: 16px; }";
  html += "button { width: 100%; padding: 10px; background-color: #28a745; border: none; border-radius: 5px; color: white; font-size: 16px; cursor: pointer; }";
  html += "button:hover { background-color: #218838; }";
  html += "footer { font-size: 12px; color: #aaa; margin-top: 15px; }";
  html += ".note { font-size: 12px; color: #888; margin-top: 5px; }";
  html += ".ip-fields { display: none; }"; // Ocultar los campos de IP por defecto
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<div class='container'>";
  html += "<h1>Configura tu dispositivo</h1>";
  html += "<form action='/save' method='POST'>";
  html += "<label for='ssid'>Nombre de la red WiFi (SSID):</label>";
  html += "<input type='text' id='ssid' name='ssid' placeholder='Ingrese el SSID' required>";
  html += "<label for='password'>Contraseña de WiFi:</label>";
  html += "<input type='password' id='password' name='password' placeholder='Ingrese la contraseña' required>";
  
  // Checkbox para habilitar/deshabilitar IP fija
  html += "<label><input type='checkbox' id='useStaticIp' onchange='toggleIpFields()'> Usar IP fija</label>";
  html += "<p class='note'>Se recomienda agregar IP fija, ya que optimiza la conexión al WiFi y hace que el rendimiento de la batería sea más eficiente.</p>";
  
  // Campos de IP fija (deshabilitados por defecto)
  html += "<div class='ip-fields' id='ipFields'>";
  html += "<label for='ip'>Dirección IP fija:</label>";
  html += "<input type='text' id='ip' name='ip' placeholder='Ejemplo: 192.168.1.100' maxlength='15' oninput='validateIp(this)'>";
  html += "<label for='gateway'>Gateway:</label>";
  html += "<input type='text' id='gateway' name='gateway' placeholder='Ejemplo: 192.168.1.1' maxlength='15' oninput='validateIp(this)'>";
  html += "<label for='subnet'>Máscara de subred:</label>";
  html += "<input type='text' id='subnet' name='subnet' placeholder='Ejemplo: 255.255.255.0' maxlength='15' oninput='validateIp(this)'>";
  html += "</div>";
  
  // Campo oculto para enviar el valor de useStaticIp
  html += "<input type='hidden' id='useStaticIpValue' name='useStaticIp' value='false'>";
  
  html += "<button type='submit'>Guardar configuración</button>";
  html += "</form>";
  html += "</div>";
  
  // Script para habilitar/deshabilitar los campos de IP fija y actualizar el campo oculto
  html += "<script>";
  html += "function toggleIpFields() {";
  html += "  var checkbox = document.getElementById('useStaticIp');";
  html += "  var ipFields = document.getElementById('ipFields');"; // Se obtuvo por ID";
  html += "  var useStaticIpValue = document.getElementById('useStaticIpValue');"; // Campo oculto para el valor de useStaticIp
  html += "  if (checkbox.checked) {";
  html += "    ipFields.style.display = 'block';"; // Mostrar los campos";
  html += "    useStaticIpValue.value = 'true';"; // Establecer el valor de useStaticIp a 'true'";
  html += "  } else {";
  html += "    ipFields.style.display = 'none';"; // Ocultar los campos";
  html += "    useStaticIpValue.value = 'false';"; // Establecer el valor de useStaticIp a 'false'";
  html += "  }";
  html += "}";
  
  // Lógica para validar la IP
  html += "function validateIp(input) {";
  html += "  var value = input.value.replace(/[^0-9.]/g, '');"; // Eliminar todo lo que no sea un número o punto
  html += "  var parts = value.split('.');";
  html += "  if (parts.length > 4) { value = value.substring(0, value.lastIndexOf('.')); }"; // Asegurar que no haya más de 4 partes";
  html += "  input.value = value;";
  html += "  var part = value.split('.');";
  html += "  if (part.length <= 4) {";
  html += "    for (var i = 0; i < part.length; i++) {";
  html += "      if (part[i].length > 3) { part[i] = part[i].substring(0, 3); }"; // Limitar a 3 caracteres por parte";
  html += "    }";
  html += "  }";
  html += "  input.value = part.join('.');";
  html += "}";
  html += "</script>";
  
  html += "</body>";
  html += "</html>";

  server.send(200, "text/html", html);
}


// Función para manejar la solicitud de guardar las credenciales
void handleSave() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  String useStaticIp = server.arg("useStaticIp"); // Detecta si el checkbox fue marcado
  String ip = server.arg("ip");
  String gateway = server.arg("gateway");
  String subnet = server.arg("subnet");

  if (ssid != "" && password != "") {
    if (password.length() < 8) {
      server.send(400, "text/html", "La contraseña debe tener al menos 8 caracteres.");
      return;
    }

    Serial.println("Guardando credenciales...");
    Serial.println("SSID: " + ssid);
    Serial.println("Se usara ip estatica: " + useStaticIp);
    if (useStaticIp == "true") {
      Serial.println("Usando IP fija:");
      Serial.println("IP: " + ip);
      Serial.println("Gateway: " + gateway);
      Serial.println("Subnet: " + subnet);
    } else {
      Serial.println("Usando DHCP.");
    }

    StaticJsonDocument<350> doc;
    doc["ssid"] = ssid;
    doc["password"] = password;
    doc["useStaticIp"] = useStaticIp; // Guardar si se usa IP fija
    if (useStaticIp) {
      doc["ip"] = ip;
      doc["gateway"] = gateway;
      doc["subnet"] = subnet;
    }

    if (!LittleFS.begin()) {
      Serial.println("Error al cargar sistema de archivos");
      return;
    }

    File file = LittleFS.open(credentialsFile, "w");
    if (!file) {
      server.send(500, "text/html", "Error al guardar las credenciales.");
      Serial.println("El archivo de credenciales no se puede abrir");
      return;
    }

    serializeJson(doc, file);
    file.close();

    server.send(200, "text/html", "Credenciales guardadas correctamente. Reinicia el ESP para aplicar los cambios.");
    delay(2000);
    ESP.restart();
  } else {
    server.send(400, "text/html", "Por favor, complete todos los campos.");
  }
}

bool conectarWifi(const char* ssid, const char* password, bool useStaticIp, const char* ip, const char* gateway, const char* subnet, int maxAttempts) {
  if (useStaticIp) {
    // Convertir Strings a IPAddress
    IPAddress ipAddr, gatewayAddr, subnetAddr;
    ipAddr.fromString(ip);
    gatewayAddr.fromString(gateway);
    subnetAddr.fromString(subnet);
    WiFi.config(ipAddr, gatewayAddr, subnetAddr);
  }

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < maxAttempts) {
    delay(1000);
    Serial.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado a la red WiFi. Dirección IP: " + WiFi.localIP().toString());
    return true;
  } else {
    Serial.println("\nNo se pudo conectar al WiFi.");
    return false;
  }
}

WiFiCredentials* loadWiFiCredentials() {
  if (!LittleFS.begin()) {
    Serial.println("Error al cargar sistema de archivos");
    return nullptr;
  }
    
  File file = LittleFS.open(credentialsFile, "r");
  if (!file) {
    Serial.println("No se encontró el archivo de credenciales.");
    return nullptr;
  }

  StaticJsonDocument<300> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.println("Error al leer el archivo JSON.");
    return nullptr;
  }

  WiFiCredentials* credentials = new WiFiCredentials();
  credentials->ssid = doc["ssid"].as<String>();
  credentials->password = doc["password"].as<String>();
  credentials->useStaticIp = doc["useStaticIp"].as<String>();
  credentials->ip = doc["ip"].as<String>();
  credentials->gateway = doc["gateway"].as<String>();
  credentials->subnet = doc["subnet"].as<String>();

  return credentials;
}

void checkButtonPress() {
  byte contadorReset = 0;  // Guardar el tiempo de inicio

  Serial.println("Se ha presionado el boton de reset, si dura mas de 5 segundos resetea el ESP y borra las credenciales...");
  // Esperar mientras el botón esté presionado
  while (!digitalRead(RESET)) {
    if(contadorReset == 5){
      Serial.println("Botón presionado por 5 segundos. Eliminando credenciales...");

      if (!LittleFS.begin()) {
        Serial.println("Error al cargar sistema de archivos");
      }
      
      // Borrar credenciales y reiniciar
      if (LittleFS.exists(credentialsFile)) {
        LittleFS.remove(credentialsFile);
        Serial.println("Credenciales eliminadas.");
      }

      Serial.println("Reiniciando el dispositivo...");
      ESP.restart();
    }
    delay(1000);
    contadorReset++;
    Serial.print("Intento ");
    Serial.println(contadorReset);
  }

  Serial.println("Botón liberado antes de los 5 segundos. Continuando ejecución...");
}


// Función para manejar el parpadeo del LED mientras está en modo AP
void handleLedBlink(const long interval) {
    unsigned long currentMillis = millis();  // Obtener el tiempo actual

    // Si ha pasado el intervalo de tiempo
    if (currentMillis - previousMillis >= interval) {
      // Guardar el último tiempo que cambió el LED
      previousMillis = currentMillis;

      // Leer el estado actual del LED (encendido o apagado) y cambiarlo
      int ledState = digitalRead(LED_BUILTIN_PIN);
      digitalWrite(LED_BUILTIN_PIN, !ledState);  // Cambiar el estado del LED

      // Si no esta en modod AP es porque no se conecto a internet
      if(!modoAP){
        if(contadorSinInternet == 50){
          Serial.print("Apagando ESP");
          digitalWrite(ESP_OFF, LOW);
        }
        contadorSinInternet++;
      }
    }

}

void enviarDatos() {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;

        String mac = WiFi.macAddress();  // Obtiene la MAC del ESP8266
        int estatus = !digitalRead(SENSOR); // Lee el estado del pin D7
        int porcentajeBateria = leerBateria(); // Obtiene el porcentaje de la batería

        // Construimos el JSON
        StaticJsonDocument<200> jsonDoc;
        jsonDoc["mac"] = mac;
        jsonDoc["estatus"] = estatus;
        jsonDoc["bateria"] = porcentajeBateria;

        String jsonStr;
        serializeJson(jsonDoc, jsonStr);

        Serial.println("Enviando JSON: " + jsonStr);

        http.begin(client, endpoint);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("x-api-key", apiKey); // Agregar API Key

        int httpResponseCode = http.POST(jsonStr); // Enviar JSON

        if (httpResponseCode > 0) {
            Serial.println("Respuesta del servidor: " + String(httpResponseCode));
        } else {
            Serial.println("Error en la solicitud HTTP");
        }

        http.end(); // Finaliza la conexión
    } else {
        Serial.println("WiFi no conectado, no se enviaron datos");
    }
}

int leerBateria() {
    int lectura = analogRead(BATERIA_PIN);  // Lee el voltaje (0 - 1023)
    
    // Convertir la lectura del ADC al voltaje real de la batería
    float voltaje = (lectura / 1023.0) * 4.2;  

    // Convertir el voltaje a porcentaje (basado en el rango 3.0V - 4.2V)
    int porcentaje = map(voltaje * 100, 300, 420, 0, 100);

    // Limitar el porcentaje a un rango de 0-100%
    if (porcentaje > 100) porcentaje = 100;
    if (porcentaje < 0) porcentaje = 0;

    return porcentaje;
}


void setup() {
  Serial.begin(115200);  // Inicializar comunicación serial
  
  // Declaracion de pines
  pinMode(LED_BUILTIN_PIN, OUTPUT);
  pinMode(ESP_OFF, OUTPUT);
  pinMode(SENSOR, INPUT);
  pinMode(RESET, INPUT);

  digitalWrite(LED_BUILTIN_PIN, LOW);  // Enciende el LED interno
  digitalWrite(ESP_OFF, HIGH);         // Salida para apagar el ESP

  // Obtiene el estado actual del boton de reset
  estadoInicialBotonReset = digitalRead(RESET);

  Serial.println("Montando sistema de archivos");
  if (!LittleFS.begin()) {
    Serial.println("Error al cargar sistema de archivos");
    return;
  }

  LittleFS.end();

  // Revisa si el boton de reset esta oprimido por 5 segundos
  if(!estadoInicialBotonReset){
    checkButtonPress();
  }

  // Intentar cargar las credenciales
  WiFiCredentials* credentials = loadWiFiCredentials();
  
  if (credentials != nullptr) {
    Serial.println("Credenciales cargadas exitosamente:");
    Serial.println("SSID: " + credentials->ssid);
    Serial.println("Password: " + credentials->password);
    
    // Conectar a WiFi usando las credenciales
    const bool conexionExitosa = conectarWifi(credentials->ssid.c_str(), credentials->password.c_str(), credentials->useStaticIp.c_str(), credentials->ip.c_str(), credentials->gateway.c_str(), credentials->subnet.c_str(), intentosDeConexionWifi);
    // Liberar la memoria del objeto creado
    delete credentials;
    // Valida si se conecto a la red exiosamente
    if(conexionExitosa){
      enviarDatos();
      digitalWrite(ESP_OFF, LOW); // Poner D8 en BAJO después de enviar los datos
    }    
  } else {
    // Setea el modo AP
    modoAP = true;
    // Si no se encuentran las credenciales, crear un servidor web
    Serial.println("No se encontraron las credenciales. Iniciando servidor web.");
    
    // Configurar el ESP8266 en modo AP (Access Point)
    WiFi.softAP(nombreSSIDAP, passSSIDAP);  // SSID y contraseña del AP
    Serial.println("Modo AP iniciado. Accede a http://192.168.4.1 para configurar las credenciales.");
    
    // Configurar rutas del servidor web
    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);

    // Iniciar el servidor web
    server.begin();
  }
}

void loop() {
  if(modoAP){
    server.handleClient();
    // Si esta en modo AP el led parpadea
    handleLedBlink(1000);
  }else{
    // Si no se apaga correctamente hace un blink de 100 ms para cualquier error
    handleLedBlink(100); 
  }
}
