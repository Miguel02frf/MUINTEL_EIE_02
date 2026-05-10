#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Servo.h>

//
// ===================== DEFINICIÓN DE PINES =====================
//
#define DHTPIN 2
#define DHTTYPE DHT22

#define LED_CALEFACCION 6
#define LED_REFRIGERACION 7
#define LED_LUZ 5   // PWM

#define LDR A0
#define SERVO_PIN 9

#define TRIG_PIN 12
#define ECHO_PIN 13

//
// ===================== OBJETOS =====================
//
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);
Servo ascensorServo;

//
// ===================== BOTONES =====================
//
const int botones[5] = {3, 4, 8, 10, 11};
const int posiciones[5] = {0, 45, 90, 135, 180};

//
// ===================== VARIABLES ASCENSOR =====================
//
enum EstadoAscensor {
  IDLE,
  MOVIENDO,
  LLEGANDO,
  PUERTAS_ABIERTAS
};

EstadoAscensor estado = IDLE;

int plantaActual = 1;
int plantaDestino = 1;

int posicionServo = 0;
int posicionObjetivo = 0;

bool sensorPlanta[5] = {false, false, false, false, false};

bool enMovimiento = false;
bool cabinaOcupada = false;
float distanciaPersona = 0;

//
// ===================== COLA DE LLAMADAS =====================
//

const int MAX_COLA = 10;

int colaPlantas[MAX_COLA];

int inicioCola = 0;
int finCola = 0;
int llamadasPendientes = 0;

//
// ===================== VARIABLES AMBIENTALES =====================
//
float temperatura = 0.0;
float humedad = 0.0;

int valorLDR = 0;
int porcentajeLuz = 0;
int pwmLuz = 0;

//
// ===================== CONTROL TEMPERATURA =====================
//
const float TEMP_SETPOINT = 25.0;
const float TEMP_ZONA_MUERTA = 2.0; // ±2 ºC

bool calefaccionActiva = false;
bool refrigeracionActiva = false;


unsigned long ultimoCambioTemperatura = 0;

//
// ===================== LCD =====================
//
int pantallaLCD = 0;

//
// ===================== VARIABLES TEMPORALES =====================
//
unsigned long ultimoMovimientoServo = 0;
unsigned long ultimaLecturaSensores = 0;
unsigned long ultimaActualizacionLCD = 0;
unsigned long tiempoEstado = 0;

//
// ===================== DEBOUNCE BOTONES =====================
//
bool estadoBotones[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};
unsigned long ultimoDebounce[5] = {0, 0, 0, 0, 0};
const unsigned long tiempoDebounce = 50;

//
// ===================== SETUP =====================
//
void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 5; i++) {
    pinMode(botones[i], INPUT_PULLUP);
  }

  pinMode(LED_CALEFACCION, OUTPUT);
  pinMode(LED_REFRIGERACION, OUTPUT);
  pinMode(LED_LUZ, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.init();
  lcd.backlight();

  dht.begin();

  ascensorServo.attach(SERVO_PIN);
  posicionServo = 0;
  posicionObjetivo = 0;
  ascensorServo.write(posicionServo);

  lcd.setCursor(0, 0);
  lcd.print("Sistema listo");
  lcd.setCursor(0, 1);
  lcd.print("Planta 1");
  delay(2000);
  lcd.clear();
}

//
// ===================== LOOP PRINCIPAL =====================
//
void loop()
{
  leerBotones();

  moverAscensor();

  actualizarSensoresPlanta();

  leerSensores();

  controlarTemperatura();

  controlarIluminacion();

  actualizarLCD();

  detectarPresencia();
}

//
// ===================== LEER BOTONES =====================
//

void leerBotones()
{
  for(int i = 0; i < 5; i++)
  {
    if(digitalRead(botones[i]) == LOW)
    {
      delay(30);

      if(digitalRead(botones[i]) == LOW)
      {
        int plantaSolicitada = i + 1;

        // Evitar pedir misma planta actual
        if(plantaSolicitada != plantaActual)
        {
          agregarLlamada(plantaSolicitada);
        }

        while(digitalRead(botones[i]) == LOW)
        {
        }
      }
    }
  }

  // Si está libre, tomar siguiente llamada
  if(estado == IDLE && llamadasPendientes > 0)
  {
    obtenerSiguienteLlamada();

    estado = MOVIENDO;

    enMovimiento = true;

    Serial.print("Atendiendo planta ");
    Serial.println(plantaDestino);
  }
}

