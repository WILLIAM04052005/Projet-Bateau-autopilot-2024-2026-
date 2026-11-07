// Envoi des commandes moteur à l'ESP32 (fetch HTTP)

function cmd(c) {
  fetch('/' + c);
}
function vitesse(v) {
  document.getElementById("val").innerText = v;
  fetch('/vitesse?val=' + v);
}
