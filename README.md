# Ascensor inteligente en entorno industrial ACME S.A.

Proyecto de instrumentación programable desarrollado con **Arduino UNO** y simulado en **Wokwi**, orientado a la implementación de un **ascensor inteligente de 5 plantas** con supervisión ambiental, lógica de control, actuación mediante sensores y actuadores, e interfaz local HMI mediante pantalla LCD.

---

## 1. Descripción del proyecto

Este proyecto forma parte del desarrollo de una práctica de automatización industrial centrada en la simulación de un ascensor inteligente dentro del contexto de transformación digital e Industria 4.0 planteado por **ACME S.A.**

El sistema implementado permite:

- gestionar llamadas a distintas plantas mediante pulsadores,
- simular el desplazamiento de la cabina mediante un servomotor,
- supervisar variables ambientales como **temperatura, humedad e iluminación**,
- detectar la presencia de usuarios en cabina mediante sensor ultrasónico,
- actuar sobre variables manipulables mediante LEDs,
- mostrar el estado del sistema mediante una **pantalla LCD I2C**,
- registrar eventos y medidas a través del **monitor serie**.

---

## 2. Objetivos

Los objetivos principales desarrollados en esta actividad han sido los siguientes:

- Implementar la lógica de funcionamiento de un ascensor de cinco plantas mediante pulsadores de llamada.
- Simular el desplazamiento de la cabina utilizando un servomotor como elemento de actuación.
- Gestionar múltiples solicitudes de plantas mediante una cola FIFO de llamadas.
- Supervisar en tiempo real variables ambientales del sistema.
- Detectar presencia de usuarios en la cabina.
- Aplicar un algoritmo de control de temperatura tipo **ON-OFF con zona muerta**.
- Regular la iluminación artificial mediante una salida PWM.
- Presentar el estado operativo del sistema mediante una interfaz HMI basada en pantalla LCD.
- Registrar información del proceso y eventos del sistema a través del monitor serie.

---

## 3. Componentes empleados

El sistema ha sido implementado utilizando los siguientes componentes:

- **Arduino UNO**
- **Sensor DHT22** para temperatura y humedad
- **Sensor LDR** para medición de iluminación
- **Sensor ultrasónico HC-SR04** para detección de presencia en cabina
- **Pantalla LCD 16x2 con interfaz I2C**
- **Servomotor** para simular el desplazamiento del ascensor
- **5 pulsadores** para solicitud de llamada a plantas
- **LED rojo** para simular calefacción
- **LED azul** para simular refrigeración
- **LED blanco PWM** para simular iluminación artificial
- **Protoboard y cableado**

---

## 4. Arquitectura del sistema

El sistema está formado por varios subsistemas funcionales integrados en un único programa sobre Arduino.

### 4.1. Subsistema de control
El microcontrolador Arduino UNO actúa como núcleo del sistema y se encarga de:

- ejecutar la lógica de control del ascensor,
- procesar las señales de entrada de sensores,
- gestionar las llamadas a plantas,
- controlar los actuadores,
- actualizar la interfaz LCD,
- ejecutar los algoritmos de control ambiental.

### 4.2. Subsistema de adquisición de datos
Este subsistema supervisa las condiciones ambientales de operación del ascensor mediante:

- **DHT22** para temperatura y humedad,
- **LDR** para iluminación ambiente,
- **sensor ultrasónico** para detección de presencia en cabina.

### 4.3. Subsistema de actuación
Se emplean varios actuadores para simular el comportamiento del sistema:

- **Servomotor**: simula el desplazamiento entre plantas.
- **LED rojo**: simula calefacción.
- **LED azul**: simula refrigeración.
- **LED blanco PWM**: simula iluminación artificial regulable.

### 4.4. Subsistema de interacción con el usuario
La interacción con el usuario se realiza mediante:

- **pulsadores** para indicar la planta solicitada,
- **pantalla LCD I2C** para mostrar el estado del ascensor y las variables medidas.

---

## 5. Arquitectura de software

El software se ha diseñado utilizando una arquitectura modular basada en funciones independientes y una máquina de estados.

### 5.1. Máquina de estados del ascensor

El comportamiento del ascensor se organiza en los siguientes estados:

| Estado | Descripción |
|--------|-------------|
| `IDLE` | Ascensor en reposo |
| `MOVIENDO` | Desplazamiento entre plantas |
| `LLEGANDO` | Simulación de llegada |
| `PUERTAS_ABIERTAS` | Simulación de apertura de puertas |

