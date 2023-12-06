/*
 Control de un motor AC con ESP32, NodeRED y el variador Micromaster 420.
 Basado en el ejemplo MQTT ESP8266

 En este código se realiza el control de un motor AC trifásico con un
 ESP32 que se comunica mediante MQTT a NodeRED donde se realiza una interfaz de usuario,
 con el objetivo de:

  -Enceder un motor AC trifásico a través de un variador de frecuencia.
  -Invertir el giro del motor AC.
  -Variar la velocidad del motor AC con pre-configuraciones.
  -Variar la velocidad del motor AC de forma dinámica por medio de un HMI.
  -Obtener una señal de retroalimentación para conocer la velocidad real de motor AC.
  
 */

#include <WiFi.h> //Libreria para el uso de WIFI
#include <PubSubClient.h> //Libreria para la creación de un cliente para la conexión MQTT
//Se declaran los puertos para el manejo de leds de advertencia
#define LED1 32 // Pin 7
#define LED2 33 // Pin 8
#define LED3 13 // Pin 15
#define LED4 26 // Pin 10
#define LED5 27 // Pin 11
#define LED6 14 // Pin 12
#define LED7 12 // Pin 13
//Se declaran los puertos de control del motor
#define ONM 2   // Pin 24
#define INVM 0  // Pin 25
#define OFFM 4  // Pin 26
#define ADC0 36 //Pin 23

int ac=0; //Variable global para leer el ADC0

// Se ingresan a continuación los datos de la red.

const char* ssid = "Android de Felix"; //Nombre de la red WiFi
const char* password = "12as34qw"; //Contraseña de la red WiFi
const char* mqtt_server = "192.168.253.184"; //Se ingresa el IP del broker MQTT
/*Nota: por defecto la conexión MQTT en este código se da por el puerto 1883
        en caso de se necesario usar otro puerto revisar la línea de código
        en el void setup() ->  client.setServer(mqtt_server, 1883); y cambiar
        donde se encuentra el 1883 que es el valor del puerto.
*/

WiFiClient espClient; //Se crea una instancia para manejar el WiFi
PubSubClient client(espClient); //Se crea una instancia para la comunicación MQTT apartir del objeto anterior 
unsigned long lastMsg = 0; 
#define MSG_BUFFER_SIZE  (50)
#define DAC_C1 25 //Convertidor Digital-Análogo
char msg[MSG_BUFFER_SIZE];
int value = 0;
int aux = 0;
boolean Rflag=false;  //Avisa si existe o no un mensaje MQTT nuevo en el main Loop
boolean onVar=false;  //Setea encendido del motor hacia la derecha
boolean invVar=false; //Setea encendido del motor hacia la izquierda
boolean onEqui=false; //Setea el apagado del equipo

void comproInv(){ //Comprueba si hay que hacer inversión de giro
 if(aux==0){
  if(invVar){
    aux=aux+1;
  }
 }
 if(aux==1){
    digitalWrite(INVM,HIGH);
    digitalWrite(ONM,HIGH);
    digitalWrite(OFFM,LOW);
    aux=0;
 }
}

