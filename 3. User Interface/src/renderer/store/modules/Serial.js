const state = {
  position: {
    x: 0,
    y: 0
  },
  rotation: 0
};

const getters = {
  position: state => state.position,
  rotation: state => state.rotation
};

const mutations = {
  SET_POSITION(state, res) {
    state.position.x = res.x;
    state.position.y = res.y;
    state.rotation = res.theta;
  }
};

export default {
  state,
  getters,
  mutations
};
