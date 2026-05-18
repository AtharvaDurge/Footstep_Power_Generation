#include <WiFi.h>
#include <WebServer.h>

#define ADC_PIN 35
#define LED_PIN 26
#define THRESHOLD 1800
#define DEBOUNCE_MS 450

const char* ssid = "YourSSID";
const char* password = "YourPassword";

WebServer server(80);

int stepCount = 0;
unsigned long lastStep = 0;
float peakVoltage = 0.0;
float avgVoltage = 0.0;
int stepsPerMin = 0;

void handleRoot();
void handleEvents();
void detectStep();

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
  
  server.on("/events", handleEvents);
  server.begin();
}

void detectStep() {
  int raw = analogRead(ADC_PIN);
  float voltage = raw * (3.3f / 4095.0f);
  
  if (raw > THRESHOLD && (millis() - lastStep) > DEBOUNCE_MS) {
    stepCount++;
    lastStep = millis();
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
  }
}

void handleEvents() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/event-stream", "");

  while (true) {
    int raw = analogRead(ADC_PIN);
    float voltage = raw * (3.3f/4095.0f);
    
    float energy = 0.5 * 1000e-6f * voltage * voltage * 1000.0f;

    String data = "data: {\"steps\":" + String(stepCount) +
                  ",\"voltage\":" + String(voltage, 3) +
                  ",\"energy\":" + String(energy, 3) +
                  ",\"peak\":" + String(peakVoltage, 3) +
                  ",\"avg\":" + String(avgVoltage, 3) +
                  ",\"spm\":" + String(stepsPerMin) +
                  "}\n\n";

    server.sendContent(data);
    delay(200);
  }
}

void loop() {
  server.handleClient();
  detectStep();
}