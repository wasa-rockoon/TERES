import { createRouter, createWebHistory } from 'vue-router'
import SystemListView from '../views/SystemListView.vue'
import SystemView from '../views/SystemView.vue'
import FlightView from '../views/FlightView.vue'
import * as settings from '../settings'


const base = process.env.NODE_ENV == 'development' ? '/' : settings.basePath


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

console.log(process.env.BASE_URL)

const router = createRouter({
    history: createWebHistory(base),
    routes
})

export default router
