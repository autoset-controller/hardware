<template>
  <div>
    <input type="text" name="packet" v-model="packet">
    <button class="send" @click="sendData">Send</button>
    <button @click="setPos">Set Position</button>
    <button class="secondary" @click="emergencyStop">Cancel</button>
    <button @click="followPath">Circle Path</button>
    <button @click="draw">Draw Waypoints</button>
  </div>
</template>

<script>
import { mapGetters, mapMutations } from 'vuex';
import { checkSide, getRatio, getIntersect } from '../../pathFollowing';
export default {
  data() {
    return {
      packet: '1234|0|0|353|0|24|-24|5|5'
    };
  },
  methods: {
    sendData() {
      console.log('Sending data...');
      // Write data and confirm it was written
      // Message: key|platform|target|id|cmd|left|right|delay|checksum
      let message = '';
      message = Buffer.from(`${this.packet}\n`);
      // message = Buffer.from('1234|0|0|353|0|0|5|5\n');
      this.$port.write(message, () => {
        console.log('Write:\t\t Complete!');
        console.log('Last write:\t', message.toString());
      });
    },
    emergencyStop() {
      console.log('Emergency Stop');
      const message = '1234|E_STOP|\n';
      this.$port.write(message, () => {
        console.log('Stopped!');
      });
    },
    setPos() {
      console.log('Setting position');
      const message = '1234|0|1|353|0|0.0|0.0|0.0|0.0\n';
      this.$port.write(message, () => {
        console.log('Set position');
      });
    },
    followPath() {
      console.log('Path Following activated.');
      // CONSTANTS:
      const baseMotors = 2.5; // distance between motor wheels
      // const platformRadius = .15; // radius of encoder wheels
      const maxSpeed = 24; // one rotation per second
      const waypoints = [
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
      ];
      const cycles = 1;

      const speed = {
        left: 0,
        right: maxSpeed
      }; // left/right speeds in m/s
      const wpCount = waypoints.size;

      let wpCurrent = 0;
      let waypointPos = waypoints[wpCurrent];

      let completeCycles = 0;
      // MAIN LOOP
      const platformPos = {
        x: 1,
        y: 1,
        theta: 1
      };
      // Check if waypoint is passed, and update waypoint
      const ispassed = checkSide(platformPos, waypointPos);
      if (ispassed === 1) {
        wpCurrent++;
        if (wpCurrent > wpCount) {
          wpCurrent = 1;
          completeCycles++;
        }
        waypointPos = waypoints[wpCurrent];
      }
      // Find intersect and distances to intersect
      const { intersect, radii } = getIntersect(platformPos, waypointPos);
      // Determine appropriate ratio for current scenario
      const ratio = getRatio(
        speed, baseMotors,
        platformPos, waypointPos, intersect, radii
      );
      // Set speed of motors: In final version, send out packet
      speed.left = ratio.left * maxSpeed;
      speed.right = ratio.right * maxSpeed;
      console.log(`Writing ${ratio.left} and ${ratio.right}.`);
      this.$port.write(`1234|0|0|353|0|${speed.left}|${speed.right}|1|${speed.left + speed.right + 1}\n`, () => {
        console.log('Wrote');
        if (completeCycles < cycles) {
          this.followPath();
        }
      });
    },
    draw() {
      this.TOGGLE_DRAWING();
      console.log('Drawing waypoints is ', this.isDrawing);
    },
    ...mapMutations([
      'TOGGLE_DRAWING'
    ])
  },
  computed: {
    ...mapGetters({
      isDrawing: 'isDrawing'
    })
  }
};
</script>

<style scoped>
  input[type=text] {
    color: black;
    width: 300px;
  }
</style>
