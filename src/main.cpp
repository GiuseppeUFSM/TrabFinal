#include <Arduino.h>

//#include <FS.h>
#include <DHT.h>
#include <WiFi.h>
#include <SPIFFS.h>



#define WIFI_NETWORK "SUZANA 40 OI FIBRA 2G"
#define  WIFI_PASSWORD "10081958"
#define WIFI_TIMEOUT_MS 20000


/*
HARDWARE
*/
// DHT22 SENSOR
DHT my_sensor(5, DHT22);
//LED PINOUT
int LED = 2;

/*
 * Prototipos das tarefas
 */
void tarefa_1(void * parameters); //Ler temperatura
void tarefa_2(void * parameters); //Conectar e Manter WIFI
void tarefa_3(void * parameters); // Web Server
void tarefa_4(void * parameters); // Salvar as medias, semaforo
/*
Task Handlers
*/
xSemaphoreHandle sem_media = NULL;

/*
Globals
*/
volatile float temperature;
volatile float temp_med = 0;

/*
DEFAULT FUNCTIONS
*/
void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(9600);


  xTaskCreate(
    tarefa_1,  // nome pro os
    "get_temperature", // nome humano
    1000, // tamanho da pilha
    NULL, // parameters
    4,  //prioridade
    NULL // handle
  );

xTaskCreatePinnedToCore(
  tarefa_2,
  "Keep Alive",
  5000,
  NULL,
  2,
  NULL,
  CONFIG_ARDUINO_RUNNING_CORE
);
xTaskCreate(
  tarefa_3,
  "Web_Server",
  5000,
  NULL,
  1,
  NULL
);
xTaskCreate(
  tarefa_4,
  "Salvar Valores",
  8000,
  NULL,
  3,
  NULL
);

}

void loop() {
  // put your main code here, to run repeatedly:
  vTaskDelay(15);

}





/* Tarefas de exemplo que usam funcoes para suspender/continuar as tarefas */
void tarefa_1(void * parameters)
{
  my_sensor.begin();
  volatile float temp_sum = 0;
  volatile int temp_ind = 0;
  sem_media = xSemaphoreCreateBinary();

	for(;;)
	{
		temperature = my_sensor.readTemperature();        // Leitura do Sensor
		temp_ind ++;
		temp_sum = temp_sum + temperature;                // Soma 631 temps (20min)
    Serial.print("Temperatura: ");
    Serial.println(temperature);
		if(temp_ind == 631){
			temp_med = temp_sum/temp_ind;                   // Media de temperatura a cada 20m (631*1.9s)
			temp_ind = 0;
			temp_sum = 0;
      xSemaphoreGive(sem_media);                      // Libera para a funcao que grava
    Serial.print("Temperatura Media: ");
    Serial.println(temp_med);                         // So pra manter informado no Serial
		} 
		vTaskDelay(1900);
	}
}


void tarefa_2(void * parameters){
  for(;;){
		if(WiFi.status() == WL_CONNECTED){              // Verifica se o Wifi Ainda esta conectado
      Serial.println("[WIFI] Still Connected");
			vTaskDelay(31000 / portTICK_PERIOD_MS);       // Periodo de repeticao da verificacao
			continue;                                     // Ignora o restante da funcao
		}
		Serial.println("WiFI Connecting");
		WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);        // Caso tenha perdido a conexao, rotina de reconeccao

		unsigned long startAttemptTime = millis();      // contagem p/ timeout
		
    while (WiFi.status() != WL_CONNECTED && (millis() - startAttemptTime < WIFI_TIMEOUT_MS) ){}
		if(WiFi.status() != WL_CONNECTED){
            Serial.println("[WIFI] FAILED");        // TimedOut ao tentar reconectar
            vTaskDelay(21000 / portTICK_PERIOD_MS);
			  continue;
        }
    
    Serial.println("[WIFI] Connected: " + WiFi.localIP());   // Reconectou
	}
}

void tarefa_3(void * parameters){
  WiFiServer server(80);
  server.begin();
  for(;;){
    WiFiClient client = server.available();
    if (client) { 
      Serial.println("New Client.");
      String currentLine = "";
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          Serial.write(c);
          if (c == '\n') {
            if (currentLine.length() == 0) {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();
              client.println("<!DOCTYPE HTML>");
              client.println("<html>");
              client.println("<meta http-equiv=\"refresh\" content=\"2\" >");
              client.print(temperature);
              client.println("<br />");
              client.println();
              client.println("</html>");
              break;
            }
            else {
              currentLine = "";
            }
          } else if (c != '\r') {
            currentLine += c;
          }
          if (currentLine.endsWith("GET /H")) {
            digitalWrite(LED, HIGH);
          }
          if (currentLine.endsWith("GET /L")) {
            digitalWrite(LED, LOW);
          }
        }
      }
      client.stop();
      Serial.println("Client Disconnected.");
    }

  }
}

void tarefa_4(void * parameters){

  if(SPIFFS.begin()){
    Serial.println("Montou");
  }
  else{
      Serial.println("Erro de mount");
    }
  

  volatile int tempo = 0;         // tempo de execucao que foi obtido o dado (min)
  File file;
  
  for(;;){
      if(xSemaphoreTake(sem_media, portMAX_DELAY)){       // Verifica disponibilidade do semaforo
        Serial.println("Gravando...");
        if(file = SPIFFS.open("/media.txt", "a")){        //Grava [0]24.45 ...
          file.print("[");
          file.print(tempo);
          file.print("]");
          file.println(temp_med);
          file.close();
          Serial.println("Gravacao Concluida");
        }
        else{Serial.println("Erro na abertura do arquivo");}        
        tempo += 20;

      }
    }
  
}
