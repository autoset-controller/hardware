const state = {
  points: []
};

const getters = {
  points: state => state.points,
};

const mutations = {
  ADD_POINTS(state, res) {
    state.points.push(res);
    console.log(state.points);
  }
};

export default {
  state,
  getters,
  mutations
};
