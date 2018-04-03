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

    camera.position.set(0, 4, 0);
    camera.up = new THREE.Vector3(0, 0, 1);
    camera.lookAt(new THREE.Vector3(0, 0, 0));

    const floorGeometry = new THREE.BoxGeometry(30, 1, 30);
    const floorMaterial = new THREE.MeshBasicMaterial({
      color: 0x372411
    });
    const floor = new THREE.Mesh(floorGeometry, floorMaterial);
    scene.add(floor);
    floor.position.y = -1;
    floor.receiveShadow = true;

    // line// material
    // const lineGeometry = new THREE.BufferGeometry(1, 0.2, 1);
    // const linePositions = new Float32Array(500 * 3);
    // lineGeometry.addAttribute('position', new THREE.BufferAttribute(linePositions, 3));
    // const lineMaterial = new THREE.LineBasicMaterial({
    //   color: 0xff0000,
    //   linewidth: 2
    // });
    // const line = new THREE.Line(lineGeometry, lineMaterial);
    // scene.add(line);

    // // update line
    // function updateLine() {
    //   linePositions[count * 3 - 3] = mouse.x;
    //   linePositions[count * 3 - 2] = mouse.y;
    //   linePositions[count * 3 - 1] = mouse.z;
    //   line.geometry.attributes.position.needsUpdate = true;
    // }

    // // mouse move handler
    // function onMouseMove(event) {
    //   mouse.x = (event.clientX / CANVAS_WIDTH) * 2 - 1;
    //   mouse.y = -(event.clientY / CANVAS_HEIGHT) + 1;
    //   mouse.z = 0;
    //   mouse.unproject(camera);
    //   if (count !== 0) {
    //     updateLine();
    //   }
    // }

    // // add point
    // function addPoint() {
    //   // console.log(`point nr:  ${count}: ${mouse.x} ${mouse.y} ${mouse.z}.`);
    //   const positions = { x: mouse.x, y: mouse.y, z: mouse.z };
    //   linePositions[count * 3 + 0] = positions.x;
    //   linePositions[count * 3 + 1] = positions.y;
    //   linePositions[count * 3 + 2] = positions.z;
    //   count += 1;
    //   store.commit('ADD_POINTS', positions);
    //   line.geometry.setDrawRange(0, count);
    //   updateLine();
    // }

    // // mouse down handler
    // function onMouseDown() {
    //   // on first click add an extra point
    //   if (count === 0) {
    //     addPoint();
    //   }
    //   addPoint();
    // }

    // document.addEventListener('mousemove', onMouseMove, false);
    // document.addEventListener('mousedown', onMouseDown, false);

    // LIGHT

    const spot1 = new THREE.SpotLight(0xffffff);
    spot1.position.set(10, 100, 0);
    scene.add(spot1);

    const animate = () => {
      requestAnimationFrame(animate);
      cube.position.x = this.position.x * 1;
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

