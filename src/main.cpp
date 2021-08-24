#include <Arduino.h>

#include <DHT.h>
#include <WiFi.h>

#define WIFI_NETWORK = "SUZANA 40 OI FIBRA 2G";
#define  WIFI_PASSWORD = "10081958";
#define WIFI_TIMEOUT_MS = 20000;


/*
DHT SENSOR
*/
DHT my_sensor(5, DHT22);


/*
 * Prototipos das tarefas
 */
void tarefa_1(void * parameters); //Ler temperatura
void tarefa_2(void * parameters); //Conectar e Manter WIFI
/*
Task Handlers
*/


/*
Globals
*/
volatile float temperature;
volatile float temp_sum = 0;
volatile int temp_ind = 0;
volatile float temp_med = 0;

/*
DEFAULT FUNCTIONS
*/
void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  my_sensor.begin();

  xTaskCreate(
    tarefa_1,  // nome pro os
    "get_temperature", // nome humano
    configMINIMAL_STACK_SIZE + 24, // tamanho da pilha
    NULL, // parameters
    2,  //prioridade
    NULL // handle
  );


}

void loop() {
  // put your main code here, to run repeatedly:

}





/* Tarefas de exemplo que usam funcoes para suspender/continuar as tarefas */
void tarefa_1(void * parameters)
{
	for(;;)
	{
		temperature = my_sensor.readTemperature();
		med_ind ++;
		temp_sum = temp_sum + temp_sum;
		if(med_ind = 6){
			temp_med = temp_sum/temp_ind;
			temp_ind = 0;
			temp_sum = 0;
		} 
		vTaskDelay(10000);
	}
}


void tarefa_2(void * parameters){
	fo(;;){
		if(wiFi.status() == WL_CONNECTED){
			vTaskDelay(20000 / portTICK_PERIOD_MS);
			continue;
		}
		Serial.println("WiFI Connecting");
		WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
		unsigned long startAttemptTime = millis(); // contagem p/ timeout
		while (WiFi.status() != WL_CONNECTED && millis() - startAttmptTime < WIFI_TIMEOUT_MS){
			Serial.print(".");
		}
		If{
			
		}

	}

}