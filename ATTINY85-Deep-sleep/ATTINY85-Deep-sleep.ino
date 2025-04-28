#include <avr/sleep.h>
#include <avr/power.h>

// Definiciones de pines
#define BUTTON_PIN PB3  // Pin del pulsador (PB3, Pin 2)
#define LED_PIN PB4     // Pin del LED (PB4, Pin 3)

// Variable para almacenar el estado del pulsador
volatile bool buttonPressed = false;

void setup() {
  // Configura el pin del LED como salida
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Configura el pin del pulsador como entrada con pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Habilita la interrupción PCINT para el pin PB3
  GIMSK |= (1 << PCIE);   // Habilita las interrupciones PCINT
  PCMSK |= (1 << PCINT3); // Habilita la interrupción en PB3 (PCINT3)
  sei();                  // Habilita las interrupciones globales

  ADCSRA &= ~ bit(ADEN); // Deshabilita el conversion analogico digital para reducir el consumo

  //power_all_disable(); // Deshabilita todos los modulos

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void loop() {
  if (buttonPressed) {
    // Enciende el LED por 500 ms
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);

    // Reinicia la bandera
    buttonPressed = false;

    // Vuelve a dormir
    goToSleep();
  }else{
    // Vuelve a dormir
    goToSleep();
  }
}

// Función para entrar en modo de sueño profundo
void goToSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Modo de sueño profundo
  sleep_enable();                       // Habilita el modo de sueño
  sei();                                // Habilita las interrupciones globales
  sleep_cpu();                          // Entra en modo de sueño
  sleep_disable();                      // Deshabilita el modo de sueño al despertar
}

// Rutina de servicio de interrupción (ISR) para PCINT
ISR(PCINT0_vect) {
  buttonPressed = true;  // Cambia el estado de la bandera
}
