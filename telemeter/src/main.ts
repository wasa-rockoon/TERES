// import './plugins/axios'
import { createApp } from 'vue'
import App from './App.vue'
import vuetify from './plugins/vuetify'
import { loadFonts } from './plugins/webfontloader'
import router from './router'
import axios from 'axios'
import '@mdi/font/css/materialdesignicons.css'
import * as settings from './settings'


loadFonts()

declare module '@vue/runtime-core' {
    interface ComponentCustomProperties {
        $system_id: string;
        $password?: string;
    }
}

console.log('mode', process.env.NODE_ENV, settings)

const app = createApp(App)
app.provide('systemId', settings.systemId)

app.use(router)
  .use(vuetify)
  .mount('#app')
