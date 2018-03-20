/* eslint-disable */
const SerialPort = require('serialport');
let port;
let tmpData = '';
const regex = /\d*\|\d*\|\d*(\.\d*)?\|\d*(\.\d*)?\|\d*(\.\d*)?\|\d*(\.\d*)\n?/;

function getRandom(min, max) {
  return Math.floor(Math.random() * (max - min) + min);
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
    console.log(key, platform, x, y, theta, checksum);
  }
}
function init(comPort, baudRate) {
  // Create a port set to specified COM and baudRate
  port = new SerialPort(comPort, {
    baudRate
  });

  port.on('error', (err) => {
    console.error('There was an error opening the COM port. Please check your connection.\n', err);
  });

  port.on('open', () => {
    console.log('Port opened:\t', port.path);
  });

  // log received data
  // key|platform|x|y|theta|checksum
  port.on('data', (data) => {
    // console.log('Received:\t', data.toString());
    const packet = buildPacket(data);
    if (packet) {
      // console.log('Complete packet: \n', packet);
      getPacketData(packet);
      port.write('OK');
    }
  });
}