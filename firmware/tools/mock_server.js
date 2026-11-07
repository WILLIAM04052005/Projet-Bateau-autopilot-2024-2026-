#!/usr/bin/env node
/**
 * mock_server.js — Simulateur de l'API HTTP de l'ESP32 (sans matériel)
 *
 * Sert l'interface de firmware/web/ et répond aux mêmes routes que le
 * firmware (/mag, /speed_data, /motor, /dir, /speed, /servo, /autopilot,
 * /calibrate) avec un petit modèle physique : le bateau accélère selon la
 * puissance et tourne selon l'angle du safran. Permet de développer et
 * tester l'interface dans VS Code / le navigateur sans flasher la carte.
 *
 * Utilisation (Node.js ≥ 16, aucune dépendance) :
 *     node firmware/tools/mock_server.js      →  http://localhost:8080
 */
const http = require("http");
const fs = require("fs");
const path = require("path");

const PORT = process.env.PORT || 8080;
const WEB_DIR = path.join(__dirname, "..", "web");
const MIME = { ".html": "text/html; charset=utf-8", ".css": "text/css", ".js": "text/javascript" };

// ------------------------- État simulé du bateau -------------------------
const boat = {
  motorOn: false, direction: "stop", speedVal: 0, servo: 90,
  autopilot: false, targetHeading: 0, heading: 180, headingOffset: 0,
  rpm: 0, speed: 0, current: 0,
};

const clamp = (v, lo, hi) => Math.max(lo, Math.min(hi, v));
const wrap180 = (e) => ((e + 540) % 360) - 180;

// Boucle de simulation à 20 Hz (même cadence que le PID du firmware)
setInterval(() => {
  const dt = 0.05;
  const sign = boat.direction === "fwd" ? 1 : boat.direction === "rev" ? -1 : 0;
  const duty = boat.motorOn ? (sign * boat.speedVal) / 255 : 0;

  boat.rpm += (duty * 9500 - boat.rpm) * 0.1;           // moteur 775 en charge ≈ 9500 tr/min
  boat.speed = Math.abs(boat.rpm) * 0.1 / 60;           // même calibration que le firmware (fictive)
  boat.current = Math.abs(duty) * 8.3 + (boat.motorOn ? 0.2 : 0); // ≈ 100 W / 12 V à pleine puissance

  if (boat.autopilot) {                                  // PID simplifié (P) pour la démo
    const err = wrap180(boat.targetHeading - boat.heading);
    boat.servo = Math.abs(err) < 8 ? 90 : clamp(Math.round(90 + 0.35 * err), 65, 115);
  }
  const turnRate = ((boat.servo - 90) / 60) * 40 * Math.abs(duty); // °/s
  boat.heading = (boat.heading + turnRate * dt + 360) % 360;
}, 50);

// --------------------------------- API -----------------------------------
const routes = {
  "/mag": () => {
    const h = (boat.heading + boat.headingOffset + 360) % 360;
    const rad = (h * Math.PI) / 180;
    return { ok: true, x: Math.round(300 * Math.cos(rad)), y: Math.round(300 * Math.sin(rad)), z: -120, heading: +h.toFixed(1) };
  },
  "/speed_data": () => ({ rpm: Math.round(Math.abs(boat.rpm)), speed: +boat.speed.toFixed(1), current: +boat.current.toFixed(2) }),
  "/motor": (q) => { boat.motorOn = q.get("on") === "1"; },
  "/dir": (q) => { boat.direction = q.get("d"); },
  "/speed": (q) => { boat.speedVal = clamp(parseInt(q.get("val"), 10) || 0, 0, 255); },
  "/servo": (q) => { if (!boat.autopilot) boat.servo = clamp(parseInt(q.get("angle"), 10), 30, 150); },
  "/autopilot": (q) => {
    if (q.has("active")) boat.autopilot = q.get("active") === "1";
    if (q.has("heading")) boat.targetHeading = parseInt(q.get("heading"), 10) || 0;
  },
  "/calibrate": () => { boat.headingOffset = -boat.heading; },
};

http.createServer((req, res) => {
  const url = new URL(req.url, `http://${req.headers.host}`);
  const route = routes[url.pathname];

  if (route) {
    const data = route(url.searchParams);
    if (data) {
      res.writeHead(200, { "Content-Type": "application/json" });
      return res.end(JSON.stringify(data));
    }
    res.writeHead(200, { "Content-Type": "text/plain" });
    return res.end("OK");
  }

  const file = url.pathname === "/" ? "index.html" : url.pathname.slice(1);
  const full = path.join(WEB_DIR, path.normalize(file));
  if (!full.startsWith(WEB_DIR)) { res.writeHead(403); return res.end(); }
  fs.readFile(full, (err, buf) => {
    if (err) { res.writeHead(404); return res.end("404"); }
    res.writeHead(200, { "Content-Type": MIME[path.extname(full)] || "application/octet-stream" });
    res.end(buf);
  });
}).listen(PORT, () => console.log(`Simulateur Boat Control : http://localhost:${PORT}`));
