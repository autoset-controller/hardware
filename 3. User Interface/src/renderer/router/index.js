import Vue from 'vue';
import Router from 'vue-router';

Vue.use(Router);

export default new Router({
  routes: [
    {
      path: '/design',
      name: 'design',
      component: require('@/components/Design').default,
    },
    {
      path: '/test',
      name: 'test',
      component: require('@/components/Test').default,
    },
    {
      path: '*',
      redirect: '/design',
    },
  ],
});
