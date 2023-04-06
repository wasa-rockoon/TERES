<template>
<div class="system">
  <v-app-bar color="primary" height=40>
    <v-btn icon="mdi-arrow-left" :to="'/'"></v-btn>
    <v-app-bar-title>{{system?.name}} - {{flight?.name}}</v-app-bar-title>
    
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
      
    </v-card>

    </v-main>
  </div>
</template>

<script setup lang="ts">
    import { reactive, ref, onMounted, inject } from 'vue';
import { useRoute, useRouter, onBeforeRouteLeave } from 'vue-router'
import axios from 'axios'
import { Flight, System, api } from '../library/api'
import '../library/packet'
import { Packet } from '../library/packet';

const flight = ref<Flight | undefined>()
const system = ref<System | undefined>()
const login = reactive({
    dialog: false,
    form: false,
    password: null,
})

let connection = undefined

let route = useRoute()
let router = useRouter()

onMounted(async () => {
  try {
    flight.value = await api.getFlight(route.query.id)
    system.value = flight.value.system
    connection = await api.connect(flight.value.id, 'web',
                                   flight.value.start_time, undefined,
                                   onReceive)
    console.log('flight', flight.value, connection)
  }
  catch {
    console.log('unknown flight', inject('systemId'), route.query.id)
    router.push('/systems')
  }
})

onBeforeRouteLeave((to, from) => {
  connection?.close()
})


const onReceive = (packet: Packet, time: Date, source: string) => {
  // console.log(packet)
}

const onLogin = () => {
  if (!login.form) return
  
  login.dialog = false
  
  axios.get(`/systems/${inject('systemId')}`, {
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

const required = (v: any) => {
  return !!v || 'Field is required'
}


</script>