void workVaria (int varMode){ //Función para enviar la señal de entrada
  if (!onEqui){
    digitalWrite(INVM,LOW);
    digitalWrite(ONM,LOW);
    digitalWrite(OFFM,HIGH);
    dacDisable(DAC_C1);
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,LOW);
    digitalWrite(LED3,LOW);
    digitalWrite(LED4,LOW);
    digitalWrite(LED5,LOW);
    digitalWrite(LED6,LOW);
    digitalWrite(LED7,LOW);
  return;
  }
  if(onVar || invVar){
    digitalWrite(ONM,HIGH);
    digitalWrite(OFFM,LOW);
    comproInv();
    //Código de DAC
    int i=(varMode*0.145134)-0.4354;
    dacWrite(DAC_C1,i);
    if (i<=25 && i>0){
      digitalWrite(LED1,HIGH);
      digitalWrite(LED2,LOW);
      digitalWrite(LED3,LOW);
      digitalWrite(LED4,LOW);
      digitalWrite(LED5,LOW);
      digitalWrite(LED6,LOW);
      digitalWrite(LED7,LOW);
    }
    else if (i<=48 && i>25){
      digitalWrite(LED1,HIGH);
      digitalWrite(LED2,HIGH);
      digitalWrite(LED3,LOW);
      digitalWrite(LED4,LOW);
      digitalWrite(LED5,LOW);
      digitalWrite(LED6,LOW);
      digitalWrite(LED7,LOW);
    }
    else if (i<=73 && i>48){
      digitalWrite(LED1,HIGH);
      digitalWrite(LED2,HIGH);
      digitalWrite(LED3,HIGH);
      digitalWrite(LED4,LOW);
      digitalWrite(LED5,LOW);
      digitalWrite(LED6,LOW);
      digitalWrite(LED7,LOW);
    }
    else if (i<=97 && i>73){
      digitalWrite(LED1,HIGH);
      digitalWrite(LED2,HIGH);
      digitalWrite(LED3,HIGH);
      digitalWrite(LED4,HIGH);
      digitalWrite(LED5,LOW);
      digitalWrite(LED6,LOW);
      digitalWrite(LED7,LOW);
    }
    else if (i<=121 && i>97){
      digitalWrite(LED1,HIGH);
      digitalWrite(LED2,HIGH);
      digitalWrite(LED3,HIGH);
      digitalWrite(LED4,HIGH);
      digitalWrite(LED5,HIGH);
      digitalWrite(LED6,LOW);
      digitalWrite(LED7,LOW);
    }
    else if(i<=145 && i>121){
      digitalWrite(LED1,HIGH);
      digitalWrite(LED2,HIGH);
      digitalWrite(LED3,HIGH);
      digitalWrite(LED4,HIGH);
      digitalWrite(LED5,HIGH);
      digitalWrite(LED6,HIGH);
      digitalWrite(LED7,LOW);
    }
    else if(i>145){
      digitalWrite(LED1,HIGH);
      digitalWrite(LED2,HIGH);
      digitalWrite(LED3,HIGH);
      digitalWrite(LED4,HIGH);
      digitalWrite(LED5,HIGH);
      digitalWrite(LED6,HIGH);
      digitalWrite(LED7,HIGH);
    }
  }
}

int workMode(String inf){
  int a=0xff;
  if(inf =="0" || inf =="3"){
    onEqui=false;
    onVar=false;
    invVar=false;
  }else if(inf =="1"){
    onEqui=true;
    onVar=true;
  }else if(inf =="2"){
    invVar=true;
    onEqui=true;
  }else{
    if(inf.toInt()!=0){
      a=inf.toInt();
    }
  }
  return a;
}

void setup_wifi() { //Se realiza la configuración y conexión WiFi

  delay(10);
  // Se inicia la conexión por WiFi
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid); //Se imprime en el monitor serial a que red se conecta

  WiFi.mode(WIFI_STA); //Se indica el modo de trabajo de la ESP32 
                       //en este caso como un dipositivo que se conecta
  WiFi.begin(ssid, password); //Se ingresan los datos para realizar la conexión

  while (WiFi.status() != WL_CONNECTED) { //Se crea un lazo para imprimir en el 
    delay(500);                           // monitor serial puntos '.' hasta que
    Serial.print(".");                    // se realice la conexión con la red.
  }

  randomSeed(micros());     //Se inicializa el generador de números aleatorios a partir
                            //del tiempo que lleva en funcionamiento de la ESP32.
  Serial.println("");                   //Se imprime en el monitor serial que
  Serial.println("WiFi connected");     //se ha conectado a la red y el IP del dispositivo.
  Serial.println("IP address: ");  
  Serial.println(WiFi.localIP());       //Método de WiFi para obtener el IP.
}

void callback(char* topic, byte* payload, unsigned int length) { //Se define la función callback que necesitará el PubSubClient
  char buff[length];
  Serial.print("Mensaje Recivido [");  //Se muestra en el monitor serial la información que llega a la ESP32.
  Serial.print(topic);                //El topic viene dado por la comunicación y este es de salida o entrada, para el callback
  Serial.print("] ");                 //se tiene un topic de entrada, por ejemplo, "inTopic".
  for (int i = 0; i < length; i++) {  //Se imprime el mensaje que llega de la comunicación MQTT.
    Serial.print((char)payload[i]);
  }
  Serial.println();
  //Parte del código para pasar mensaje al main Loop
  Rflag=true;
  int i=0;
  for (i; i < length; i++){
  buff[i]=(char)payload[i];
  }
  buff[i]='\0';
  
  snprintf (msg, MSG_BUFFER_SIZE, buff);
}

