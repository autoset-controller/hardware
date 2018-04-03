<template>
  <section class="configuration">
      <h3>Configuration</h3>

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
};
</script>

<style scoped>

</style>

