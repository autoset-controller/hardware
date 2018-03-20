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
    console.log(key, platform, x, y, theta, checksum);
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


Vue.prototype.$port = port;

/* eslint-disable no-new */
new Vue({
  components: { App },
  router,
  store,
  template: '<App/>',
}).$mount('#app');
