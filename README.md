<h1 align="center" id="title">Control de Motor AC</h1>

<p id="description">En este código se realiza el control de un motor AC trifásico con un ESP32 que se comunica mediante MQTT a NodeRED donde se realiza una interfaz de usuario.<\p> La comunicación MQTT se encuentra basada en el ejemplo para ESP8266 de la librería PubSubClient.</p>

  
  
<h2>🧐 Características</h2>

Aqui se presentan las principales características del programa:

*   Enceder un motor AC trifásico a través de un variador de frecuencia.
*   Invertir el giro del motor AC.
*   Variar la velocidad del motor AC con pre-configuraciones.
*   Variar la velocidad del motor AC de forma dinámica por medio de un HMI.
*   Obtener una señal de retroalimentación para conocer la velocidad real de motor AC.
*   Comunicación MQTT para la publicación del valor leído en la realimentación y recibir las señales de control.

<h2>🍰 Autores:</h2>

Realizado por: Felix R. Navas V. - Alan Haro - Andy Robalino - Isaac Tayo

  
  
<h2>💻 Realizado con</h2>

Las tecnologías utilizadas son:

*   Mosquitto
*   Arduino IDE
*   ESP32