//
// ===================== MOVER ASCENSOR =====================
//
void moverAscensor() {
  if (estado == MOVIENDO) {
    // Más lento: un paso cada 60 ms
    if (millis() - ultimoMovimientoServo >= 60) {
      ultimoMovimientoServo = millis();

      if (posicionServo < posicionObjetivo) {
        posicionServo++;
      } else if (posicionServo > posicionObjetivo) {
        posicionServo--;
      }

      ascensorServo.write(posicionServo);

      if (posicionServo % 5 == 0) {
        Serial.print("Posicion servo: ");
        Serial.println(posicionServo);
      }

      if (posicionServo == posicionObjetivo) {
        plantaActual = plantaDestino;
        estado = LLEGANDO;
        tiempoEstado = millis();

        Serial.println("Ascensor llegando...");
      }
    }
  }
  else if (estado == LLEGANDO) {
    if (millis() - tiempoEstado >= 2000) {
      estado = PUERTAS_ABIERTAS;
      tiempoEstado = millis();

      Serial.println("Puertas abiertas");
    }
  }
  else if (estado == PUERTAS_ABIERTAS) {
    if (millis() - tiempoEstado >= 3000) {
      estado = IDLE;
      enMovimiento = false;

      Serial.println("Ascensor disponible");
    }
  }
}


//
// ===================== LEER SENSORES =====================
//
void leerSensores() {
  if (estado != IDLE && estado != PUERTAS_ABIERTAS) {
    return;
  }

  if (millis() - ultimaLecturaSensores >= 5000) {
    ultimaLecturaSensores = millis();

    float tempLeida = dht.readTemperature();
    float humLeida = dht.readHumidity();

    if (!isnan(tempLeida)) {
      temperatura = tempLeida;
    }

    if (!isnan(humLeida)) {
      humedad = humLeida;
    }

    valorLDR = analogRead(LDR);

    porcentajeLuz = map(valorLDR, 0, 1023, 100, 0);
    porcentajeLuz = constrain(porcentajeLuz, 0, 100);

    Serial.print("Temp: ");
    Serial.print(temperatura);
    Serial.print(" C | Hum: ");
    Serial.print(humedad);
    Serial.print(" % | Luz: ");
    Serial.print(porcentajeLuz);
    Serial.println(" %");
    Serial.print(" | Cabina: ");
    if(cabinaOcupada)
    {
      Serial.println("Ocupada");
    }
    else
    {
      Serial.println("Vacia");
    }
  }
}

//
// ===================== CONTROL TEMPERATURA =====================
//
void controlarTemperatura() {
  float limiteInferior = TEMP_SETPOINT - TEMP_ZONA_MUERTA;
  float limiteSuperior = TEMP_SETPOINT + TEMP_ZONA_MUERTA;

  if (temperatura < limiteInferior) {
    calefaccionActiva = true;
    refrigeracionActiva = false;
  }
  else if (temperatura > limiteSuperior) {
    calefaccionActiva = false;
    refrigeracionActiva = true;
  }
  else {
    calefaccionActiva = false;
    refrigeracionActiva = false;
  }

  digitalWrite(LED_CALEFACCION, calefaccionActiva ? HIGH : LOW);
  digitalWrite(LED_REFRIGERACION, refrigeracionActiva ? HIGH : LOW);

  // Simulación simple del efecto del control:
  // +0.5 ºC cada 500 ms si calefacción activa
  // -0.5 ºC cada 500 ms si refrigeración activa
  if (millis() - ultimoCambioTemperatura >= 500) {
    ultimoCambioTemperatura = millis();

    if (calefaccionActiva) {
      temperatura += 0.5;
    } else if (refrigeracionActiva) {
      temperatura -= 0.5;
    }
  }
}

