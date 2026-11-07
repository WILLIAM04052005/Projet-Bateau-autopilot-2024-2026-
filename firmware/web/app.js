// Boat Control — logique de l'interface web (pilotage manuel, autopilote, télémétrie)
// Communique avec le serveur HTTP de l'ESP32 via fetch() : voir firmware/README.md (API).

var mot = false;
var curDir = 'stop';
var curHead = 0;
var curServoAngle = 90;
var curSpd = 0;
var currentMode = 'manual';
var autopilotActive = false;
var targetHeading = 0;
var realCurrentHeading = 0;
var magOK = false;

function headingToServoAngle(heading) {
  var servoAngle;
  
  if (heading >= 270) {
    servoAngle = heading - 270;
  } else if (heading <= 90) {
    servoAngle = heading + 90;
  } else {
    servoAngle = 90;
  }
  
  return Math.max(0, Math.min(180, Math.round(servoAngle)));
}

function switchMode(mode) {
  currentMode = mode;
  
  document.getElementById('pageManual').className = mode === 'manual' ? 'page active' : 'page';
  document.getElementById('pageAutopilot').className = mode === 'autopilot' ? 'page active' : 'page';
  document.getElementById('btnManual').className = mode === 'manual' ? 'mode-btn active' : 'mode-btn';
  document.getElementById('btnAutopilot').className = mode === 'autopilot' ? 'mode-btn active' : 'mode-btn';
}

function calibrateCompass() {
  if (confirm('Le bateau est-il pointé vers le Nord géographique ?')) {
    fetch('/calibrate')
      .then(response => response.text())
      .then(data => {
        alert('Calibration effectuée ! Le cap actuel est défini comme Nord (0°)');
      })
      .catch(err => {
        alert('Erreur de calibration');
        console.error(err);
      });
  }
}

function toggleAutopilot() {
  if (!magOK) {
    alert('⚠️ Magnétomètre non disponible! Vérifiez les connexions I2C (SDA=22, SCL=21)');
    return;
  }
  
  autopilotActive = !autopilotActive;
  var btn = document.getElementById('autopilotToggle');
  var statusText = document.getElementById('autopilotStatusText');
  
  if (autopilotActive) {
    btn.innerText = 'Désactiver Auto-Pilote';
    btn.className = 'autopilot-btn active';
    statusText.innerText = 'Pilote Automatique Actif';
    statusText.style.color = '#22c55e';
    
    if (!mot) {
      mot = true;
      document.getElementById('statusDot').className = 'status-dot on';
      fetch('/motor?on=1');
    }
    
    fetch('/autopilot?active=1&heading=' + targetHeading);
  } else {
    btn.innerText = 'Activer Auto-Pilote';
    btn.className = 'autopilot-btn';
    statusText.innerText = 'Pilote Automatique Désactivé';
    statusText.style.color = '#60a5fa';
    
    fetch('/autopilot?active=0');
  }
  
  updateAutopilotVisualization();
}

function setTargetHeading(heading) {
  targetHeading = (heading + 360) % 360;
  
  document.getElementById('targetHeadingDisplay').innerText = targetHeading + '°';
  document.getElementById('autopilotHeading').innerText = targetHeading + '°';
  
  updateAutopilotVisualization();
  
  if (autopilotActive) {
    fetch('/autopilot?active=1&heading=' + targetHeading);
  }
}

function adjustTargetHeading(delta) {
  targetHeading = (targetHeading + delta + 360) % 360;
  setTargetHeading(targetHeading);
}

function updateAutopilotVisualization() {
  var targetAngle = targetHeading;
  var targetRad = (targetAngle - 90) * Math.PI / 180;
  var x2 = 150 + Math.cos(targetRad) * 120;
  var y2 = 150 + Math.sin(targetRad) * 120;
  
  document.getElementById('trajectoryLine').setAttribute('x2', x2);
  document.getElementById('trajectoryLine').setAttribute('y2', y2);
  
  var currentAngle = realCurrentHeading;
  document.getElementById('boatIcon').setAttribute('transform', 
    'translate(150, 150) rotate(' + currentAngle + ')');
  
  var color = autopilotActive ? '#22c55e' : '#60a5fa';
  document.getElementById('boatDirection').setAttribute('fill', color);
  document.getElementById('trajectoryLine').setAttribute('stroke', color);
  
  if (autopilotActive) {
    var error = targetHeading - realCurrentHeading;
    while (error > 180) error -= 360;
    while (error < -180) error += 360;
    document.getElementById('autopilotError').innerText = Math.round(error) + '°';
  }
}

