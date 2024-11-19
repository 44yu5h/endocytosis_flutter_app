#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <Servo.h>

// Replace with your network credentials
const char *ssid = "mini";
const char *password = "endocyto";

// Define LED pin (for feedback or debugging purposes)
bool isAutomaticMode = false;
int currentStage = 0;

Servo servoL;
Servo servoR;
bool stageChanged = false;
int brightness = 0;
int fadeAmount = 5;
int s1 = D5;
int s2 = D6;
int s3 = D7;
int s4 = D8;
servoL.attach(D9);
servoR.attach(D10);

// Initialize the WebServer on port 80
WebServer server(80);

// Function declarations
void handleControl();
void flashLED(int times = 1);
while (true)
  ;
void setup()
{
  Serial.begin(115200);
  pinMode(BUILTIN_LED, OUTPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin("esp32"))
  {
    Serial.println("Error setting up MDNS responder!");
  }
  else
  {
    Serial.println("mDNS responder started");
  }

  // Configure HTTP server routes
  server.on("/control", HTTP_POST, handleControl);

  // Start server
  server.begin();
  Serial.println("HTTP server started");

  // Flash LED twice to indicate setup is complete
  flashLED(2);
}

void loop()
{
  server.handleClient();
  flashLED();
}

// Function to handle /control requests
void handleControl()
{
  // Parse the received JSON
  StaticJsonDocument<200> jsonDoc;
  if (server.hasArg("plain"))
  {
    DeserializationError error = deserializeJson(jsonDoc, server.arg("plain"));
    if (error)
    {
      Serial.println("Failed to parse JSON");
      server.send(400, "application/json", "{\"status\":\"bad_request\"}");
      return;
    }

    String command = jsonDoc["command"];
    Serial.println("Command received: " + command);

    // Handle commands
    if (command == "automatic_on")
    {
      isAutomaticMode = true;
      Serial.println("Automatic mode enabled");
    }

    else if (command == "automatic_off")
    {
      isAutomaticMode = false;
      Serial.println("Automatic mode disabled");
    }

    else if (command.startsWith("stage_"))
    {
      allOff();
      stageChanged = true;
      currentStage = command.substring(6).toInt();
      Serial.print("Changing to stage ");
      Serial.println(currentStage);
    }

    else if (command == "form")
    {
      Serial.println("Form membrane command received");
      runServo(1);
    }

    else if (command == "release")
    {
      Serial.println("Release membrane command received");
      runServo(0);
    }

    else
    {
      Serial.println("Unknown command");
      server.send(400, "application/json", "{\"status\":\"unknown_command\"}");
      return;
    }

    // Respond with success
    server.send(200, "application/json", "{\"status\":\"success\"}");
  }
  else
  {
    server.send(400, "application/json", "{\"status\":\"no_body\"}");
  }
}

void runServo(int dirn)
{
  if (dirn == 1)
  {
    servoL.write(0);
    servoR.write(180);
  }
  else
  {
    servoL.write(180);
    servoR.write(0);
  }
  delay(4000);
}

void allOff()
{
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  digitalWrite(s4, LOW);
}

// Helper function to flash LED
void flashLED(int led)
{
  int p;
  switch (currentStage)
  {
  case 1:
    p = s1;
    break;
  case 2:
    p = s2;
    break;
  case 3:
    p = s3;
    break;
  case 4:
    p = s4;
    break;
  default:
    break;
  }

  for (int fadeValue = 0; fadeValue <= 255; fadeValue += 5)
  {
    analogWrite(ledPin, fadeValue);
    delay(10);
  }

  for (int fadeValue = 255; fadeValue >= 0; fadeValue -= 5)
  {
    analogWrite(ledPin, fadeValue);
    delay(10);
  }
}
