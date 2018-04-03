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
        <tr v-for="(cue, index) in cues" @click="configure(index)" :key='index'>
          <th>{{ index + 1 }}</th>
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
const cues = [{
  name: 'Starting Motion',
  targets: 'A',
  duration: 9.45,
  speed: 2
},
{
  name: 'All Around the World',
  targets: 'B',
  duration: 12.45,
  speed: 3,
},
];
console.log();
export default {
  data() {
    return {
      cues,
    };
  },
};
</script>

<style scoped>
table {
  font-size: 14px;
}
</style>

