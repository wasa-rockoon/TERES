<template>
<div class="system">
  <v-app-bar color="primary" height=40>
    <v-btn icon="mdi-arrow-left" :to="'/systems'"></v-btn>
    <v-app-bar-title>{{system?.name}}</v-app-bar-title>
    
    <v-dialog v-model="login.dialog" width="auto">
      <template v-slot:activator="{ props }">
        <v-btn icon="mdi-lock" v-if="!(system?.admin)"
               v-bind="props">
        </v-btn>
        <v-btn icon="mdi-check" v-if="system?.admin"
               v-bind="props">
        </v-btn>
      </template>
      <v-card class="px-10" width=600>
        <v-card-title class="my-5">
          Login with Password
        </v-card-title>
        <v-form v-model="login.form" @submit.prevent="onLogin">
          <v-text-field v-model="login.password"
                        label="Password" type="password" :rules="[required]">
          </v-text-field>
          <v-card-actions>
            <v-spacer></v-spacer>
            <v-btn color="primary" :disabled="!login.form" type="submit">
              Login
            </v-btn>
          </v-card-actions>
        </v-form>
      </v-card>
    </v-dialog>
  </v-app-bar>
  <v-main>
    
    <v-card class="ma-10">
      <v-list>
        <v-row class="d-flex justify-space-between ma-0">
          <v-list-subheader>
            Active Flight
          </v-list-subheader>
          
          <v-dialog v-if="system?.admin" v-model="new_flight.dialog"
                    width="auto">
            <template v-slot:activator="{ props }">
              
              <v-btn v-bind="props" variant="outlined" color="primary"
                     class="ma-2" prepend-icon="mdi-rocket-launch">
                New Flight
              </v-btn>
            </template>
            <v-card class="px-10" width=600>
              <v-card-title class="my-5">
                Start New Flight
              </v-card-title>
              <v-form v-model="form" @submit.prevent="onNewFlight">
                <v-text-field v-model="new_flight.name"
                              label="Name"
                              :rules="[required]">
                </v-text-field>
                <v-card-text>
                  <v-icon icon="mdi-alert"></v-icon>
                  Current flight will be terminated.
                </v-card-text>
                <v-card-actions>
                  <v-spacer></v-spacer>
                  <v-btn color="primary" @click="new_flight.dialog = false">
                    Cancel</v-btn>
                  <v-btn color="primary" :disabled="!new_flight.name"
                         type="submit">
                    Start
                  </v-btn>
                </v-card-actions>
              </v-form>
            </v-card>
          </v-dialog>
        </v-row>
        
        <v-list-item v-if="system?.active_flight"
                     :to="'/flight?id=' + system.active_flight.id">
          <template v-slot:prepend>
            <v-icon icon="mdi-rocket-launch" color="primary"></v-icon>
          </template>
          <v-list-item-title v-text="system?.active_flight.name">
          </v-list-item-title>
          <v-list-item-subtitle>
            {{showDatetime(system?.active_flight.start_time)}}
            ~
            {{showDatetime(system?.active_flight.end_time)}}
          </v-list-item-subtitle>
          
        </v-list-item>
        
        <v-divider inset></v-divider>
        
        <v-row class="d-flex justify-space-between ma-0">
          <v-list-subheader>
            Past Flights
          </v-list-subheader>
          <v-dialog v-if="system?.admin" v-model="upload.dialog" width="auto">
            <template v-slot:activator="{ props }">
              <v-btn v-bind="props" variant="outlined" color="primary"
                     class="ma-2" prepend-icon="mdi-cloud-upload">
                Upload Log File
              </v-btn>
            </template>
            <v-card class="px-10" width=600 min-height=700>
              <v-card-title class="my-5">
                Upload Log File
              </v-card-title>
              <v-form v-model="upload.form" @submit.prevent="onUpload">
                <v-text-field v-model="upload.name"
                              label="Name"
                              :rules="[required]">
                </v-text-field>
                <div class="my-5">
                  Start Date
                  <VueDatePicker v-model="upload.start_date" dark>
                  </VueDatePicker>
                </div>
                <v-file-input label="File input" prepend-icon="mdi-file">
                </v-file-input>
                <v-select
                  v-model="upload.version"
                  label="Log file version"
                  :items="['v1', 'v2', 'v3']"
                ></v-select>
                <v-card-actions>
                  <v-spacer></v-spacer>
                  <v-btn color="primary" @click="upload.dialog = false">
                    Cancel</v-btn>
                  <v-btn color="primary" :disabled="!upload.form"
                         type="submit">
                    Upload
                  </v-btn>
                </v-card-actions>
              </v-form>
            </v-card>
          </v-dialog>
        </v-row>



        <v-list-item v-for="flight in flights" :key="flight.name"
                     :to="'/flight?id=' + flight.id">
          <template v-slot:prepend>
            <v-icon icon="mdi-rocket"></v-icon>
          </template>
          <v-list-item-title v-text="flight.name"></v-list-item-title>
          <v-list-item-subtitle>
            {{showDatetime(flight.start_time)}}
            ~
            {{showDatetime(flight.end_time)}}
          </v-list-item-subtitle>
        </v-list-item>
      </v-list>


    </v-card>

    </v-main>
  </div>
</template>

<script setup lang="ts">
import { reactive, ref, onMounted, inject } from 'vue';
import { useRouter } from 'vue-router'
import axios from 'axios';
import VueDatePicker from '@vuepic/vue-datepicker';
import '@vuepic/vue-datepicker/dist/main.css'
import { System, Flight } from '../library/types'


const system = ref<System | undefined>(undefined)
const flights = reactive<Flight[]>([])
const login = reactive({
  dialog: false,
  form: false,
  password: null,
})
const new_flight = reactive({
  dialog: false,
  form: false,
  name: "New Flight",
})
const upload = reactive({
  dialog: false,
  form: false,
  name: null,
  start_date: new Date(),
  version: 'v3'
})

onMounted(() => {
  const system_id = inject('system_id')
  axios.get(`/systems/${system_id}`, {
    params: {
      password: localStorage.password
    }
  })
       .then(response => {
         console.log(response.data)
         
         system.value = response.data
         
         axios.get(`/systems/${system_id}/flights/`)
              .then(response => {
                console.log(response.data)
                for (const flight of response.data.flights) {
                  if (system.value?.active_flight &&
                      flight.id == system.value?.active_flight.id) {
                    // this.system.active_flight = flight
                  }
                  else flights.push(flight)
                }
              })
       })
       .catch(() => {
         console.log('unknown system', inject('system_id'))
         useRouter().push('/systems')
       })
})

const onLogin = () => {
  if (!login.form) return
  
  login.dialog = false
  
  axios.get(`systems/${inject('system_id')}`, {
    params: {
      password: login.password
    }
  })
       .then(response => {
         console.log(response.data)
         system.value = response.data
         localStorage.password = login.password
       })
}
const onNewFlight = () => {
  if (!new_flight.name) return
  
  console.log(new_flight.name)
  
  new_flight.dialog = false
  
  axios.post(`systems/${inject('system_id')}/flights/`, null,
             {
               params: {
                 name: new_flight.name,
                 activate: true,
                 password: localStorage.password,
               }
  })
       .then(response => {
         console.log(response.data)
         if (system.value?.active_flight)
           flights.push(system.value?.active_flight)
         if (system.value) system.value.active_flight = response.data
       })
}
const onUpload = () => {
    console.log(upload)
}
const required = (v: any) => {
  return !!v || 'Field is required'
}
const showDatetime = (str: string) => {
  if (!str) return ""
  const date = new Date(str)
  return date.toLocaleString()
}
</script>
