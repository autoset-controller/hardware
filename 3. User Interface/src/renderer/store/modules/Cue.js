const state = {
  currCue: {},
};

const getters = {
  currCue: state => state.currCue,
  waypoints: state => state.currCue.waypoints,
  name: state => state.currCue.name
};

const mutations = {
  CHANGE_CUE(state, cue) {
    state.currCue = cue;
  },
  updateName(state, name) {
    state.currCue.name = name;
  },
  SET_START(state, pos) {
    state.currCue.start = pos;
  },
  ADD_WAYPOINT(state, pos) {
    console.log(state.currCue);
    state.currCue.waypoints.push(pos);
  }
};

export default {
  state,
  getters,
  mutations
};
