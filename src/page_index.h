#pragma once

static const char PAGE_INDEX[] PROGMEM = R"HTML(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<title>Zero Utility Vehicle</title>
<style>
  html,body { margin:0; height:100%; background:#111; color:#eee;
              font-family:sans-serif; display:flex; flex-direction:column;
              align-items:center; justify-content:center; overflow:hidden; }
  h1 { margin:0 0 4px; font-size:20px; letter-spacing:2px; }
  #status { margin-bottom:20px; font-size:13px; color:#888; }
  #status.live { color:#5f5; }
  #stick { position:relative; width:70vmin; height:70vmin; max-width:320px; max-height:320px;
           border-radius:50%; background:#1c1c1c; border:2px solid #333;
           touch-action:none; }
  #knob { position:absolute; width:35%; height:35%; border-radius:50%;
          background:#3a7; top:32.5%; left:32.5%; box-shadow:0 0 12px #3a7a; }
</style>
</head>
<body>
<h1>ZUV</h1>
<div id="status">connecting...</div>
<div id="stick"><div id="knob"></div></div>
<script>
const stick = document.getElementById('stick');
const knob = document.getElementById('knob');
const statusEl = document.getElementById('status');

let ws, dragging = false, t = 0, s = 0;

function connect() {
  ws = new WebSocket('ws://' + location.host + '/ws');
  ws.onopen = () => { statusEl.textContent = 'connected'; statusEl.className = 'live'; };
  ws.onclose = () => { statusEl.textContent = 'disconnected'; statusEl.className = '';
                        setTimeout(connect, 1000); };
  ws.onerror = () => ws.close();
}
connect();

setInterval(() => { if (ws.readyState === 1) ws.send(t + ',' + s); }, 50);

function setFromEvent(e) {
  const r = stick.getBoundingClientRect();
  const cx = r.left + r.width / 2, cy = r.top + r.height / 2;
  const radius = r.width / 2;
  let dx = (e.clientX - cx) / radius, dy = (e.clientY - cy) / radius;
  const mag = Math.hypot(dx, dy);
  if (mag > 1) { dx /= mag; dy /= mag; }
  t = Math.round(-dy * 100);
  s = Math.round(dx * 100);
  knob.style.left = (32.5 + dx * 32.5) + '%';
  knob.style.top  = (32.5 + dy * 32.5) + '%';
}

function reset() {
  dragging = false;
  t = 0; s = 0;
  knob.style.left = '32.5%';
  knob.style.top = '32.5%';
}

stick.addEventListener('pointerdown', e => { dragging = true; stick.setPointerCapture(e.pointerId); setFromEvent(e); });
stick.addEventListener('pointermove', e => { if (dragging) setFromEvent(e); });
stick.addEventListener('pointerup', reset);
stick.addEventListener('pointercancel', reset);
</script>
</body>
</html>
)HTML";
