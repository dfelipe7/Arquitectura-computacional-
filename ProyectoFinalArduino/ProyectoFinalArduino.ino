
/**
 * @brief Archivo principal del proyecto que incluye las bibliotecas y define las configuraciones iniciales.
 */

#include <Keypad.h>
#include <LiquidCrystal.h>
#include <LiquidMenu.h>
#include "AsyncTaskLib.h"
#include "StateMachineLib.h"
#include <Servo.h>
#include <DHT.h>

/** 
 * @brief Pin digital al que está conectado el sensor DHT22.
 * @details Define el pin al que está conectado el sensor DHT22 y el tipo de sensor.
 */
#define DHTPIN 10
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE); ///< Instancia del sensor DHT.

/** 
 * @brief Pin del LED verde, rojo y azul.
 * @details Define los pines para los LEDs verde, rojo y azul.
 */
#define LED_GREEN 6
#define LED_RED 7
#define LED_BLUE 8

/** 
 * @brief Pins para medir la luz y la temperatura/humedad.
 * @details Define los pines para medir la luz y la temperatura/humedad.
 */
#define PIN_LUZ A0
#define PIN_TEMYHUM 6

/** 
 * @brief Macro para imprimir mensajes de depuración en el Serial.
 * @details Define una macro para imprimir mensajes de depuración en el puerto serie.
 * @param a Mensaje a imprimir.
 */
#define DEBUG(a) Serial.print(millis()); Serial.print(": "); Serial.println(a)

/**
 * @brief Valor de la temperatura leída.
 * 
 * Almacena el valor de la temperatura leída del sensor DHT en grados Celsius.
 */
int valuetemp;

/**
 * @brief Pin analógico para el sensor de luz.
 * 
 * Define el pin analógico utilizado para leer el valor del sensor de luz.
 */
#define PIN_LUZ A1

/**
 * @brief Valor de la luz leída.
 * 
 * Almacena el valor de la luz leída del sensor conectado al pin PIN_LUZ.
 */
int valueluz;

/**
 * @brief Pin analógico para el sensor de gas.
 * 
 * Define el pin analógico utilizado para leer el valor del sensor de gas.
 */
#define PIN_GAS A0

/**
 * @brief Valor del gas leído.
 * 
 * Almacena el valor del gas leído del sensor conectado al pin PIN_GAS.
 */
int valuegas;


/// @brief Pin digital para el pushbutton.
/// 
/// Se define el pin digital utilizado para el pushbutton.
#define PIN_BUTTON 13

/// @brief Estado anterior del pushbutton.
///
/// Almacena el estado anterior del pushbutton para detectar cambios.
bool lastButtonState = HIGH;  // Estado inicial como HIGH debido a la resistencia pull-up interna

/// @brief Estado actual del botón.
bool currentButtonState;

/// @brief Bandera para indicar si el botón ha sido liberado.
bool buttonReleased = true;

bool buttonPressedLastLoop = false; // Variable para mantener el estado del botón en el ciclo anterior

// Variables globales
bool buttonState = HIGH;  // Estado inicial del botón (no presionado)
//bool lastButtonState = HIGH;  // Estado anterior del botón (inicializado igual al estado actual)
unsigned long lastDebounceTime = 0;  // Último tiempo de rebote del botón
unsigned long debounceDelay = 0;  // Retardo de rebote del botón en milisegundos

/** 
 * @brief Contraseña predeterminada y contraseña ingresada por el usuario.
 * @details Define la contraseña predeterminada y la contraseña ingresada por el usuario.
 */
String password = "1234";
String passIngresada;

/** 
 * @brief  Pin del buzzer activo.
 * @details Define el pin del buzzer activo.
 */

int buzzer = 9;


//-------------------------------------------------------------------------------------------
//Definir nota musical
//-------------------------------------------------------------------------------------------
/** 
 * @brief Asignación de pines para el teclado.
 */
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {22, 24, 26, 28};
byte colPins[COLS] = {30, 32, 34, 36};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