function toggleMotor() {
  mot = !mot;
  var b = document.getElementById('motorBtn');
  b.className = 'motor-btn ' + (mot ? 'on' : 'off');
  b.innerText = mot ? 'ON' : 'OFF';
  document.getElementById('statusDot').className = 'status-dot' + (mot ? ' on' : '');
  document.getElementById('gFwd').disabled = !mot;
  document.getElementById('gN').disabled = !mot;
  document.getElementById('gRev').disabled = !mot;
  fetch('/motor?on=' + (mot ? '1' : '0'));
  if (!mot) { setDir('stop'); setSpeed(0); }
}

function setDir(d) {
  curDir = d;
  fetch('/dir?d=' + d);
  document.getElementById('gFwd').className = 'gear-btn' + (d=='fwd'?' active':'');
  document.getElementById('gN').className = 'gear-btn' + (d=='stop'?' active':'');
  document.getElementById('gRev').className = 'gear-btn' + (d=='rev'?' active':'');
}

var powerGauge = document.getElementById('powerGauge');
var isPowerDragging = false;

function updatePowerFromEvent(e) {
  var rect = powerGauge.getBoundingClientRect();
  var clientY;
  if (e.type.indexOf('touch') >= 0) {
    clientY = e.touches[0].clientY;
  } else {
    clientY = e.clientY;
  }
  
  var y = clientY - rect.top;
  var pct = 1 - (y / rect.height);
  pct = Math.max(0, Math.min(1, pct));
  var val = Math.round(pct * 255);
  setSpeed(val);
}

powerGauge.addEventListener('mousedown', function(e) {
  isPowerDragging = true;
  updatePowerFromEvent(e);
});

powerGauge.addEventListener('touchstart', function(e) {
  isPowerDragging = true;
  updatePowerFromEvent(e);
});

document.addEventListener('mousemove', function(e) {
  if (isPowerDragging) {
    e.preventDefault();
    updatePowerFromEvent(e);
  }
});

document.addEventListener('touchmove', function(e) {
  if (isPowerDragging) {
    e.preventDefault();
    updatePowerFromEvent(e);
  }
}, {passive: false});

document.addEventListener('mouseup', function() {
  isPowerDragging = false;
});

document.addEventListener('touchend', function() {
  isPowerDragging = false;
});

var st = null;
function setSpeed(v) {
  curSpd = parseInt(v);
  document.getElementById('pwrVal').innerText = v;
  var pct = Math.round(v / 255 * 100);
  document.getElementById('powerFill').style.height = pct + '%';
  
  clearTimeout(st);
  st = setTimeout(function(){ fetch('/speed?val=' + v); }, 80);
}

var joystickWrap = document.getElementById('joystickWrap');
var joystickKnob = document.getElementById('joystickKnob');
var isDragging = false;
var joystickCenterX = 0;
var joystickCenterY = 0;
var maxRadius = 60;

function updateJoystickCenter() {
  var rect = joystickWrap.getBoundingClientRect();
  joystickCenterX = rect.left + rect.width / 2;
  joystickCenterY = rect.top + rect.height / 2;
}

var lastServoSend = 0;
var servoDelay = 40;

function sendServoCommand(angle){
  var now = Date.now();
  
  if(now - lastServoSend > servoDelay){
    fetch('/servo?angle=' + angle);
    lastServoSend = now;
  }
}

function handleJoystickStart(e) {
  isDragging = true;
  updateJoystickCenter();
  handleJoystickMove(e);
}

