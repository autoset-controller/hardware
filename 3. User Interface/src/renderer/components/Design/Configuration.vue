<template>
  <section class="configuration">
      <h3>Configuration</h3>
      <button>Connect</button>
      <button @click="setPos">Set Position</button>
      <button class="submit" @click="sendData">Save</button>
      <button class="secondary" @click="emergencyStop">Cancel</button>

      <div class="form-row">
        <div class="formElement">
          <label for="name">Cue Name</label>
          <input type="text" name="name" v-model="active.name">
        </div>
  
        <div class="formElement">
          <label for="duration">Duration</label>
          <input type="text" name="duration" v-model="active.duration">
        </div>
  
        <div class="formElement">
          <label for="speed">Speed</label>
          <input type="text" name="speed" v-model="active.speed">
        </div>
      </div>
    </section>
</template>

<script>
export default {
  data() {
    return {
      active: {
        name: '',
        duration: 0,
        speed: 0
      }
    };
  },
  methods: {
    sendData() {
      console.log('Sending data...');
      // Write data and confirm it was written
      // Message: key|platform|target|id|cmd|left|right|delay|checksum
      let message = '';
      message = Buffer.from('1234|0|999|353|0|-16|-16|5|-27\n');
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
    }
  }
};
</script>

<style scoped>

</style>