### 5.2. Gestión de llamadas
Las solicitudes de planta se gestionan mediante una **cola FIFO circular**, permitiendo almacenar múltiples llamadas pendientes y atenderlas secuencialmente. Este mecanismo evita la pérdida de solicitudes mientras el ascensor está ocupado.

### 5.3. Programación no bloqueante
La mayor parte del sistema se ha desarrollado mediante control temporal basado en `millis()`, evitando el uso intensivo de instrucciones bloqueantes. Esto se aplica a:

- movimiento progresivo del servomotor,
- actualización periódica del LCD,
- lectura de sensores,
- gestión temporal de estados,
- simulación de apertura de puertas.

Este enfoque permite una ejecución concurrente y un comportamiento más próximo al de un sistema embebido industrial real.

---

## 6. Desarrollo del programa

### 6.1. Estructura general

El funcionamiento principal del sistema se organiza a partir de la función `loop()`, en la que se ejecutan de forma cíclica las siguientes tareas:

- lectura de botones de llamada,
- movimiento del ascensor,
- actualización de sensores de planta,
- lectura de sensores ambientales,
- control de temperatura,
- control de iluminación,
- actualización de la pantalla LCD,
- detección de presencia en cabina.

Esta estructura permite integrar en un único dispositivo programable el control del ascensor, la supervisión ambiental, la actuación sobre variables manipulables y la presentación local de resultados.

### 6.2. Inicialización del sistema

En la función `setup()` se realiza la configuración inicial del sistema:

- inicialización del puerto serie a 9600 baudios,
- configuración de los cinco pulsadores como entradas `INPUT_PULLUP`,
- configuración de los LEDs de actuación,
- configuración del sensor ultrasónico,
- inicialización del display LCD I2C,
- inicialización del sensor DHT22,
- acoplamiento del servomotor,
- posicionamiento inicial del ascensor en planta 1,
- visualización del mensaje inicial de arranque.

### 6.3. Lógica de control del ascensor

Cada planta dispone de un pulsador de llamada. Cuando el usuario pulsa uno de ellos, el sistema detecta la solicitud y registra la planta deseada.

Para evitar errores debidos al rebote mecánico de los pulsadores, se ha incorporado un pequeño filtrado por software.

Si la planta solicitada es distinta de la planta actual, la llamada se añade a una **cola FIFO circular**, lo que permite almacenar varias solicitudes y atenderlas en orden de llegada.

El desplazamiento de la cabina se simula mediante un servomotor. Cada planta se asocia a una posición angular concreta:

- Planta 1 → 0°
- Planta 2 → 45°
- Planta 3 → 90°
- Planta 4 → 135°
- Planta 5 → 180°

El movimiento del ascensor se realiza de forma progresiva, variando la posición del servo grado a grado hasta alcanzar la posición objetivo. Para obtener una simulación más suave y realista, se ha establecido una temporización de **60 ms por paso**.

### 6.4. Supervisión de presencia en cabina

Para simular la detección de usuarios en cabina se ha incorporado un **sensor ultrasónico**. Si la distancia medida es inferior a **50 cm**, el sistema interpreta que la cabina está ocupada.

La variable `cabinaOcupada` se actualiza de forma periódica y su estado se muestra tanto en el monitor serie como en la pantalla LCD.

### 6.5. Medición de variables ambientales

El sistema supervisa tres variables principales:

- **temperatura**,
- **humedad relativa**,
- **iluminación ambiente**.

La temperatura y la humedad se obtienen mediante el sensor **DHT22**, mientras que el nivel de iluminación se mide con un **LDR** conectado a una entrada analógica.

Para evitar una sobrecarga de mensajes mientras el ascensor se desplaza, las lecturas ambientales solo se muestran cuando el ascensor está en estado `IDLE` o `PUERTAS_ABIERTAS`.

### 6.6. Algoritmo de control de temperatura

Para el control térmico se ha implementado un algoritmo **ON-OFF de tres posiciones con zona muerta**, tal y como se propone en el enunciado de la práctica.

Parámetros de control:

- **Temperatura de consigna:** 25 °C
- **Zona muerta:** ±2 °C

Lógica aplicada:

