<template>
  <div>
    <input type="text" name="packet" v-model="packet">
    <button class="send" @click="sendData">Send</button>
    <button @click="setPos">Set Position</button>
    <button class="secondary" @click="emergencyStop">Cancel</button>
    <button @click="followPath(0, 0)">Circle Path</button>
    <button @click="draw">Draw Waypoints</button>
    <button @click="forward">Forward</button>
    <button @click="reverse">Reverse</button>
    <button @click="rotate">Rotate</button>
    <button @click="pause">Pause</button>
  </div>
</template>

<script>
/* eslint-disable */
import { mapGetters, mapMutations } from 'vuex';
import { checkSide, getRatio, getIntersect } from '../../pathFollowing';
export default {
  data() {
    return {
      packet: '1234|0|0|353|0|24|-24|5|5',
      canRun: true
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
      this.canRun = false;
      // console.log('Emergency Stop');
      const message = '1234|E_STOP|\n';
      this.$port.write(message, () => {
        console.log('Stopped!');
      });
    },
    setPos() {
      console.log('Setting position');
      const message = '1234|0|1|353|0|2.0|0.0|-3.14|-1.14\n';
      this.$port.write(message, () => {
        console.log('Set position');
      });
    },
    forward() {
      this.$port.write('1234|0|0|353|0|24|24|5|53\n', () => {
        console.log('Set position');
      });
    },
    reverse() {
      this.$port.write('1234|0|0|353|0|-24|-24|5|-43\n', () => {
        console.log('Set position');
      });
    },
    rotate() {
      this.$port.write('1234|0|0|353|0|24|-24|5|5\n', () => {
        console.log('Set position');
      });
    },
    pause() {
      this.$port.write('1234|0|0|353|0|0|0|2|2\n', () => {
        console.log('Set position');
      });
    },
    followPath(wpCurrent, completeCycles) {
      console.log('Path Following activated.');
      // CONSTANTS:
      const baseMotors = 0.77469997968; // distance between motor wheels
      const minSpeed = 11;
      const maxSpeed = 24; // one rotation per second
      const cycles = 1;
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
      const speed = { left: 0, right: maxSpeed }; // left/right speeds in m/s
      const wpCount = waypoints.length;
      let waypointPos = waypoints[wpCurrent];

      // MAIN LOOP
      const platformPos = { x: this.position.x, y: this.position.y, theta: this.rotation };
      // Check if waypoint is passed, and update waypoint
      const ispassed = checkSide(platformPos, waypointPos);
      if (ispassed === 1) {
        wpCurrent++;
        if (wpCurrent >= wpCount) {
          console.log('Finished one cycle.');
          wpCurrent = 0;
          completeCycles++;
        }
        waypointPos = waypoints[wpCurrent];
      }
      // Determine appropriate ratio for current scenario
      const ratio = getRatio(
        speed, baseMotors, platformPos, waypointPos
      );
      // Set speed of motors: In final version, send out packet
      speed.left = Math.round(ratio[0] * (maxSpeed - minSpeed) + Math.sign(ratio[0]) * minSpeed);
      speed.right = Math.round(ratio[1] * (maxSpeed - minSpeed) + Math.sign(ratio[1]) * minSpeed);
      const packet = `1234|0|0|353|0|${speed.left}|${speed.right}|0|${speed.left + speed.right}\n`;
      console.log(`Index is: ${wpCurrent}, cycles run is: ${completeCycles}, ratio is ${ratio[0]}, ${ratio[1]} and speed is ${speed.left}, ${speed.right}`);
      // this.$port.write(packet, () => {
      //   console.log('Wrote');
        setTimeout(() => {
          if (completeCycles < cycles && this.canRun) {
            this.followPath(wpCurrent, completeCycles);
          }
          else {
            console.log('We\'re done here.');
            this.pause();
            this.canRun = false;
          }
        }, 100);
      // });
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
      isDrawing: 'isDrawing',
      position: 'position',
      rotation: 'rotation',
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
