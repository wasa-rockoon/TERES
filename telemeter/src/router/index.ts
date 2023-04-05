import { createRouter, createWebHistory } from 'vue-router'
import SystemListView from '../views/SystemListView.vue'
import SystemView from '../views/SystemView.vue'
import FlightView from '../views/FlightView.vue'

const routes = [
  {
    path: '/',
    name: 'system',
    component: SystemView
  },
  {
    path: '/systems',
    name: 'systemlist',
    component: SystemListView
  },
  {
    path: '/flight',
    name: 'flight',
    component: FlightView
  },

]

const router = createRouter({
  history: createWebHistory(process.env.BASE_URL),
  routes
})

export default router
