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
    const viewSize = CANVAS_HEIGHT;
    const aspectRatio = CANVAS_WIDTH / CANVAS_HEIGHT;
    const viewport = {
      viewSize: viewSize,
      aspectRatio: aspectRatio,
      left: (-aspectRatio * viewSize) / 2,
      right: (aspectRatio * viewSize) / 2,
      top: viewSize / 2,
      bottom: -viewSize / 2,
      near: -100,
      far: 100
    };
    // const mouse = new THREE.Vector3();
    // let count = 0;

    // Scene, Camera, Renderer
    const scene = new THREE.Scene();
    const camera = new THREE.OrthographicCamera(
      viewport.left,
      viewport.right,
      viewport.top,
      viewport.bottom,
      viewport.near,
      viewport.far
    );
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

    camera.position.set(0, 1, 0);
    camera.up = new THREE.Vector3(0, 0, 1);
    camera.lookAt(new THREE.Vector3(0, 0, 0));
    camera.zoom = 25;
    camera.updateProjectionMatrix();

    // SPHERES
    const sphereGeometry = new THREE.SphereGeometry(0.1, 8, 6);
    const sphereMaterial = new THREE.MeshBasicMaterial({
      color: 0xFFDC00
    });
    // const sphere1 = new THREE.Mesh(sphereGeometry, sphereMaterial);
    // sphere1.position.x = 1 / Math.sqrt(2);
    // sphere1.position.y = 0;
    // sphere1.position.z = 1 / Math.sqrt(2);
    // scene.add(sphere1);

    // const sphere2 = new THREE.Mesh(sphereGeometry, sphereMaterial);
    // sphere2.position.x = -1 / Math.sqrt(2);
    // sphere2.position.y = 0;
    // sphere2.position.z = 1 / Math.sqrt(2);
    // scene.add(sphere2);

    // const sphere3 = new THREE.Mesh(sphereGeometry, sphereMaterial);
    // sphere3.position.x = -1 / Math.sqrt(2);
    // sphere3.position.y = 0;
    // sphere3.position.z = -1 / Math.sqrt(2);
    // scene.add(sphere3);

    // const sphere4 = new THREE.Mesh(sphereGeometry, sphereMaterial);
    // sphere4.position.x = 1 / Math.sqrt(2);
    // sphere4.position.y = 0;
    // sphere4.position.z = -1 / Math.sqrt(2);
    // scene.add(sphere4);

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

    const animate = (prevSpheres) => {
      const spheres = [];
      requestAnimationFrame(() => {
        animate(spheres);
      });
      cube.position.x = this.position.x * 1;
      cube.position.z = -this.position.y * 1;
      cube.rotation.y = this.rotation;
      // remove existing spheres
      if (prevSpheres) {
        prevSpheres.forEach((sphere) => {
          scene.remove(sphere);
        });
      }
      // create spheres
      let customSphereMaterial = new THREE.MeshBasicMaterial({
        color: 0xfff
      });
      if (this.cueInfo.waypoints) {
        if (this.cueInfo.color) {
          customSphereMaterial = new THREE.MeshBasicMaterial({
            color: Number.parseInt(`0x${this.cueInfo.color}`, 16)
          });
        }
        const startSphere = new THREE.Mesh(sphereGeometry, sphereMaterial);
        startSphere.position.x = this.cueInfo.start.x;
        startSphere.position.y = -0.5;
        startSphere.position.z = -this.cueInfo.start.y;
        scene.add(startSphere);
        spheres.push(startSphere);
        for (let i = 0; i < this.waypoints.length; i++) {
          const sphere = new THREE.Mesh(sphereGeometry, customSphereMaterial);
          sphere.position.x = this.waypoints[i].x;
          sphere.position.y = 0.5;
          sphere.position.z = -this.waypoints[i].y;
          scene.add(sphere);
          spheres.push(sphere);
        }
      }
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
      waypoints: 'waypoints',
      cueInfo: 'currCue'
    })
  }
};
</script>

<style scoped>
#render {
  width: 100%;
}
</style>

