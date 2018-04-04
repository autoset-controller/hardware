import 'bulma/css/bulma.css';
import Vue from 'vue';
import SerialPort from 'serialport';

import App from './App';
import router from './router';
import store from './store';

if (!process.env.IS_WEB) Vue.use(require('vue-electron'));
Vue.config.productionTip = false;

// SerialPort configuration
console.clear();
const port = new SerialPort('COM5', {
  baudRate: 57600
});
port.on('error', (err) => {
  console.error('There was an error opening the COM port. Please check your connection.\n', err);
});

port.on('open', () => {
  console.log('Port opened:\t', port.path);
});

let tmpData = '';
const regex = /\d*\|\d*\|-?\d*(\.\d*)?\|-?\d*(\.\d*)?\|-?\d*(\.\d*)?\|-?\d*(\.\d*)\n?/;
const goodRegex = /good:\d*\n?/;
let portRes = 'foo';

function writeToController(data) {
  const message = `1234|0|0|353|0|${data.x}|${data.y}|1|${data.x + data.y + 1}\n`;
  port.write(message, () => {
    console.log('Sent ', data);
  });
}

function buildPacket(data) {
  tmpData += data;
  const packets = tmpData.split('\n');
  const testPacket = packets[packets.length - 1];
  if (regex.test(testPacket)) {
    tmpData = '';
    return testPacket;
  } else if (packets.length > 1) {
    tmpData = testPacket;
    return packets[packets.length - 2];
  }
  return null;
}

function getPacketData(packet) {
  if (regex.test(packet)) {
    const [key, platform, x, y, theta, checksum] = packet.split('|');
    portRes = {
      key, platform, x, y, theta, checksum
    };
    store.commit('SET_POSITION', portRes);
    // console.log(`Positions:
    // x: ${portRes.x}
    // y: ${portRes.y},
    // θ: ${portRes.theta}`);
  } else if (goodRegex.test(packet)) {
    // console.log('It\'s all good!');
  }
}

// log received data
// key|platform|x|y|theta|checksum
port.on('data', (data) => {
  const packet = buildPacket(data);
  if (packet) {
    getPacketData(packet);
  }
});
const MAX_SPEED = 60;
const motorPackets = { x: 0, y: 0 };
function gameLoop() {
  const gamepads = navigator.getGamepads();
  const gamepad = gamepads[0];
  if (gamepad) {
    const leftSpeed = gamepad.axes[1];
    const rightSpeed = gamepad.axes[3];
    if ((leftSpeed > 0.1 || leftSpeed < -0.1) ||
    (rightSpeed > 0.1 || rightSpeed < -0.1)) {
      const xValue = -Math.round(leftSpeed * MAX_SPEED);
      const yValue = -Math.round(rightSpeed * MAX_SPEED);
      if (Math.abs(xValue - motorPackets.x) >= 1 || Math.abs(yValue - motorPackets.y) >= 1) {
        motorPackets.x = xValue;
        motorPackets.y = yValue;
        writeToController({ x: motorPackets.x, y: motorPackets.y });
      }
    } else if (gamepad.buttons[1].pressed) {
      port.write('1234|E_STOP|\n', () => {
        console.log('Emergency Stop!');
      });
    } else if (motorPackets.x !== 0 || motorPackets.y !== 0) {
      motorPackets.x = 0;
      motorPackets.y = 0;
      writeToController({ x: motorPackets.x, y: motorPackets.y });
    }
  }

  window.requestAnimationFrame(gameLoop);
}
gameLoop();


Vue.prototype.$port = port;
Vue.prototype.$portRes = portRes;

/* eslint-disable no-new */
new Vue({
  components: { App },
  router,
  store,
  template: '<App/>',
}).$mount('#app');