function handleJoystickMove(e) {
  if (!isDragging) return;
  e.preventDefault();
  
  var clientX, clientY;
  if (e.type.indexOf('touch') >= 0) {
    clientX = e.touches[0].clientX;
    clientY = e.touches[0].clientY;
  } else {
    clientX = e.clientX;
    clientY = e.clientY;
  }
  
  var dx = clientX - joystickCenterX;
  var dy = clientY - joystickCenterY;
  var distance = Math.sqrt(dx*dx + dy*dy);
  
  if (distance > maxRadius) {
    var angle = Math.atan2(dy, dx);
    dx = Math.cos(angle) * maxRadius;
    dy = Math.sin(angle) * maxRadius;
  }
  
  var angleRad = Math.atan2(dx, -dy);
  var angleDeg = (angleRad * 180 / Math.PI + 360) % 360;
  
  if (angleDeg > 90 && angleDeg < 270) {
    angleDeg = (angleDeg < 180) ? 90 : 270;
    var limitedAngleRad = (angleDeg - 90) * Math.PI / 180;
    dx = Math.cos(limitedAngleRad) * Math.min(distance, maxRadius);
    dy = Math.sin(limitedAngleRad) * Math.min(distance, maxRadius);
  }
  
  joystickKnob.style.transform = 'translate(calc(-50% + ' + dx + 'px), calc(-50% + ' + dy + 'px))';
  
  curHead = Math.round(angleDeg);
  curServoAngle = headingToServoAngle(curHead);
  updateCompass();
  
  if(!autopilotActive){
    sendServoCommand(curServoAngle);
  }
}

function handleJoystickEnd(e) {
  isDragging = false;
  joystickKnob.style.transform = 'translate(-50%, -50%)';
}

joystickKnob.addEventListener('mousedown', handleJoystickStart);
joystickKnob.addEventListener('touchstart', handleJoystickStart);
document.addEventListener('mousemove', handleJoystickMove);
document.addEventListener('touchmove', handleJoystickMove, {passive: false});
document.addEventListener('mouseup', handleJoystickEnd);
document.addEventListener('touchend', handleJoystickEnd);

function updateCompass(){
  document.getElementById("headVal").innerText = curHead + "°";
  
  var dir = "N";
  if(curHead >= 45 && curHead < 135) dir = "E";
  else if(curHead >= 135 && curHead < 225) dir = "S";
  else if(curHead >= 225 && curHead < 315) dir = "W";
  
  document.getElementById("headDir").innerText = dir;
  document.getElementById("servoAngle").innerText = "Servo: " + curServoAngle + "°";
}

function updateMagnetometer() {
  fetch('/mag')
    .then(response => response.json())
    .then(data => {
      magOK = data.ok;
      
      // Mettre à jour le statut du magnétomètre
      document.getElementById('magStatus').className = magOK ? 'mag-status ok' : 'mag-status';
      
      if (magOK) {
        document.getElementById("magX").innerText = data.x;
        document.getElementById("magY").innerText = data.y;

        realCurrentHeading = parseFloat(data.heading);
        
        document.getElementById("currentHeading").innerText = Math.round(realCurrentHeading);
        
        if (currentMode === 'autopilot') {
          document.getElementById("currentHeadingAuto").innerText = Math.round(realCurrentHeading);
          document.getElementById("autopilotCurrentHeading").innerText = Math.round(realCurrentHeading);
          document.getElementById("magXAuto").innerText = data.x;
          document.getElementById("magYAuto").innerText = data.y;
          updateAutopilotVisualization();
        }
      } else {
        document.getElementById("currentHeading").innerText = "ERR";
        if (currentMode === 'autopilot') {
          document.getElementById("currentHeadingAuto").innerText = "ERR";
        }
      }
    })
    .catch(err => {
      console.log('Mag error:', err);
      magOK = false;
      document.getElementById('magStatus').className = 'mag-status';
    });
}

function updateSpeedData() {
  fetch('/speed_data')
    .then(response => response.json())
    .then(data => {
      var speed = parseFloat(data.speed).toFixed(1);
      var rpm = parseInt(data.rpm);
      var current = parseFloat(data.current).toFixed(2);
      
      document.getElementById("speedVal").innerText = speed;
      document.getElementById("rpmVal").innerText = rpm;
      document.getElementById("currentVal").innerText = current;
      
      if (currentMode === 'autopilot') {
        document.getElementById("speedValAuto").innerText = speed;
        document.getElementById("rpmValAuto").innerText = rpm;
        document.getElementById("currentValAuto").innerText = current;
      }
      
      // Mise à jour de l'info header
      var headingText = magOK ? Math.round(realCurrentHeading) + "°" : "--";
      document.getElementById("hInfo").innerText = "Cap: " + headingText + " | Vitesse: " + speed + " km/h";
    })
    .catch(err => {
      console.log('Speed data error:', err);
    });
}

updateJoystickCenter();
updateCompass();
window.addEventListener('resize', updateJoystickCenter);
window.addEventListener('orientationchange', function() {
  setTimeout(updateJoystickCenter, 100);
});

setInterval(updateMagnetometer, 200);
setInterval(updateSpeedData, 500);
