#include <Arduino.h>

// Scaffolding for Web Services
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Allow OTA software updates
#include <AsyncElegantOTA.h>

// Use local Wi-Fi
#include <WiFi.h>

// Psuedo values for web page populating
int specialValue = 42;
std::string webPageDesc = ", and wonderfully written ";

// Try and get a static IP address (so we can find the device for web pages)
IPAddress local_IP(192, 168, 1, 103); // change 103 to be unique on your system
IPAddress gateway(192, 168, 1, 254);
IPAddress subnet(255, 255, 255, 0);

// To get the local time we must include DNS servers. Google's used here.
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// Forward declarations (Arduino IDE not required, only PlatformIO)
const char *wl_status_to_string(wl_status_t status);
void updateParm1(std::string *htmlOutput, std::string findPattern, std::string replaceWith);

// Standard web server, on port 80. Must be global. Obvs.
AsyncWebServer server(80);

// =======================================================================
// INFO: All Wi-Fi and server setup is done here, once only operation
// =======================================================================
void setup()
{
  // Assuming you want your ESP32 to act like any other client (STAtion) on your Wi-Fi
  WiFi.mode(WIFI_STA);

  // Store Wi-Fi configuration in EEPROM? Not a good idea as it will never forget these settings.
  WiFi.persistent(false);

  // Reconnect if connection is lost
  WiFi.setAutoReconnect(true);

  // Modem sleep when in WIFI_STA mode not a good idea as someone might want to talk to it
  WiFi.setSleep(false);

  // Let's do it
  WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
  wl_status_t reply = WiFi.begin("YourSSID", "YourPASSWORD");

  // Log the result
  log_w("Result: %s", wl_status_to_string(reply));

  // ------------------------------
  // Now register some server pages
  // ------------------------------

  // Just to be sure we have a 404 response on unknown requests
  server.onNotFound(
      [](AsyncWebServerRequest *request) {
        // Send back a plain text message (can be made html if required)
        request->send(404, "text/plain", "404 - Page Not Found, oops!");
      });

  // Send back a web page (landing page)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Tell the system that we will be sending back html (not just plain text)
    AsyncResponseStream *response = request->beginResponseStream("text/html");

    // Format the html as a RAW literal so we don't have to escape all the quotes (and more)
    std::string htmlOutput = R"(
      <html>
        <head>
          <title>Landing @parm1 Page</title>
          <style>
            body {
              background-color: forestgreen;
              font-family: Arial, Sans-Serif;
              color: yellow;
            }
            #myValue {
              width: 50px;
            }
          </style>
        </head>
        <body>
          <h1>My Landing Page: @parm1</h1>
          <p>
            This is a descriptive)";

    htmlOutput.append(webPageDesc);

    htmlOutput += R"( paragraph for your landing page 
            <input type="text" id="myValue" value="@parm1">
          </p>
        </body>
      </html>
    )";

    // Inject the values into the web page
    updateParm1(&htmlOutput, "@parm1", std::to_string(specialValue));

    // Output the stuffed string (yes, that's a term)
    response->printf(htmlOutput.c_str());

    // Send back all the text/html for a standard web page
    request->send(response);
  });

  // Starting Async OTA web server AFTER all the server.on requests registered
  AsyncElegantOTA.begin(&server);
  server.begin();
}

// =======================================================================
// INFO: Main Loop     Main Loop     Main Loop     Main Loop     Main Loop
// =======================================================================
void loop()
{
  // Nothing to do here (yet!)
  yield();
}

// Translates the Wi-Fi connect response to English
const char *wl_status_to_string(wl_status_t status)
{
  switch (status) {
    case WL_NO_SHIELD:
      return "WL_NO_SHIELD";
    case WL_IDLE_STATUS:
      return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL:
      return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED:
      return "WL_SCAN_COMPLETED";
    case WL_CONNECTED:
      return "WL_CONNECTED";
    case WL_CONNECT_FAILED:
      return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED:
      return "WL_DISCONNECTED";
    default:
      return "UNKNOWN";
  }
}

// Update one or more parameters in the web page
// FIXME: Steamline this code: remove the pointer, pass by reference, no local copy
void updateParm1(std::string *htmlString, std::string findPattern, std::string replaceWith)
{
  // Copy the string into something we can easily work with locally
  std::string tempString = (*htmlString);

  // Find the FIRST occurence of the placeholder @parm1
  size_t parm1Pos = tempString.find(findPattern);

  // Replace all the occurences (it might be in multiple places)
  while (parm1Pos != std::string::npos) {
    log_d("Found %s", findPattern.c_str());
    tempString.replace(parm1Pos, findPattern.length(), replaceWith);
    parm1Pos = tempString.find(findPattern);
  }

  // Update the html string passed to us originally
  *htmlString = tempString;
}
