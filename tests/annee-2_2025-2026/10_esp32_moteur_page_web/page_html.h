// =====================================================================
// FICHIER GÉNÉRÉ AUTOMATIQUEMENT — NE PAS MODIFIER À LA MAIN
// Source : web/index.html (+ CSS/JS)  ·  Générateur : tools/build_web.py
// =====================================================================
#pragma once
#include <Arduino.h>

const char pageHTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Controle Moteur DC</title>
<style>
body {
  font-family: Arial;
  background: #111;
  color: white;
  text-align: center;
}
button {
  padding: 15px 25px;
  margin: 10px;
  font-size: 18px;
  border: none;
  border-radius: 10px;
  cursor: pointer;
}
#avant { background: #00c853; }
#arriere { background: #2979ff; }
#stop { background: #d50000; }
input[type=range] {
  width: 80%;
}
</style>
</head>
<body>
<h1>ESP32 - Moteur DC</h1>
<button id="avant" onclick="cmd('avant')">AVANT</button>
<button id="arriere" onclick="cmd('arriere')">ARRIÈRE</button>
<button id="stop" onclick="cmd('stop')">STOP</button>
<h2>Vitesse: <span id="val">0</span></h2>
<input type="range" min="0" max="255" value="0" oninput="vitesse(this.value)">
<script>
function cmd(c) {
  fetch('/' + c);
}
function vitesse(v) {
  document.getElementById("val").innerText = v;
  fetch('/vitesse?val=' + v);
}
</script>
</body>
</html>
)HTML";
