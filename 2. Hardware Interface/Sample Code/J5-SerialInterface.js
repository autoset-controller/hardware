var serialport = require('serialport');
var port = new serialport('COM9');

// Switches the port into "flowing mode"
port.on('data', function (data) {
    console.log('Data:', data);
});