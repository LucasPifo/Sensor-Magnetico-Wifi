# Sensor WiFi para Puertas, Cortinas y Ventanas con ATTiny85 y Wemos D1 Mini

Este proyecto desarrolla un sensor WiFi pensado para la detección de apertura y cierre en puertas, cortinas y ventanas, orientado principalmente a entornos industriales donde se requiere confiabilidad y flexibilidad.
También puede ser utilizado en entornos domésticos, montando tu propia infraestructura de red y servidor.

El sistema combina la eficiencia de un ATTiny85 para la gestión de eventos locales y la conectividad del Wemos D1 Mini (ESP8266) para la transmisión de eventos a un servidor HTTP o plataforma IoT.

Tanto el ATTiny85 como el Wemos D1 Mini son programables, lo que permite:
<li>Configurar la forma en que se detectan los eventos (apertura/cierre).</li>
<li>Personalizar la forma de envío (por ejemplo, hacer solicitudes HTTP a tu propio sistema).</li>
<li>Además, el ATTiny85 puede ser modificado para trabajar en base a intervalos de tiempo, utilizando librerías como <a href="https://github.com/connornishijima/TinySnore">TinySnore</a>, mejorando aún más la eficiencia energética y permitiendo distintas estrategias de notificación.</li>
![alt text](https://github.com/LucasPifo/Sensor-Magnetico-Wifi/blob/main/Sensor%20completo.jpeg?raw=true)

## Problemática que se buscaba resolver
En una planta industrial se requería un sistema para:
<li>Monitorear en tiempo real la apertura y cierre de cortinas.</li>
<li>Registrar el tiempo que permanecía abierta cada cortina.</li>
<li>Funcionar con baterías para maximizar el tiempo de operación.</li>
<li>Evitar instalaciones eléctricas complejas por cada sensor (sin necesidad de cableados especiales).</li>
<li>Facilitar el montaje y desmontaje de los sensores de forma rápida y sencilla.</li>
Este diseño permite cumplir todos esos requisitos, proporcionando una solución portátil, inalámbrica y de bajo consumo para entornos industriales exigentes.

## Decisiones de diseño
Una alternativa viable para este proyecto hubiera sido utilizar simplemente una compuerta lógica para la detección de eventos, eliminando así la necesidad de programar dos microcontroladores.
Sin embargo, nuestro enfoque está más orientado a la programación y la escalabilidad, por lo que decidimos integrar un ATTiny85 adicional. Sí, el costo es ligeramente mayor, pero obtenemos mayor flexibilidad.
El ATTiny85 permite escalar el sistema fácilmente.
Gracias a la programación, no solo podemos usar interrupciones físicas en los pines, sino también programar el sistema para que trabaje por intervalos de tiempo (ej: cada X segundos revisar estado), sin modificar significativamente el circuito existente.
Esta decisión nos permite tener un sistema más inteligente y modular, adaptable a distintos proyectos futuros simplemente cambiando el firmware.

## Diseño de hardware
Una de las prioridades en el diseño del circuito fue que los componentes fueran de montaje pasante (THT), con el fin de:
<li>Simplificar la soldadura.</li>
<li>No requerir herramientas o materiales especializados más allá de un cautín, soldadura y pasta para soldar.</li>
De esta manera, cualquier persona puede ensamblar el sensor de forma sencilla, sin necesidad de equipo costoso como estaciones de aire caliente.
Importante:
Todos los componentes son de tipo pasante, excepto un componente:
El MOSFET AO3413, que es de montaje superficial (SMD).

## Materiales
<li>1x Wemos D1 Mini (ESP8266)</li>
<li>1x ATTiny85</li>
<li>1x Batería 18650</li>
<li>1x Porta batería para 18650</li>
<li>1x MOSFET AO3413 (SMD)</li>
<li>2x Diodos de propósito general</li>
<li>2x Transistores 2N3904</li>
<li>1x Pulsador (push button)</li>
<li>1x Reed Switch (sensor magnético)</li>
<li>1x Imán</li>
<li>Resistencias: 220 Ω, 1 kΩ, 10 kΩ, 46 kΩ, 100 kΩ, 220 kΩ, 1 MΩ</li>

## Diagrama de conexión
![alt text](https://github.com/LucasPifo/Sensor-Magnetico-Wifi/blob/main/Esquema%20electronico.jpg?raw=true)

## PCB
![alt text](https://github.com/LucasPifo/Sensor-Magnetico-Wifi/blob/main/Dise%C3%B1o%20PCB.png?raw=true)

## Cómo funciona
El funcionamiento del sensor se basa en lograr máxima eficiencia energética y una respuesta inmediata ante eventos de apertura o cierre. El proceso completo es el siguiente:

### Modo de reposo de ultra bajo consumo:
El ATTiny85 se programa en modo deep sleep, logrando un consumo aproximado de 0.1 mA.
En este estado de reposo, la puerta, cortina o ventana permanece cerrada (Que es como comunmente va a pasar la mayoria del tiempo).

### Detección de apertura:
Cuando el reed switch cambia de estado genera una interrupción en el pin PB3 del ATTiny85.
Esto despierta el ATTiny85 durante un breve periodo (aproximadamente 500 ms), suficiente para activar el sistema de alimentación y encender el Wemos D1 Mini.

### Activación de la alimentación:
Gracias al MOSFET AO3413, el ATTiny85 enclava la alimentación del circuito.
Esto garantiza que el Wemos D1 Mini permanezca encendido de manera estable mientras transmite los datos.

### Comunicación del evento:
Una vez encendido, el Wemos D1 Mini se conecta automáticamente a la red WiFi almacenada en su memoria.
Lee el estado actual del reed switch (abierto o cerrado).
Envía un JSON con los datos al endpoint HTTP configurado en el firmware.

### Apagado controlado:
Al completar el envío de datos, el Wemos D1 Mini pone en nivel BAJO el pin ESP_OFF (GPIO 15).
Esto provoca que el MOSFET AO3413 corte la alimentación, regresando todo el sistema a su estado de reposo de bajo consumo.

### Reconfiguración WiFi (modo AP):
En caso de que el módulo deba ser reubicado a una nueva red WiFi:
La placa cuenta con un pulsador.
Al mantener presionado el pulsador durante 5 segundos al encender o apagar la placa:
El Wemos D1 Mini entrará en modo Access Point (AP).
Se creará una red WiFi con los siguientes datos:

**SSID: Sensor-magnetico-wifi**<br>
**Contraseña (PSK): 12345678**

Conectándose a esa red y accediendo a **http://192.168.4.1**, se podrá:
Agregar una nueva red WiFi.
**Asignar una IP fija al módulo (extremadamente recomendado).**
