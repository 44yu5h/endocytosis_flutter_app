#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// Replace with your network credentials
const char *ssid = "mini";
const char *password = "endocyto";

TaskHandle_t Task;

// Define LED pin (for feedback or debugging purposes)
bool isAutomaticMode = false;
bool membraneFormed = false;
int currentStage = 0;

Servo servo;
bool stageChanged = false;
int brightness = 0;
int fadeAmount = 5;
int s2 = 4;
int s3 = 27;
int s4 = 25;
int s5 = 33;

// Initialize the WebServer on port 80
WebServer server(80);

// Function declarations
void handleControl();
void flashLED(int led = 1);

void setup() {
  Serial.begin(115200);
  delay(100);
  // xTaskCreatePinnedToCore(
  //     core0,  /* Task function. */
  //     "Task", /* name of task. */
  //     10000,  /* Stack size of task */
  //     NULL,   /* parameter of the task */
  //     1,      /* priority of the task */
  //     &Task,  /* Task handle to keep track of created task */
  //     0);     /* pin task to core 0 */
  // delay(500);

  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(s5, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(s4, OUTPUT);
  servo.attach(2);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Configure HTTP server routes
  server.on("/control", HTTP_POST, handleControl);

  // Start server
  server.begin();
  Serial.println("HTTP server started");

  // Flash LED twice to indicate setup is complete
  flashLED(2);
}

void goFlashy() {

  for (int i = 0; i < 8; i++) {
    digitalWrite(s2, HIGH);
    digitalWrite(s3, LOW);
    digitalWrite(s4, HIGH);
    digitalWrite(s5, LOW);
    delay(200);
    digitalWrite(s2, LOW);
    digitalWrite(s3, HIGH);
    digitalWrite(s4, LOW);
    digitalWrite(s5, HIGH);
    delay(200);
  }

  // digitalWrite(s2, HIGH);
  // digitalWrite(s3, HIGH);
  // digitalWrite(s4, HIGH);
  // digitalWrite(s5, HIGH);
  // delay(3000);

  int maxBrightness = 255;  // Maximum brightness
  int minBrightness = 0;    // Minimum brightness
  int fadeAmount2 = 5;       // Amount by which brightness changes
  int delayTime = 2;
  int brightness = maxBrightness;

  for (int i=0;i<8;i++) {
    // Fade down
    while (brightness > minBrightness) {
      analogWrite(s2, brightness);
      analogWrite(s3, brightness);
      analogWrite(s4, brightness);
      analogWrite(s5, brightness);
      brightness -= fadeAmount2;
      delay(14);
    }
    // Fade up
    while (brightness < maxBrightness) {
      analogWrite(s2, brightness);
      analogWrite(s3, brightness);
      analogWrite(s4, brightness);
      analogWrite(s5, brightness);
      brightness += fadeAmount2;
      delay(14);
    }
  }
}


// Function to handle /control requests
void handleControl() {
  // Parse the received JSON
  StaticJsonDocument<200> jsonDoc;
  if (server.hasArg("plain")) {
    Serial.println("Received data: " + server.arg("plain"));
    DeserializationError error = deserializeJson(jsonDoc, server.arg("plain"));
    if (error) {
      Serial.println("Failed to parse JSON");
      server.send(400, "application/json", "{\"status\":\"bad_request\"}");
      return;
    }

    String command = jsonDoc["command"];
    Serial.println("Command received: " + command);

    // Handle commands
    if (command == "flashy") {
      goFlashy();
    }

    else if (command.startsWith("stage_")) {
      stageChanged = true;
      currentStage = command.substring(6).toInt();
      Serial.print("Changing to stage ");
      Serial.println(currentStage);
    }

    else if (command == "form") {
      Serial.println("Form membrane command received");
      // servo.write(0);
      // delay(500);
      // servo.write(90);
    }

    else if (command == "release") {
      Serial.println("Release membrane command received");
      // servo.write(180);
      // delay(500);
      // servo.write(90);
    }

    else {
      Serial.println("Unknown command");
      server.send(400, "application/json", "{\"status\":\"unknown_command\"}");
      return;
    }

    // Respond with success
    server.send(200, "application/json", "{\"status\":\"success\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"no_body\"}");
  }
}

void runServo(int dirn, int time = 4000) {
  if (dirn == 1) {
    servo.write(0);
  } else {
    servo.write(180);
  }
  delay(time);
  servo.write(90);
}

void allOff() {
  digitalWrite(s5, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  digitalWrite(s4, LOW);
  servo.write(90);
}

// Helper function to flash LED
void flashLED(int led) {
  Serial.println("flash led");
  if (led == 1) return;

  int p;
  switch (currentStage) {
    case 2:
      p = s2;
      break;
    case 3:
      p = s3;
      break;
    case 4:
      p = s4;
      break;
    case 5:
      p = s5;
      break;
    default:
      break;
  }

  for (int fadeValue = 0; fadeValue <= 255; fadeValue += 5) {
    analogWrite(p, fadeValue);
    delay(15);
  }

  for (int fadeValue = 255; fadeValue >= 0; fadeValue -= 5) {
    analogWrite(p, fadeValue);
    delay(15);
  }
}


void loop() {
  server.handleClient();
  delay(10);
  if (stageChanged) {
    allOff();
    stageChanged = false;
    if (currentStage == 0) {
      if (membraneFormed) {
        runServo(0);
      }
    }
    if (currentStage == 1) {
      runServo(1);
    }
    // else
    // {
    //   flashLED(currentStage);
    // }
  } else if (currentStage == 2 || currentStage == 3 || currentStage == 4 || currentStage == 5) {
    flashLED(currentStage);
  }
}

// void core0(void *pvParameters)
// {

//   for (;;)
//   {
//     if (stageChanged)
//     {
//       allOff();
//       stageChanged = false;
//       if (currentStage == 0)
//       {
//         if (membraneFormed)
//         {
//           runServo(0); // reset membrane
//         }
//       }
//       if (currentStage == 1)
//       {
//         runServo(1); // from membrane
//       }
//       else
//       {
//         flashLED(currentStage);
//       }
//     }
//   }
// }
