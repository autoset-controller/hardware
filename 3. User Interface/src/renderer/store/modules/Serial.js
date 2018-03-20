const SerialPort = require('serialport');

let port;

const state = {
  connected: false
};

const getters = {
  connected: state => state.connected
};

const mutations = {
  SET_CONNECTED_STATE(state, payload) {
    state.connected = payload.connectionState;
  }
};

const actions = {
  CONNECT_TO_PORT(state, payload) {
    port = new SerialPort(payload.comPort, {
      baudRate: payload.baudRate
    });

    port.on('error', (err) => {
      console.error('There was an error opening the COM port. Please check your connection.\n', err);
      state.commit('SET_CONNECTED_STATE', {
        connectionState: false
      });
    });

    port.on('open', () => {
      console.log('Port opened:\t', port.path);
      state.commit('SET_CONNECTED_STATE', {
        connectionState: true
      });
    });
  },
  MOVE_PLATFORM(state, payload) {
    const message = Buffer.from('1234|0|0|353|-512|512|0\n');
    console.log('Moving platform. Payload is ', payload);
    port.write(message, () => {
      console.log('Write:\t\t Complete!');
      console.log('Last write:\t', message.toString());
    });
  }
};

export default {
  state,
  getters,
  mutations,
  actions
};
