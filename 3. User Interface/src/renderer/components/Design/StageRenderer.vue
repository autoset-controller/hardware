<template>
  <div id="render">
  </div>
</template>

<script>
import { mapGetters } from 'vuex';
// import store from '../../store';
const THREE = require('three.js');
// const OrbitControls = require('three-orbit-controls')(THREE);
export default {
  data() {
    return {
    };
  },
  mounted() {
    const renderCanvas = document.querySelector('#render');
    const CANVAS_WIDTH = renderCanvas.offsetWidth;
    const CANVAS_HEIGHT = 300;
    // const mouse = new THREE.Vector3();
    // let count = 0;

    // Scene, Camera, Renderer
    const scene = new THREE.Scene();
    const camera = new THREE.PerspectiveCamera(80, CANVAS_WIDTH / CANVAS_HEIGHT, 0.1, 1000);
    const renderer = new THREE.WebGLRenderer({
      antialias: true
    });
    renderer.setClearColor(0x000, 1.0);
    renderer.setSize(CANVAS_WIDTH, CANVAS_HEIGHT);

    // const controls = new OrbitControls(camera, renderer.domElement);

    // GEOMETRY & MATERIALS
    const boxGeometry = new THREE.BoxGeometry(1, 0.2, 1);
    const material = new THREE.MeshBasicMaterial({
      color: 0xffffff
    });
    const cube = new THREE.Mesh(boxGeometry, material);
    cube.rotation.x = 0;
    cube.rotation.y = 0;
    scene.add(cube);

    camera.position.set(0, 5, 0);
    camera.up = new THREE.Vector3(0, 0, 1);
    camera.lookAt(new THREE.Vector3(0, 0, 0));

    // SPHERES
    const sphereGeometry = new THREE.SphereGeometry(0.1, 8, 6);
    const sphereMaterial = new THREE.MeshBasicMaterial({
      color: 0xffffff
    });
    const sphere1 = new THREE.Mesh(sphereGeometry, sphereMaterial);
    sphere1.position.x = 1 / Math.sqrt(2);
    sphere1.position.y = 0;
    sphere1.position.z = 1 / Math.sqrt(2);
    scene.add(sphere1);

    const sphere2 = new THREE.Mesh(sphereGeometry, sphereMaterial);
    sphere2.position.x = -1 / Math.sqrt(2);
    sphere2.position.y = 0;
    sphere2.position.z = 1 / Math.sqrt(2);
    scene.add(sphere2);

    const sphere3 = new THREE.Mesh(sphereGeometry, sphereMaterial);
    sphere3.position.x = -1 / Math.sqrt(2);
    sphere3.position.y = 0;
    sphere3.position.z = -1 / Math.sqrt(2);
    scene.add(sphere3);

    const sphere4 = new THREE.Mesh(sphereGeometry, sphereMaterial);
    sphere4.position.x = 1 / Math.sqrt(2);
    sphere4.position.y = 0;
    sphere4.position.z = -1 / Math.sqrt(2);
    scene.add(sphere4);

    const floorGeometry = new THREE.BoxGeometry(30, 1, 30);
    const floorMaterial = new THREE.MeshBasicMaterial({
      color: 0x372411
    });
    const floor = new THREE.Mesh(floorGeometry, floorMaterial);
    scene.add(floor);
    floor.position.y = -1;
    floor.receiveShadow = true;

    // LIGHT
    const spot1 = new THREE.SpotLight(0xffffff);
    spot1.position.set(10, 100, 0);
    scene.add(spot1);

    console.log(document.querySelector('canvas'));

    const animate = () => {
      requestAnimationFrame(animate);
      cube.position.x = this.position.x * -1;
      cube.position.z = this.position.y * 1;
      cube.rotation.y = this.rotation;
      // controls.update();

      renderer.render(scene, camera);
    };

    renderCanvas.appendChild(renderer.domElement);
    animate();
  },
  computed: {
    ...mapGetters({
      position: 'position',
      rotation: 'rotation',
    })
  }
};
</script>

<style scoped>
#render {
  width: 100%;
}
</style>

