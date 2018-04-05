<template>
  <section class="cues">
    <h3>Cues</h3>
    <table class="table">
      <thead>
        <tr>
          <th>#</th>
          <th>
            <abbr title="Cue Name">Name</abbr>
          </th>
          <th>
            <abbr title="Describes which platforms the cue targets">Target</abbr>
          </th>
          <th>
            <abbr title="Duration of the cue">Duration</abbr>
          </th>
          <th>
            <abbr title="Overall speed of the cue">Speed</abbr>
          </th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="(cue, index) in cues" @click="changeCue(index)" :key='index'>
          <th v-bind:style="{ 'color': `#${cue.color}` }">{{ index + 1 }}</th>
          <td>{{ cue.name }}</td>
          <td>{{ cue.targets }}</td>
          <td>{{ cue.duration }}</td>
          <td>{{ cue.speed }}</td>
        </tr>
      </tbody>
    </table>
  </section>
</template>

<script>
// const store = require('vuex');
import { mapMutations } from 'vuex';
const cues = [{
  name: 'Start of Something New',
  targets: 'A',
  duration: 9.45,
  speed: 30,
  color: '39CCCC',
  waypoints: [
    {
      x: 1 / Math.sqrt(2),
      y: 1 / Math.sqrt(2),
      theta: 3 * Math.PI / 4
    }, {
      x: -1 / Math.sqrt(2),
      y: 1 / Math.sqrt(2),
      theta: 5 * Math.PI / 4
    }, {
      x: -1 / Math.sqrt(2),
      y: -1 / Math.sqrt(2),
      theta: 7 * Math.PI / 4
    }, {
      x: 1 / Math.sqrt(2),
      y: -1 / Math.sqrt(2),
      theta: Math.PI / 4
    }
  ],
  start: {
    x: 2,
    y: 0,
    theta: Math.PI
  }
},
{
  name: 'Get\'cha Head In the Game',
  targets: 'A',
  duration: 12.45,
  speed: 24,
  color: 'B10DC9',
  waypoints: [
    {
      x: Math.PI,
      y: 0,
      theta: Math.PI - 1
    }, {
      x: Math.PI / 2,
      y: 1,
      theta: Math.PI
    }, {
      x: 0,
      y: 0,
      theta: Math.PI + 1
    }, {
      x: -Math.PI / 2,
      y: -1,
      theta: Math.PI
    }, {
      x: -Math.PI,
      y: 0,
      theta: Math.PI - 1
    }
  ],
  start: {
    x: 4,
    y: -1,
    theta: Math.PI
  }
},
{
  name: 'What I\'ve Been Looking For',
  targets: 'A',
  duration: 3.4,
  speed: 60,
  color: 'FF851B',
  waypoints: [],
  start: {}
}
];
console.log();
export default {
  data() {
    return {
      cues,
    };
  },
  methods: {
    ...mapMutations([
      'CHANGE_CUE'
    ]),
    changeCue(index) {
      this.CHANGE_CUE(this.cues[index]);
    }
  }
};
</script>

<style scoped>
table {
  font-size: 14px;
}
</style>

