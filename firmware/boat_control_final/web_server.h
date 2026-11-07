// =====================================================================
//  web_server.h — Wi-Fi et API HTTP du bateau
// =====================================================================
#pragma once

void initWiFi();          // Access Point ou Station (avec repli en AP)
void initWebServer();     // démarre le serveur HTTP sur le port 80
void handleWebClients();  // à appeler dans loop()