/** 
 * @brief Inicialización de la pantalla LCD.
 */
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/** 
 * @brief Instanciación de objetos para los botones.
 */
const bool pullup = true;
const byte pwmPin = 13;

/** 
 * @brief Variables para controlar un pin y mostrar el estado con texto.
 */
const byte ledPin = LED_BUILTIN;
bool ledState = LOW;
char ledState_text[4];
int temperatura = 25;
int tempHigh = 25;
int tempLow = 20;
int lightHigh = 300;
int lightLow = 100;
int hall = 400;

/** 
 * @brief Valores iniciales.
 */
const int initialTempHigh = 30;
const int initialTempLow = 25;
const int initialLightHigh = 300;
const int initialLightLow = 100;
const int initialHall = 300;

char string_on[] = "ON";
char string_off[] = "OFF";

/** 
 * @brief Variable 'analogValue' configurada posteriormente para ser impresa en la pantalla.
 */
const byte analogPin = A5;
unsigned short analogValue = 0;

/** 
 * @brief Configuración de pantallas LiquidCrystal.
 */
LiquidLine tempHigh_Line(0, 0, "TH-TempHigh:", tempHigh, "C");
LiquidScreen tempHigh_screen(tempHigh_Line);

LiquidLine tempLow_Line(0, 1, "TH-TemLow:", tempLow, "C");
LiquidScreen tempLow_screen(tempLow_Line);

LiquidLine lightHigh_Line(0, 0, "TH-LightHigh:", lightHigh);
LiquidScreen lightHigh_screen(lightHigh_Line);

LiquidLine lightLow_Line(0, 1, "TH-LightLow:", lightLow);
LiquidScreen lightLow_screen(lightLow_Line);

LiquidLine hall_Line(0, 0, "TH-Hall:", hall, "C");
LiquidScreen hall_screen(hall_Line);

LiquidLine reset_Line(0, 0, "Reset");
LiquidScreen reset_screen(reset_Line);

/** 
 * @brief Instanciación del menú LiquidMenu.
 */
LiquidMenu menu(lcd);
//---------------------------------------------------------------------------------------------
/**
 * @brief Definiciones de estados, entradas y tareas asociadas.
 */

/**
 * @brief Enumeración que define los posibles estados de la máquina de estados.
 */
enum State {
  INIT = 0, ///< Estado inicial.
  config = 1, ///< Estado de configuración.
  monitoreoAmb = 2, ///< Estado de monitoreo ambiental.
  monitoreoEvent = 3, ///< Estado de monitoreo de eventos.
  alarma = 4, ///< Estado de alarma.
  bloqueado = 5 ///< Estado bloqueado.
};

/**
 * @brief Enumeración que define las posibles entradas de la máquina de estados.
 */
enum Input {
  claveCorrecta = 0, ///< Clave correcta ingresada.
  bloqueo = 1, ///< Bloqueo activado.
  btn_Press = 2, ///< Botón presionado.
  TemLuzHigh = 3, ///< Temperatura o luz alta.
  gasHigh = 4, ///< Gas detectado.
  timeout_10 = 5, ///< Tiempo de espera de 10 segundos.
  timeout_5_2 = 6, ///< Tiempo de espera de 5 segundos.
  timeout_5 = 7, ///< Tiempo de espera de 5 segundos.
  timeout_2 = 8, ///< Tiempo de espera de 2 segundos.
  desconocido = 9 ///< Entrada desconocida.
};

/**
 * @brief Instancia de la máquina de estados.
 */
StateMachine stateMachine(6, 12);

/**
 * @brief Entrada actual de la máquina de estados.
 */
Input currentInput;

/**
 * @brief Funciones de temporización.
 */
void tmOut10(void) {
  currentInput = timeout_10;
}

void tmOut2(void) {
  currentInput = timeout_2;
}

void tmOut5(void) {
  currentInput = timeout_5;
}

