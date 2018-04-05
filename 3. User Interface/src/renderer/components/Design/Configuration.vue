<template>
  <section class="configuration">
      <h3>Configuration</h3>

      <div class="form-row">
        <div class="formElement">
          <label for="name">Cue Name</label>
          <input type="text" name="name" v-model="active.name" @input="updateName">
        </div>
  
        <div class="formElement">
          <label for="duration">Duration</label>
          <input type="text" name="duration" v-model="active.duration">
        </div>
  
        <div class="formElement">
          <label for="speed">Speed</label>
          <input type="text" name="speed" v-model="active.speed">
        </div>
  
        <div class="formElement">
          <label for="color">Color</label>
          <input type="color" name="color" v-bind:value="'#'+active.color">
        </div>
      </div>
      <button @click="record"><circle-icon></circle-icon></button>
      <button @click="play(0, 0, true)"><play-icon></play-icon></button>
      <button @click="goToStart(-1)"><airplay-icon></airplay-icon></button>
      <button @click="step"><corner-down-right-icon></corner-down-right-icon></button>
      <button @click="eStop"><x-square-icon></x-square-icon></button>
    </section>
</template>

<script>
import { mapGetters, mapState, mapMutations } from 'vuex';
import { CircleIcon, PlayIcon, XSquareIcon, AirplayIcon, CornerDownRightIcon } from 'vue-feather-icons';
import { checkSide, getRatio } from '../../pathFollowing';
function mod(n, m) {
  return ((n % m) + m) % m;
}
export default {
  data() {
    return {
      canRun: true,
      isRecording: false
    };
  },
  methods: {
    updateName(e) {
      this.$store.commit('updateName', e.target.value);
    },
    record() {
      if (!this.active.name) {
        console.log('Please choose a cue');
      } else {
        console.log('Recording');
        this.isRecording = !this.isRecording;
        this.SET_START({
          x: this.position.x,
          y: this.position.y,
          theta: this.rotation
        });
      }
    },
    eStop() {
      this.canRun = false;
      for (let i = 0; i < 10; i++)
        this.$port.write('1234|E_STOP|\n');
    },
    play(wpCurrent, completeCycles, start) {
      if (!this.active.waypoints) {
        console.log('No waypoints for this cue.');
      } else if (start) {
        this.canRun = true;
        this.play(wpCurrent, completeCycles, false);
      } else {
        console.log('Playing');
        // CONSTANTS:
        const baseMotors = 0.77469997968; // distance between motor wheels
        const minSpeed = 11;
        const maxSpeed = this.active.speed; // one rotation per second
        const cycles = 1;
        const speed = { left: 0, right: maxSpeed }; // left/right speeds in m/s
        const wpCount = this.active.waypoints.length;
        let waypointPos = this.active.waypoints[wpCurrent];

        console.log(waypointPos.theta);

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
          waypointPos = this.active.waypoints[wpCurrent];
        }
        // Determine appropriate ratio for current scenario
        const ratio = getRatio(speed, baseMotors, platformPos, waypointPos);
        // Set speed of motors: In final version, send out packet
        speed.left = Math.round(ratio[0] * (maxSpeed - minSpeed) + Math.sign(ratio[0]) * minSpeed);
        speed.right = Math.round(ratio[1] * (maxSpeed - minSpeed) + Math.sign(ratio[1]) * minSpeed);
        const packet = `1234|0|0|353|0|${speed.left}|${speed.right}|0|${speed.left + speed.right}\n`;
        console.log(`Index is: ${wpCurrent}, cycles run is: ${completeCycles}, ratio is ${ratio[0]}, ${ratio[1]} and speed is ${speed.left}, ${speed.right}`);
        this.$port.write(packet, () => {
          console.log('Wrote');
          setTimeout(() => {
            if (completeCycles < cycles && this.canRun) {
              this.play(wpCurrent, completeCycles, false);
            } else {
              console.log('We\'re done here.');
              this.eStop();
              this.canRun = false;
            }
          }, 100);
        });
      }
    },
    step() {
      if (this.isRecording) {
        this.ADD_WAYPOINT({
          x: parseFloat(this.position.x),
          y: parseFloat(this.position.y),
          theta: parseFloat(this.rotation)
        });
      }
    },
    goToStart(returnState) {
      if (returnState === -1) {
        this.canRun = true;
      }
      if (this.active.start && this.canRun) {
        const start = {
          x: this.active.start.x,
          y: this.active.start.y,
          theta: this.active.start.theta
        };
        // Get rotation amount
        const desiredTheta = Math.atan2(
          start.y - this.position.y,
          start.x - this.position.x
        );

        const newState = 1;
        const distance = Math.sqrt((this.position.x - start.x) ** 2 +
        (this.position.y - start.y) ** 2);

        let thetaDiff = -1;
        let speed = 20;
        let thetaError = 0.2;
        let checksum = 40;

        if (distance > 0.1) {
          thetaDiff = mod((this.rotation - desiredTheta), 2 * Math.PI);
          speed = 20;
          thetaError = 0.2;
          checksum = speed * 2;
        } else {
          thetaDiff = mod((this.rotation - start.theta), 2 * Math.PI);
          speed = 16;
          thetaError = 0.1;
          checksum = speed * 2;
        }

        console.log('distance, thetaDiff', distance, thetaDiff);

        if (distance <= 0.1) {
          if (Math.PI >= thetaDiff && thetaDiff > 0 + thetaError) {
            console.log('On Position: Clockwise');
            setTimeout(() => {
              this.$port.write(`1234|0|0|353|0|${speed}|-${speed}|0|0\n`);
              this.goToStart(newState);
            }, 50);
          } else if (2 * Math.PI - thetaError > thetaDiff && thetaDiff > Math.PI) {
            console.log('On Position: Counter-Clockwise');
            setTimeout(() => {
              this.$port.write(`1234|0|0|353|0|-${speed}|${speed}|0|0\n`);
              this.goToStart(newState);
            }, 50);
          } else {
            console.log('On Position: E-Stop');
            this.eStop();
          }
        } else if (Math.PI >= thetaDiff && thetaDiff > 0 + thetaError) {
          console.log('Off Position: Clockwise');
          setTimeout(() => {
            this.$port.write(`1234|0|0|353|0|${speed}|-${speed}|0|0\n`);
            this.goToStart(newState);
          }, 50);
        } else if (2 * Math.PI - thetaError > thetaDiff && thetaDiff > Math.PI) {
          console.log('Off Position: Counter-Clockwise');
          setTimeout(() => {
            this.$port.write(`1234|0|0|353|0|-${speed}|${speed}|0|0\n`);
            this.goToStart(newState);
          }, 50);
        } else if (distance > 0.1) {
          console.log('Off Position: Forward');
          setTimeout(() => {
            this.$port.write(`1234|0|0|353|0|${speed}|${speed}|0|${checksum}\n`);
            this.goToStart(newState);
          }, 50);
        }
      }
    },
    ...mapMutations([
      'SET_START',
      'ADD_WAYPOINT'
    ])
  },
  components: {
    CircleIcon,
    PlayIcon,
    XSquareIcon,
    AirplayIcon,
    CornerDownRightIcon
  },
  computed: {
    ...mapGetters({
      active: 'currCue',
      position: 'position',
      rotation: 'rotation',
    }),
    ...mapState({
      name: state => state.currCue.name
    })
  }
};
</script>

<style scoped>
label {
  font-size: 14px;
}
</style>

