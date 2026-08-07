#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_sleep.h>

#define ledRojo    18
#define ledVerde   19
#define ledAzul    21
#define pinBoton   GPIO_NUM_33

#define duracionModemSleepMs    6000
#define temporizadorLightSleepUs  8000000ULL
#define temporizadorDeepSleepUs   10000000ULL
#define segundosFaseActiva      10

enum EstadoSistema {
  estadoActivo = 0,
  estadoModemSleep,
  estadoLightSleep,
  estadoDeepSleep,
  estadoHibernacion
};

RTC_DATA_ATTR int contador = 0;
RTC_DATA_ATTR int conteoArranques = 0;
RTC_DATA_ATTR int estadoActual = estadoActivo;

void imprimirLinea(const String &s) {
  Serial.print(s);
  Serial.print("\r\n");
}

void imprimirLinea() {
  Serial.print("\r\n");
}

void iniciarSerial() {
  Serial.begin(115200);
  delay(300);
}

void establecerColorLed(bool r, bool g, bool b) {
  digitalWrite(ledRojo, r ? HIGH : LOW);
  digitalWrite(ledVerde, g ? HIGH : LOW);
  digitalWrite(ledAzul, b ? HIGH : LOW);
}

void iniciarLed() {
  pinMode(ledRojo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  establecerColorLed(false, false, false);
}

void iniciarBoton() {
  pinMode(pinBoton, INPUT_PULLUP);
}

void imprimirCausaDespertar() {
  esp_sleep_wakeup_cause_t causa = esp_sleep_get_wakeup_cause();

  switch (causa) {
    case ESP_SLEEP_WAKEUP_EXT0:
      imprimirLinea("Causa del despertar: boton externo");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      imprimirLinea("Causa del despertar: temporizador");
      break;
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    default:
      imprimirLinea("Causa de despertar: encendido / reset");
      break;
  }
}

void ejecutarFaseActiva() {
  establecerColorLed(false, true, false);
  imprimirLinea("Contador activo - LED verde");

  for (int i = 0; i < segundosFaseActiva; i++) {
    contador++;
    Serial.printf("Contador: %d\r\n", contador);
    delay(1000);
  }

  imprimirLinea("Pasando al siguiente estado de ahorro de energia");
}

void entrarModemSleep() {
  establecerColorLed(false, false, true);
  imprimirLinea("Modem Sleep - LED azul");

  WiFi.mode(WIFI_STA);
  WiFi.begin("Wokwi-GUEST", "");

  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 5000) {
    delay(200);
    Serial.print(".");
  }
  imprimirLinea();

  if (WiFi.status() == WL_CONNECTED) {
    imprimirLinea("WiFi conectado, activando Modem Sleep");
  } else {
    imprimirLinea("Sin conexion WiFi confirmada, activando Modem Sleep");
  }

  esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
  Serial.printf("Modem Sleep activo durante %d ms.\r\n", duracionModemSleepMs);
  delay(duracionModemSleepMs);

  esp_wifi_set_ps(WIFI_PS_NONE);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  imprimirLinea("Modem Sleep finalizado, radio WiFi apagado.");
}

void entrarLightSleep() {
  establecerColorLed(false, true, true);
  imprimirLinea("Light Sleep - LED cyan");

  esp_sleep_enable_timer_wakeup(temporizadorLightSleepUs);
  esp_sleep_enable_ext0_wakeup(pinBoton, 0);

  imprimirLinea("Entrando en modo Light Sleep");
  Serial.flush();

  esp_light_sleep_start();

  imprimirLinea("Se salio de Light Sleep.");
  imprimirCausaDespertar();
}

void entrarDeepSleep() {
  establecerColorLed(true, true, false);
  imprimirLinea("Deep Sleep - LED amarillo");

  esp_sleep_enable_timer_wakeup(temporizadorDeepSleepUs);
  esp_sleep_enable_ext0_wakeup(pinBoton, 0);

  imprimirLinea("Entrando en Deep Sleep. El sistema se reiniciara al despertar.");
  Serial.flush();

  esp_deep_sleep_start();
}

void entrarHibernacion() {
  establecerColorLed(true, false, false);
  imprimirLinea("Hibernation - LED rojo");

  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  esp_sleep_enable_ext0_wakeup(pinBoton, 0);

  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);

  imprimirLinea("Entrando en Hibernation. El sistema se reiniciara al despertar.");
  Serial.flush();

  esp_deep_sleep_start();
}

void setup() {
  iniciarSerial();
  iniciarLed();
  iniciarBoton();

  conteoArranques++;
  imprimirLinea();
  Serial.printf("Arranque numero: %d | Estado guardado: %d\r\n", conteoArranques, estadoActual);
  imprimirCausaDespertar();

  switch (estadoActual) {

    case estadoActivo:
      ejecutarFaseActiva();
      estadoActual = estadoModemSleep;
      entrarModemSleep();

      estadoActual = estadoLightSleep;
      entrarLightSleep();

      estadoActual = estadoDeepSleep;
      entrarDeepSleep();
      break;

    case estadoDeepSleep:
      estadoActual = estadoHibernacion;
      entrarHibernacion();
      break;

    case estadoHibernacion:
    default:
      contador = 0;
      estadoActual = estadoActivo;
      ejecutarFaseActiva();
      estadoActual = estadoModemSleep;
      entrarModemSleep();

      estadoActual = estadoLightSleep;
      entrarLightSleep();

      estadoActual = estadoDeepSleep;
      entrarDeepSleep();
      break;
  }
}

void loop() {}