void tmOut5_2(void) {
  currentInput = timeout_5_2;
}

/**
 * @brief Funciones para realizar tareas específicas.
 */
void seguridad();
void menu_config();
void readtemp(void);
void readlight(void);
void readgas();
void retorno();

/**
 * @brief Tareas asincrónicas.
 */
AsyncTask asyncTask_seguridad(100, seguridad);
AsyncTask asyncTask_config(100, true, menu_config);
AsyncTask TaskTemp(2500, true, readtemp);
AsyncTask TaskLuz(2500, true, readlight);
AsyncTask TaskGas(2000, true, readgas);
AsyncTask TaskRetorno(100, true, retorno);

AsyncTask asyncTask_10seg(10000, tmOut10);
AsyncTask asyncTask_5seg(5000, tmOut5);


/**
 * @brief Configura la máquina de estados y define las transiciones.
 */
void setupStateMachine()
{

  stateMachine.AddTransition(INIT, bloqueado, []() {
    return currentInput == bloqueo;
  });
  stateMachine.AddTransition(bloqueado, INIT, []() {
    return currentInput == timeout_10;
  });
  stateMachine.AddTransition(INIT, config, []() {
    return currentInput == claveCorrecta;
  });
  stateMachine.AddTransition(config, monitoreoAmb, []() {
    return currentInput == btn_Press;
  });
  stateMachine.AddTransition(monitoreoAmb, config, []() {
    return currentInput == btn_Press;
  });

  stateMachine.AddTransition(monitoreoAmb, alarma, []() {
    return currentInput == TemLuzHigh;
  });
  stateMachine.AddTransition(alarma, monitoreoAmb, []() {
    return currentInput == timeout_5;
  });
  stateMachine.AddTransition(alarma, INIT, []() {
    return currentInput == btn_Press;
  });

  stateMachine.AddTransition(monitoreoAmb, monitoreoEvent, []() {
    return currentInput == timeout_5_2;
  });
  stateMachine.AddTransition(monitoreoEvent, monitoreoAmb, []() {
    return currentInput == timeout_2;
  });
  stateMachine.AddTransition(monitoreoEvent, alarma, []() {
    return currentInput == gasHigh;
  });
  stateMachine.AddTransition(monitoreoEvent, config, []() {
    return currentInput == btn_Press;
  });

  stateMachine.SetOnEntering(INIT, input_init);
  stateMachine.SetOnEntering(config, input_config);
  stateMachine.SetOnEntering(bloqueado, input_bloqueado);
  stateMachine.SetOnEntering(monitoreoAmb, input_monitoreoAmb);
  stateMachine.SetOnEntering(monitoreoEvent, input_monitoreoEvent);
  stateMachine.SetOnEntering(alarma, input_alarma);

  stateMachine.SetOnLeaving(INIT, output_init);
  stateMachine.SetOnLeaving(config, output_config);
  stateMachine.SetOnLeaving(bloqueado, output_bloqueado);
  stateMachine.SetOnLeaving(monitoreoAmb, output_monitoreoAmb);
  stateMachine.SetOnLeaving(monitoreoEvent, output_monitoreoEvent);
  stateMachine.SetOnLeaving(alarma, output_alarma);
}

/**
 * @brief Función de entrada al estado INIT.
 */
void input_init()
{
  currentInput = static_cast<Input>(Input::desconocido);
  asyncTask_seguridad.Start();
}

/**
 * @brief Función de salida del estado INIT.
 */
void output_init()
{
  asyncTask_seguridad.Stop();
}


/**
 * @brief Función de entrada al estado config.
 */
void input_config()
{
  lcd.clear();
  lcd.print("MENU CONFIG");
  delay(1000);
  menu.add_screen(tempHigh_screen);
  menu.add_screen(tempLow_screen);
  menu.add_screen(lightHigh_screen);
  menu.add_screen(lightLow_screen);
  menu.add_screen(hall_screen);
  menu.add_screen(reset_screen);

  strncpy(ledState_text, string_off, sizeof(string_off));
  menu.update();

  asyncTask_config.Start();

}

