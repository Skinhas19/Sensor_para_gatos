#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h>

// WiFi
#define WIFI_SSID "A15 de Lucas"
#define WIFI_PASSWORD "*******"//arrumar senha caso precise

// Telegram
#define TELEGRAM_BOT_TOKEN "7857352930:AAF5077aQ7BxTvqLYliFebzZiofnHZJHao0"
#define CHAT_ID "7967251086"

#define Sensor 22  // GPIO22 da ESP32
bool movimentoDetectado = false;

TaskHandle_t detectarTaskHandle;
TaskHandle_t enviarTaskHandle;

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao WiFi!");
}

void detectarMovimentoTask(void *parameter) {
  pinMode(Sensor, INPUT);
  for (;;) {
    int Porta = digitalRead(Sensor);
    if (Porta == HIGH && !movimentoDetectado) {
      movimentoDetectado = true;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // Verifica a cada 100ms
  }
}

void enviarTelegramTask(void *parameter) {
  for (;;) {
    if (movimentoDetectado) {
      String mensagem = "🐱 Gato quer comida!";
      String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN) + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + urlEncode(mensagem);

      HTTPClient http;
      http.begin(url);
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        Serial.println("✅ Mensagem enviada para o Telegram.");
      } else {
        Serial.print("❌ Erro ao enviar: ");
        Serial.println(httpResponseCode);
      }

      http.end();

      // Espera o movimento cessar antes de permitir nova mensagem
      while (digitalRead(Sensor) == HIGH) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }

      movimentoDetectado = false;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS); // Verifica periodicamente
  }
}

void setup() {
  Serial.begin(115200);
  setupWiFi();

  xTaskCreatePinnedToCore(
    detectarMovimentoTask,
    "Detectar Movimento",
    2048,
    NULL,
    1,
    &detectarTaskHandle,
    1
  );

  xTaskCreatePinnedToCore(
    enviarTelegramTask,
    "Enviar Telegram",
    8192, // stack aumentada para evitar StackOverflow
    NULL,
    1,
    &enviarTaskHandle,
    0
  );
}

void loop() {
  // loop vazio pois usamos FreeRTOS
}