//
// ===================== CONTROL ILUMINACION =====================
//
void controlarIluminacion() {
  // Objetivo: mantener aproximadamente 80% de iluminación
  // Si hay poca luz ambiente, aumenta la luz artificial
  // Si hay mucha luz ambiente, la disminuye
  int errorLuz = 80 - porcentajeLuz;

  if (errorLuz <= 0) {
    pwmLuz = 0;
  } else {
    pwmLuz = map(errorLuz, 0, 80, 0, 255);
  }

  pwmLuz = constrain(pwmLuz, 0, 255);

  analogWrite(LED_LUZ, pwmLuz);
}

//
// ===================== ACTUALIZAR LCD =====================
//
void actualizarLCD() {
  if (millis() - ultimaActualizacionLCD >= 2000) {
    ultimaActualizacionLCD = millis();

    pantallaLCD++;
    if (pantallaLCD > 2) {
      pantallaLCD = 0;
    }

    lcd.clear();

    if (pantallaLCD == 0) {
      lcd.setCursor(0, 0);
      lcd.print("Planta:");
      lcd.print(plantaActual);

      lcd.setCursor(10, 0);
      if(cabinaOcupada)
      {
        lcd.print("ON");
      }
      else
      {
        lcd.print("OFF");
      }

      lcd.setCursor(0, 1);
      if (estado == IDLE) {
        lcd.print("Ascensor libre");
      } else if (estado == MOVIENDO) {
        lcd.print("Yendo a P");
        lcd.print(plantaDestino);
      } else if (estado == LLEGANDO) {
        lcd.print("Llegando...");
      } else if (estado == PUERTAS_ABIERTAS) {
        lcd.print("Puertas abiertas");
      }
    }
    else if (pantallaLCD == 1) {
      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(temperatura, 1);
      lcd.print((char)223);
      lcd.print("C H:");
      lcd.print(humedad, 0);
      lcd.print("%");

      lcd.setCursor(0, 1);
      if (calefaccionActiva) {
        lcd.print("Calefaccion ON");
      } else if (refrigeracionActiva) {
        lcd.print("Refrigeracion");
      } else {
        lcd.print("Temp estable");
      }
    }
    else if (pantallaLCD == 2) {
      lcd.setCursor(0, 0);
      lcd.print("Luz amb:");
      lcd.print(porcentajeLuz);
      lcd.print("%");

      lcd.setCursor(0, 1);
      lcd.print("PWM luz:");
      lcd.print(pwmLuz);
    }
  }
}

//
// ===================== AGREGAR LLAMADA =====================
//

void agregarLlamada(int planta)
{
  // Evitar desbordamiento
  if(llamadasPendientes >= MAX_COLA)
  {
    Serial.println("Cola llena");
    return;
  }

  // Guardar planta
  colaPlantas[finCola] = planta;

  // Avanzar índice circular
  finCola = (finCola + 1) % MAX_COLA;

  llamadasPendientes++;

  Serial.print("Llamada agregada P");
  Serial.println(planta);
}

//
// ===================== OBTENER SIGUIENTE LLAMADA =====================
//

bool obtenerSiguienteLlamada()
{
  if(llamadasPendientes == 0)
  {
    return false;
  }

  plantaDestino = colaPlantas[inicioCola];

  inicioCola = (inicioCola + 1) % MAX_COLA;

  llamadasPendientes--;

  posicionObjetivo = posiciones[plantaDestino - 1];

  return true;
}

//
// ===================== ACTUALIZAR SENSORES DE PLANTA =====================
//

void actualizarSensoresPlanta()
{
  for(int i = 0; i < 5; i++)
  {
    sensorPlanta[i] = false;

    // Tolerancia de ±2 grados
    if(abs(posicionServo - posiciones[i]) <= 2)
    {
      sensorPlanta[i] = true;
    }
  }
}

//
// ===================== DETECCION PRESENCIA =====================
//

void detectarPresencia()
{
  long duracion;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  duracion = pulseIn(ECHO_PIN, HIGH);

  distanciaPersona = duracion * 0.034 / 2;

  // Filtrado simple
  if(distanciaPersona > 0 && distanciaPersona < 50)
  {
    cabinaOcupada = true;
  }
  else
  {
    cabinaOcupada = false;
  }
}
