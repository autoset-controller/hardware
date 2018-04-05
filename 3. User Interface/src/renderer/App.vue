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
    name: 'autoset-vue-electron',
    mounted() {
      console.log('Pinging server');
      this.$port.write('1234|PING|\n');
    }
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
    background: #000;
    font-size: 14px;
  }
  body{
    height: 100vh;
    margin: 0;
    background: #1C1C1C;
    color: white;
    display: flex;
    flex-direction: column;
    font-family: 'Source Sans Pro', sans-serif;
    font-size: 16px;
  }
  nav {
      text-align: center;
      /* border-bottom: 1px solid white; */
  }
  nav .nav-item{
      display: inline;
      font-size: 2rem;
      margin: 20px;
      cursor: pointer;
  }
  nav .nav-item.active {
      text-shadow: 0px 2px 5px white;
  }
  button {
    color: white;
    background: black;
    border: 1px solid white;
    padding: 5px;
    margin: 5px;
    cursor: pointer;
  }
  input[type="text"] {
    color: black;
  }
</style>