- si la temperatura es inferior a **23 °C**, se activa la calefacción,
- si la temperatura es superior a **27 °C**, se activa la refrigeración,
- si la temperatura está entre **23 °C y 27 °C**, ambas actuaciones permanecen desactivadas.

La actuación se representa mediante:

- **LED rojo** → calefacción
- **LED azul** → refrigeración

### 6.7. Algoritmo de control de iluminación

El control de iluminación se realiza a partir de la lectura del LDR y una salida PWM aplicada a un LED blanco que simula la iluminación artificial.

Se ha fijado un objetivo de iluminación del **80 %**. El programa calcula el error entre ese valor deseado y la iluminación ambiente medida:

- si la luz ambiente es suficiente, la salida PWM disminuye,
- si la luz ambiente es insuficiente, la salida PWM aumenta progresivamente.

Esto permite una regulación más suave y eficiente que una simple salida binaria.

### 6.8. Interfaz HMI mediante pantalla LCD

La información del sistema se presenta mediante una pantalla LCD 16x2 con interfaz I2C. Para optimizar el espacio, la pantalla rota automáticamente entre tres vistas:

1. **Estado del ascensor**
   - planta actual,
   - cabina ocupada o vacía,
   - estado del ascensor.

2. **Variables térmicas**
   - temperatura,
   - humedad,
   - acción de control activa.

3. **Iluminación**
   - luz ambiente,
   - valor PWM aplicado.

Además de la interfaz LCD, el sistema registra eventos y medidas a través del monitor serie.

---

## 7. Pruebas realizadas

Para validar el funcionamiento del sistema se realizaron diferentes pruebas en simulación.

### 7.1. Prueba de funcionamiento básico del ascensor
Se pulsaron distintas plantas y se comprobó que:

- la llamada quedaba registrada,
- el ascensor se desplazaba correctamente,
- el servo alcanzaba la posición asociada,
- el LCD mostraba el estado del proceso.

### 7.2. Prueba de múltiples llamadas
Se realizaron varias pulsaciones consecutivas para verificar que la cola FIFO almacenaba correctamente las solicitudes y las atendía en orden.

### 7.3. Prueba de presencia en cabina
Se modificó la distancia del sensor ultrasónico comprobando que:

- si la distancia era inferior a 50 cm, la cabina se marcaba como ocupada,
- si la distancia era superior, la cabina aparecía como vacía.

### 7.4. Prueba de control de temperatura
Se modificó la temperatura del DHT22 para comprobar que:

- por debajo de 23 °C se encendía el LED rojo,
- entre 23 °C y 27 °C no se activaba ningún LED térmico,
- por encima de 27 °C se encendía el LED azul.

### 7.5. Prueba de control de iluminación
Se modificó el nivel de luz del LDR observando que el valor PWM del LED blanco variaba de forma progresiva en función de la iluminación ambiente.

### 7.6. Prueba de interfaz LCD
Se verificó que la pantalla mostraba correctamente:

- planta y estado del ascensor,
- ocupación de cabina,
- temperatura y humedad,
- control térmico,
- iluminación y PWM.

---

## 8. Resultados y conclusiones

El sistema desarrollado cumple con los requisitos principales de la práctica, integrando en una única solución:

- control de un ascensor de 5 plantas,
- gestión de llamadas,
- supervisión ambiental,
- control térmico e iluminación,
- detección de presencia,
- visualización local mediante LCD,
- trazabilidad mediante monitor serie.

Entre sus principales ventajas destacan:

- diseño modular,
- facilidad de ampliación,
- uso de máquina de estados,
- control temporal mediante `millis()`,
- integración de sensores y actuadores en una sola plataforma.

Como posibles mejoras futuras se proponen:

- prioridad de llamadas,
- sensores de seguridad adicionales,
- apertura/cierre real de puertas,
- registro histórico de variables,
- comunicación IoT con plataforma en la nube.

---

## 9. Archivos del proyecto

Este repositorio incluye:

- `sketch.ino` → código fuente principal del sistema
- `diagram.json` → circuito de simulación en Wokwi
- `README.md` → documentación del proyecto

---

## 10. Simulación

La simulación del proyecto se ha realizado en **Wokwi**.

> Puedes añadir aquí el enlace directo a tu simulación cuando la tengas publicada.

Ejemplo:

```txt
https://wokwi.com/projects/TU_ID_DEL_PROYECTO
```

---

## 11. Autor

Proyecto realizado por **Miguel02frf**.