/**
 * @brief Función de salida del estado de configuración.
 * Limpia la pantalla LCD y detiene la tarea de configuración.
 */
void output_config()
{
  lcd.clear();
  asyncTask_config.Stop();
}

/**
 * @brief Función de entrada al estado bloqueado.
 * Inicia la tarea de temporización de 10 segundos, activa el buzzer y enciende el LED rojo.
 * Muestra un mensaje en la pantalla LCD indicando que el sistema está bloqueado.
 */
void input_bloqueado()
{
  asyncTask_10seg.Start();
  tone(buzzer, 2000); // Hacer sonar el buzzer a 1000Hz
  digitalWrite(LED_RED, HIGH);
  lcd.print("Sist. Bloqueado");
}

/**
 * @brief Función de salida del estado bloqueado.
 * Detiene la tarea de temporización de 10 segundos, apaga el buzzer y el LED rojo.
 */
void output_bloqueado()
{
  noTone(buzzer); // Apagar el buzzer
  digitalWrite(LED_RED, LOW);
  asyncTask_10seg.Stop();
}

/**
 * @brief Función de entrada al estado de monitoreo ambiental.
 * Limpia la pantalla LCD, inicia las tareas de monitoreo de temperatura y luz, y configura el sensor DHT.
 * Muestra un mensaje en la pantalla LCD indicando que se está monitoreando el ambiente.
 */
void input_monitoreoAmb()
{
  lcd.clear();
  Serial.println("Monitoreo Ambiental");
  lcd.println("Ambiente... ");
  TaskTemp.Start();
  TaskLuz.Start();
  dht.begin();
}

/**
 * @brief Función de salida del estado de monitoreo ambiental.
 * Detiene las tareas de monitoreo de temperatura y luz.
 */
void output_monitoreoAmb()
{
  TaskTemp.Stop();
  TaskLuz.Stop(); 
}

/**
 * @brief Función de entrada al estado de monitoreo de eventos.
 * Limpia la pantalla LCD, inicia la tarea de retorno, y configura la tarea de monitoreo de gas.
 * Muestra un mensaje en la pantalla LCD indicando que se están monitoreando eventos.
 */
void input_monitoreoEvent()
{
  lcd.clear();
  TaskRetorno.Start(); 
  TaskGas.Start();
  Serial.println("Monitoreo Eventos");
  lcd.println("Eventos... ");
  delay(500);
}

/**
 * @brief Función de salida del estado de monitoreo de eventos.
 * Detiene la tarea de retorno y la tarea de monitoreo de gas.
 */
void output_monitoreoEvent()
{
  TaskRetorno.Stop();
  TaskGas.Stop();
}

/**
 * @brief Función de entrada al estado de alarma.
 * Inicia la tarea de temporización de 5 segundos, la tarea de retorno y activa el buzzer y el LED azul.
 * Muestra un mensaje en la pantalla LCD indicando que se ha activado la alarma.
 */
void input_alarma()
{
  lcd.clear();
  asyncTask_5seg.Start();
  TaskRetorno.Start(); 
  tone(buzzer, 2000); // Hacer sonar el buzzer a 1000Hz
  digitalWrite(LED_BLUE, HIGH);
  lcd.print("ALARMA");
  Serial.println("Alarma");
}

/**
 * @brief Función de salida del estado de alarma.
 * Detiene la tarea de retorno, la tarea de temporización de 5 segundos, el buzzer y el LED azul.
 */
void output_alarma()
{
  noTone(buzzer); // Apagar el buzzer
  digitalWrite(LED_BLUE, LOW);
  TaskRetorno.Stop();
  asyncTask_5seg.Stop();
}

/**
 * @brief Configura los pines y componentes necesarios.
 * Inicia la máquina de estados y establece el estado inicial.
 * Inicia la tarea de seguridad.
 */
