// import './plugins/axios'
import { createApp } from 'vue'
import App from './App.vue'
import vuetify from './plugins/vuetify'
import { loadFonts } from './plugins/webfontloader'
import router from './router'
import axios from 'axios'
import '@mdi/font/css/materialdesignicons.css'

loadFonts()

axios.defaults.baseURL = 'http://localhost:8888/api';
axios.defaults.headers.put['Content-Type'] = 'application/json;charset=utf-8';
axios.defaults.headers.put['Access-Control-Allow-Origin'] = '*';

declare module '@vue/runtime-core' {
    interface ComponentCustomProperties {
        $system_id: string;
        $password?: string;
    }
}

const app = createApp(App)
app.provide('system_id', 'teres1')

app.config.globalProperties.$system_id = 'teres1'
app.config.globalProperties.$password = undefined

app.use(router)
  .use(vuetify)
  .mount('#app')
