<template>
  <section class="preview">
    <h3>Preview</h3>
    <stage-renderer></stage-renderer>
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

<style>
.preview {
    height: 100%;
}
#canvas {
    width: 100%;
    height: 100%;
}
</style>