void reconnect() { // Se realiza la reconexión MQTT en caso de ser necesario 
  //Lazo para reconectar al ESP32
  while (!client.connected()) { //Se comprueba si el ESP esta conectado o no
    Serial.print("Intentado conexión MQTT..."); //Si no esta conectado se realoza:
          // Se crea un ID de cliente de forma aleatoria 
          //(Por se previamente se inicializó randomSeed()
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Se realiza in intento de conexión
    if (client.connect(clientId.c_str())) { //Se intenta la conexión dando el puntero de clientID
      Serial.println("conectado");
      // Se se realiza la conexión se publica un "Hola Mundo"
      client.publish("outTopic", "Hola Mundo");
      // ... luego se debe resubscribir
      client.subscribe("inTopic");
    } else { //En caso de no lograr la reconexión
      Serial.print("Error al conectar, rc=");   //Se muestra en el monitor serial un mensaje con el estado del cliente
      Serial.print(client.state());  //y que se reintentará en 5 segundos
      Serial.println(" se reintentará en 5 segundos");
      // Se crea un retardo de 5 segundos
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED1, OUTPUT);                   // Se define como salida a los pines de los LEDs y 
  pinMode(LED2, OUTPUT);                  //  los de manejo del micromaster
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(LED6, OUTPUT);
  pinMode(LED7, OUTPUT);

  pinMode(ONM, OUTPUT);
  pinMode(INVM, OUTPUT);
  pinMode(OFFM, OUTPUT);
  Serial.begin(9600);                     // Se inicia el contador serial
  setup_wifi();                           // Se llama a la función para la conexión WiFi
  client.setServer(mqtt_server, 1883);    // Se setea el servidor y puerto de la comunicación MQTT
  client.setCallback(callback);           // Se declara la función callback como la propia de client
}

void loop() {
  
  if (!client.connected()) { //Se revisa si se encuentra activa la conexión MQTT
    reconnect(); //Si no se está conectado se llama a reconectar
  }
  client.loop(); //Lee el buffer de datos y llama al callback si existe un msg

  unsigned long now = millis(); //Se obtiene el tiempo que se ha corrido el programa actual
  if (now - lastMsg > 2000) { // Se comprueba si han pasado más de 2 segundos
    lastMsg = now;            // Se almacena el tiempo que ha pasado
    ++value;                  // Se incrementa un contador
    if (Rflag){

    String mens= String(msg);
    int varMode = workMode(mens);
    Serial.print(varMode);
    Serial.println();
    workVaria(varMode);
    //client.publish("outTopic",msg);
    Rflag=false;
    }
    else{
    ac = analogRead(ADC0);
    Serial.print(ac);
    snprintf (msg, MSG_BUFFER_SIZE, "%ld", ac);
    
    /*Uso de snprintf():
    Este método permite almacenar dentro de buffer de un tamaño n, un string
    cuyo tamaño máximo es n, es decir, si el string a guardar es menor a n 
    elementos se almacenara todo el string, pero si el string a guardar es 
    mayor n elementos solo se almacenarán los primeros elementos que llenen
    el buffer. Su estructura es:
    snprintf(buffer,tamaño_buffer,cadena_de_caracteres,otras_opciones)
    Para este caso se está trabajando con chars y se tiene:
    > tamaño del buffer = MSG_BUFFER_SIZE
    > buffer = msg ->Array de char de tamaño del buffer
    > cadena de caracteres = "Hola Mundo #$ld" -> Se referencia a una variable tipo int
    > otras opciones = value -> Variable referenciada en la cadena
    
    Nota: msg es un buffer diferente al buffer que se menciona dentro la comunicación MQTT
    */
    
   
    
    Serial.print("Velocidad Actual ... "); //Se muestra el msg en el monitor serial
    Serial.println(msg);
    client.publish("outTopic", msg); //Publica un mensaje en el buffer de comunicación MQTT
  }
}
}
