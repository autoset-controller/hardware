var serialport = require('serialport');

console.log('starting serial communication...');

serialport.list(function (err, results) {
    if (err) {
        throw err;
    }

    for (var i = 0; i < results.length; i++) {
        var item = results[i];
        console.log(sf('{comName,-15} {pnpId,-20} {manufacturer}', item));
    }
});

  console.log('COMs listed.')

var port = new serialport('COM9', { baudRate: 9600 });

port.open(function (err) {
    if(err) {
        return console.log('Error opening port: ', err.message);
    }
});


var c;
while(true) {
    c = port.read(1);
    if(c != null) {
        console.log(c);
    }
}