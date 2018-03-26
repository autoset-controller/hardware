<template>
  <div id="app">
    <div id="title-bar">
      <p>Autoset Controller</p>
    </div>
    <navigation></navigation>
    <router-view></router-view>
  </div>
</template>

<script>
  import Navigation from './components/Navigation';
  export default {
    components: {
      Navigation
    },
    name: 'autoset-vue-electron'
  };
</script>

<style>
  /* CSS */
  * {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  }
  /* Let's get this party started */ 
  ::-webkit-scrollbar { 
    width: 12px; 
    background: black;
  } 
  /* Track */ 
  ::-webkit-scrollbar-track { 
    -webkit-box-shadow: inset 0 0 6px rgba(0,0,0,0.3); 
    -webkit-border-radius: 10px; border-radius: 10px; 
  } 
  /* Handle */ 
  ::-webkit-scrollbar-thumb { 
    -webkit-border-radius: 10px; 
    border-radius: 10px; 
    background: rgb(255, 255, 255); 
    -webkit-box-shadow: inset 0 0 6px rgba(0,0,0,0.5); 
  }
  #title-bar {
    -webkit-app-region: drag;
    text-align: center; 
    background: #1C1C1C;
    font-size: 14px;
  }
</style>
