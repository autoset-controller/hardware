<template>
  <section class="preview">
    <h3>Preview</h3>
    <p>Back</p>
    <stage-renderer></stage-renderer>
    <p>Front</p>
  </section>
</template>

<script>
import StageRenderer from './StageRenderer.vue';
export default {
  components: {
    StageRenderer
  }
};
</script>

<style scoped>
.preview {
    height: 100%;
}
p {
  font-size: 12px;
  text-align: center;
}
</style>

