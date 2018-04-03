const state = {
  position: {
    x: 0,
    y: 0
  },
  rotation: 0,
  isDrawing: false
};

const getters = {
  position: state => state.position,
  rotation: state => state.rotation,
  isDrawing: state => state.isDrawing
};

const mutations = {
  SET_POSITION(state, res) {
    state.position.x = res.x;
    state.position.y = res.y;
    state.rotation = res.theta;
  },
  TOGGLE_DRAWING(state) {
    state.isDrawing = !state.isDrawing;
  }
};

export default {
  state,
  getters,
  mutations
};