void setup() {
  Serial.begin(9600);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(buzzer, OUTPUT); // Inicializar el pin del buzzer como salida

  pinMode(analogPin, INPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  lcd.begin(16, 2);

  tempHigh_Line.attach_function(1, tempHigh_up);
  tempHigh_Line.attach_function(2, tempHigh_down);
  tempLow_Line.attach_function(1, tempLow_up);
  tempLow_Line.attach_function(2, tempLow_down);
  lightHigh_Line.attach_function(1, lightHigh_up);
  lightHigh_Line.attach_function(2, lightHigh_down);
  lightLow_Line.attach_function(1, lightLow_up);
  lightLow_Line.attach_function(2, lightLow_down);
  hall_Line.attach_function(1, hall_up);
  hall_Line.attach_function(2, hall_down);
  reset_Line.attach_function(1, reset_values);

  lcd.print("Bienvenido");
  delay(1000);
  //Starting State Machine
  setupStateMachine();
  stateMachine.SetState(State::INIT, true, true);
}

/**
 * @brief Función principal del programa. Se ejecuta de forma continua.
 * Actualiza las tareas asíncronas, la máquina de estados y el estado de la entrada actual.
 */
void loop()
{
  //actualizacion de las tareas asincronicas
  asyncTask_seguridad.Update();
  asyncTask_config.Update();
  TaskTemp.Update();
  TaskLuz.Update();
  TaskGas.Update();
  TaskRetorno.Update();
  readButton();
  
  asyncTask_10seg.Update();
  asyncTask_5seg.Update();
  stateMachine.Update();
  currentInput = static_cast<Input>(Input::desconocido);


    
}

/**
 * @brief Función de seguridad para ingresar la clave.
 * Verifica la clave ingresada y actualiza el estado de entrada.
 * Si se ingresa la clave correcta, activa el LED verde y emite un sonido.
 * Si se ingresa una clave incorrecta, activa el LED azul y emite un sonido de error.
 * Si se ingresan tres claves incorrectas, activa el estado de bloqueo.
 * @param None
 * @return Nothing
 */
void seguridad() {
  int count = 0;
  unsigned long startTime;
  const unsigned long timeout = 10000; // 10 segundos

  do {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ingrese clave:");
    lcd.setCursor(0, 1);
    passIngresada = ""; // Resetear la clave ingresada
    startTime = millis(); // Registrar el tiempo inicial

    do {
      char key = keypad.getKey();
      if (key) {
        lcd.print('*');
        passIngresada += key;
        startTime = millis(); // Reiniciar el tiempo porque hubo una entrada
      }

      // Verificar si se ha excedido el tiempo de espera
      if (millis() - startTime > timeout) {
        lcd.clear();
        lcd.print("Clave incorrecta");
        tone(buzzer, 1000, 3000); // Encender el buzzer a una frecuencia de 1000Hz durante 3 segundos
        digitalWrite(LED_BLUE, HIGH);
        delay(1000);
        noTone(buzzer); // Apagar el buzzer
        digitalWrite(LED_BLUE, LOW);
        lcd.clear();
        count += 1;
        break; // Salir del bucle actual y pedir la clave nuevamente
      }
    } while (passIngresada.length() != password.length());

    if (passIngresada.equals(password)) {
      count = 5;
      lcd.clear();
      lcd.print("Clave correcta");
      tone(buzzer, 1000, 2000); // Encender el buzzer a una frecuencia de 1000Hz durante 1 segundo
      digitalWrite(LED_GREEN, HIGH);
      delay(1500);
      noTone(buzzer); // Apagar el buzzer
      digitalWrite(LED_GREEN, LOW);
      lcd.clear();
      passIngresada = "";
    } else if (passIngresada.length() == password.length()) {
      lcd.clear();
      lcd.print("Clave incorrecta");
      tone(buzzer, 1000, 3000); // Encender el buzzer a una frecuencia de 1000Hz durante 3 segundos
      digitalWrite(LED_BLUE, HIGH);
      delay(1000);
      noTone(buzzer); // Apagar el buzzer
      digitalWrite(LED_BLUE, LOW);
      lcd.clear();
      count += 1;
    }
  } while (count != 3 && count != 5);

  if (count == 3) {
    currentInput = static_cast<Input>(Input::bloqueo);
  }

  if (count == 5) {
    currentInput = static_cast<Input>(Input::claveCorrecta);
  }
}

/**
 * @brief Maneja la entrada del usuario para el retorno al estado anterior.
 * Esta función espera a que el usuario presione el botón y
 * establece la entrada como 'btn_Press' cuando se detecta esta acción.
 * @param None
 * @return Nothing
 */
void readButton(){
  // Leer el estado del botón
    bool reading = digitalRead(PIN_BUTTON);

    // Verificar si hay un cambio de estado del botón
    if (reading != lastButtonState) {
        // Reiniciar el temporizador de rebote
        lastDebounceTime = millis();
    }

    // Verificar si ha pasado el tiempo de rebote
    if ((millis() - lastDebounceTime) > debounceDelay) {
        // Verificar si el estado del botón ha cambiado de forma estable
        if (reading != buttonState) {
            // Actualizar el estado del botón
            buttonState = reading;

            // Si el botón ha sido presionado
            if (buttonState == LOW) {
                // Realizar alguna acción cuando el botón se presione
                currentInput = static_cast<Input>(Input::btn_Press);
            }
        }
    }

    // Actualizar el estado anterior del botón
    lastButtonState = reading;
}

/**
 * @brief Función para gestionar el menú de configuración.
 * Detecta las pulsaciones de teclas y realiza las acciones correspondientes
 * (siguiente opción, anterior opción, aumentar valor, disminuir valor, seleccionar opción, cambiar estado).
 * @param None
 * @return Nothing
 */
void menu_config() {
  char key = keypad.getKey();

  if (key) {
    Serial.println(key);
    Serial.println(tempHigh);
    Serial.println(tempLow);
    // Check all the buttons
    if (key == 'A') {
      Serial.println(F("Siguiente opcion"));
      menu.next_screen();
    }
    if (key == 'B') {
      Serial.println(F("Anterior opcion"));
      menu.previous_screen();
    }
    if (key == 'C') {
      Serial.println(F("Aumentar valor"));
      // Calls the function identified with one
      // for the focused line.
      menu.call_function(1);
    }
    if (key == 'D') {
      Serial.println(F("Disminuir valor"));
      menu.call_function(2);
    }
    if (key == '*') {
      Serial.println(F("Opcion seleccionada"));
      // Switches focus to the next line.
      menu.switch_focus();
    }
    if (key == '#') {
      Serial.println(F("Cambiar estado"));
      currentInput = static_cast<Input>(Input::btn_Press);
    }
  }

}


/**
 * @brief Lee la temperatura del sensor DHT y la muestra por el monitor serie.
 * Esta función lee la temperatura del sensor DHT y la muestra por el monitor serie.
 * Si la lectura falla, muestra un mensaje de error.
 * @param None
 * @return Nothing
 */
void readtemp(void) {
    // Leer la temperatura como grados Celsius
    valuetemp = dht.readTemperature();
    // Comprobar si la lectura ha fallado e intentarlo de nuevo
    if (isnan(temperatura)) {
        Serial.println("Fallo al leer del sensor DHT!");
        return;
    }
    // Imprimir la temperatura en el monitor serie
    Serial.print("Temperatura: ");
    Serial.print(valuetemp);
    Serial.println(" °C");
    // Imprimir la temperatura en el lcd
    lcd.clear();
    lcd.print("Temperatura: ");
    lcd.print(valuetemp);
    lcd.println("°C");
    delay(1000);
}

/**
 * @brief Lee el valor del sensor de luz y lo compara con los umbrales configurados.
 * Esta función lee el valor del sensor de luz, lo muestra por el monitor serie
 * y compara con los umbrales configurados de temperatura y luz alta.
 * Si la luz supera el umbral y la temperatura es alta, establece la entrada como
 * 'TemLuzHigh', de lo contrario, establece la entrada como 'timeout_5_2'.
 * @param None
 * @return Nothing
 */
void readlight(void) {
    long prevtime = micros();
    valueluz = analogRead(PIN_LUZ);
    // Imprimir la luz en el monitor serie
    Serial.print("Luz: ");
    Serial.println(valueluz);
    // Imprimir la luz en el lcd
    lcd.clear();
    lcd.print("Luz: ");
    lcd.println(valueluz);

    delay(1000);
    if(valueluz > lightHigh && valuetemp > tempHigh){
        currentInput = static_cast<Input>(Input::TemLuzHigh);
    } else {
        currentInput = static_cast<Input>(Input::timeout_5_2);
    }
}

/**
 * @brief Lee el valor del sensor de gas y establece la entrada en función del valor leído.
 * Esta función lee el valor del sensor de gas, lo muestra por el monitor serie
 * y compara con un umbral predefinido. Si el valor leído supera el umbral,
 * establece la entrada como 'gasHigh', de lo contrario, establece la entrada como 'timeout_2'.
 * @param None
 * @return Nothing
 */
void readgas() {
    valuegas = analogRead(PIN_GAS);
    // Imprimir hall en el monitor serie
    Serial.print("Gas: ");
    Serial.println(valuegas);
    // Imprimir hall en el lcd
    lcd.clear();
    lcd.print("Gas: ");
    lcd.println(valuegas);
    
    delay(1000);
    if(valuegas > hall){
        currentInput = static_cast<Input>(Input::gasHigh);
    } else {
        currentInput = static_cast<Input>(Input::timeout_2);
    }
}

/**
 * @brief Maneja la entrada del usuario para el retorno al estado anterior.
 * Esta función espera a que el usuario presione el botón '#' en el teclado y
 * establece la entrada como 'btn_Press' cuando se detecta esta acción.
 * @param None
 * @return Nothing
 */
void retorno() {
    char key = keypad.getKey();

    if (key) {
        Serial.println("Retorno");
        if (key == '#') {
            Serial.println(F("DONE"));
            currentInput = static_cast<Input>(Input::btn_Press);
        }
    }
}


/**
 * @brief Incrementa el valor de la temperatura alta en 5 grados.
 * Esta función incrementa el valor de la temperatura alta en 5 grados Celsius,
 * asegurándose de que no exceda los límites establecidos y actualiza el menú.
 * @param None
 * @return Nothing
 */
void tempHigh_up() {
    if (tempHigh < 50 && tempHigh + 5 >= tempLow) { 
        tempHigh += 5;
    } else {
        tempHigh = 50; 
    }
    menu.update();
}

/**
 * @brief Decrementa el valor de la temperatura alta en 5 grados.
 * Esta función decrementa el valor de la temperatura alta en 5 grados Celsius,
 * asegurándose de que no caiga por debajo de los límites establecidos y actualiza el menú.
 * @param None
 * @return Nothing
 */
void tempHigh_down() {
    if (tempHigh > tempLow + 5 && tempHigh - 5 >= 20) { 
        tempHigh -= 5;
    } else {
        tempHigh = tempLow; 
    }
    menu.update();
}

/**
 * @brief Incrementa el valor de la temperatura baja en 5 grados.
 * Esta función incrementa el valor de la temperatura baja en 5 grados Celsius,
 * asegurándose de que no exceda los límites establecidos y actualiza el menú.
 * @param None
 * @return Nothing
 */
void tempLow_up() {
    if (tempLow < 50 && tempLow + 5 <= tempHigh) { 
        tempLow += 5;
    } else {
        tempLow = tempHigh; 
    }
    menu.update();
}

/**
 * @brief Decrementa el valor de la temperatura baja en 5 grados.
 * Esta función decrementa el valor de la temperatura baja en 5 grados Celsius,
 * asegurándose de que no caiga por debajo de los límites establecidos y actualiza el menú.
 * @param None
 * @return Nothing
 */
void tempLow_down() {
    if (tempLow > 20 && tempLow - 5 >= 0) {
        tempLow -= 5;
    } else {
        tempLow = 20; 
    }
    menu.update();
}

// Funciones para la luz alta 
/**
 * @brief Incrementa el valor de la luz alta en 5 unidades.
 * Esta función incrementa el valor de la luz alta en 5 unidades,
 * asegurándose de que no exceda los límites establecidos y actualiza el menú.
 * @param None
 * @return Nothing
 */
void lightHigh_up() {
    if (lightHigh < 600 && lightHigh + 100 >= lightLow) { 
        lightHigh += 100;
    } else {
        lightHigh = 600; 
    }
    menu.update();
}

/**
 * @brief Decrementa el valor de la luz alta en 5 unidades.
 * Esta función decrementa el valor de la luz alta en 5 unidades,
 * asegurándose de que no caiga por debajo de los límites establecidos y actualiza el menú.
 * @param None
 * @return Nothing
 */
void lightHigh_down() {
    if (lightHigh > lightLow + 100 && lightHigh - 100 >= 100) { 
        lightHigh -= 100;
    } else {
        lightHigh = lightLow; 
    }
    menu.update();
}

/**
 * @brief Incrementa el valor de la luz baja en 5 unidades.
 * Esta función incrementa el valor de la luz baja en 5 unidades,
 * asegurándose de que no exceda los límites establecidos y actualiza el menú.
 * @param None
 * @return Nothing
 */
void lightLow_up() {
    if (lightLow < 600 && lightLow + 100 <= lightHigh) {
        lightLow += 100;
    } else {
        lightLow = lightHigh; 
    }
    menu.update();
}

/**
 * @brief Decrementa el valor de la luz baja en 5 unidades.
 * Esta función decrementa el valor de la luz baja en 5 unidades,
 * asegurándose de que no caiga por debajo de los límites establecidos y actualiza el menú.
 * @param None
 * @return Nothing
 */
void lightLow_down() {
    if (lightLow > 100 && lightLow - 100 >= 0) { 
        lightLow -= 100;
    } else {
        lightLow = 100; 
    }
    menu.update();
}

/**
 * @brief Incrementa el valor del sensor Hall en 5 unidades.
 * Esta función incrementa el valor del sensor Hall en 5 unidades,
 * asegurándose de que no exceda el límite superior establecido y actualiza el menú.
 * @param None
 * @return Nothing
 */
void hall_up() {
    if (hall < 700) {
        hall += 100;
    } else {
        hall = 700;
    }
    menu.update();
}

/**
 * @brief Decrementa el valor del sensor Hall en 5 unidades.
 * Esta función decrementa el valor del sensor Hall en 5 unidades,
 * asegurándose de que no caiga por debajo del límite inferior establecido y actualiza el menú.
 * @param None
 * @return Nothing
 */
void hall_down() {
    if (hall > 100) {
        hall -= 100;
    } else {
        hall = 100;
    }
    menu.update();
}

/**
 * @brief Restablece los valores de temperatura y luz a los predeterminados.
 * Esta función restablece los valores de temperatura alta, baja, luz alta y baja a los valores predeterminados,
 * así como el valor del sensor Hall, y actualiza el menú.
 * @param None
 * @return Nothing
 */
void reset_values() {
    Serial.println(F("Restableciendo valores a los predeterminados..."));
    tempHigh = initialTempHigh;
    tempLow = initialTempLow;
    lightHigh = initialLightHigh;
    lightLow = initialLightLow;
    hall = initialHall;
    menu.update();
    Serial.println(F("Valores restablecidos."));
}
