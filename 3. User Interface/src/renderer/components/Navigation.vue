<template>
  <nav>
    <router-link to="/design" class="active">Design</router-link>
    <router-link to="/test">Test</router-link>
    <router-link to="/play">Play</router-link>
  </nav>
</template>

<script>
export default {};
</script>

<style scoped>
  nav {
    margin: 10px 0;
  }
  nav a {
    color: white;
    font-size: 18px;
    margin: 0 25px;
  }
    a.active {
      border-bottom: solid 2px white;
    }
</style>
