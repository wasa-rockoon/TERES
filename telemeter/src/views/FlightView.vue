<template>
  <div class="system h-screen overflow-hidden">
    <v-app-bar color="primary" height=40>
    <v-btn icon="mdi-arrow-left" :to="'/'"></v-btn>
    <v-spacer></v-spacer>

    <v-dialog v-model="setting.dialog" width="auto">
      <template v-slot:activator="{ props }">
        <v-btn v-bind="props" size="x-large" class="text-capitalize">
          {{system?.name}} - {{flight?.name}}
        </v-btn>
      </template>

      <v-card class="px-10" width=600 min-height=800>
        <v-card-title class="my-5">
          Settings
        </v-card-title>
        <v-form v-model="setting.form" @submit.prevent="onSaveSetting">
          <v-text-field v-model="setting.name" label="Name"
                        :rules="[required]">
          </v-text-field>
          <div class="my-5">
            Start time
            <VueDatePicker v-model="setting.startTime" enable-seconds dark>
            </VueDatePicker>
          </div>
          <div class="my-5">
            Launch time
            <VueDatePicker v-model="setting.launchTime" enable-seconds dark>
            </VueDatePicker>
          </div>
          <div class="my-5">
            End time
            <VueDatePicker v-model="setting.endTime" enable-seconds dark>
            </VueDatePicker>
          </div>
          <v-card-actions>
            <v-spacer></v-spacer>
            <v-btn color="primary" type="submit">
              Save
            </v-btn>
          </v-card-actions>
        </v-form>
      </v-card>
    </v-dialog>

    <v-spacer></v-spacer>

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
    <v-main class="h-100 overflow-hidden">
      <v-container class="ma-0 w-screen h-100">
        <v-row class="h-100">
          <v-container id="graphics-panel" class="ma-0 pa-3">
            <v-row>
              <v-col>

              </v-col>
              <v-col class="overflow-y-auto h-100 pb-16">
                <ChartList/>
              </v-col>
            </v-row>
          </v-container>
          <v-col id="packets-panel" class="overflow-y-auto h-100 pb-16">
            <v-card>
              <PacketList/>
            </v-card>
          </v-col>
        </v-row>
      </v-container>


    </v-main>

    <v-footer fixed elevation="20">
      <FlightTimeline/>
    </v-footer>
  </div>
</template>

<script setup lang="ts">
import { reactive, ref, onMounted, inject, provide, computed,
         getCurrentInstance, watch} from 'vue';
import { useRoute, useRouter, onBeforeRouteLeave } from 'vue-router'
import axios from 'axios'
import VueDatePicker from '@vuepic/vue-datepicker';
import { Flight, System, api } from '../library/api'
import { DataStore } from '../library/datastore'
import '@/library/packet'
import { Packet } from '../library/packet'
import FlightTimeline from '../components/FlightTimeline'
import PacketList from '../components/PacketList'
import ChartList from '../components/ChartList'

const datastore = ref<DataStore | undefined>(undefined)

const flight = computed<Flight>(() => datastore.value?.flight)
const system = computed<System | undefined>(() => flight.value?.system)

provide('datastore', datastore)

const login = reactive({
  dialog: false,
  form: false,
  password: null,
})
const setting = reactive({
  dialog: false,
  form: false,
  name: "",
  startTime: new Date(),
  launchTime: null,
  endTime: null,
})

let connection = undefined

const route = useRoute()
const router = useRouter()

const instance = getCurrentInstance()

watch(datastore, () => {

})

onMounted(async () => {
  try {
    const flight = await api.getFlight(route.query.id)
    datastore.value = new DataStore(flight)
    datastore.value.currentTime = flight.startTime
    setting.name = flight.name
    setting.startTime = flight.startTime
    setting.launchTime = flight.launchTime
    setting.endTime = flight.endTime

    connection = await api.connect(flight.id, 'web',
                                   flight.startTime, undefined,
                                   onReceive)
    console.log('flight', flight, connection, setting)
  }
  catch {
    console.log('unknown flight', inject('systemId'), route.query.id)
    router.push('/systems')
  }
})

onBeforeRouteLeave((to, from) => {
  connection?.close()
})

const onReceive = (packets: {packet: Packet, time: Date, source: string }[]) => {
  console.log('received', packets.length)
  datastore.value.addPackets(packets)
}

const onSaveSetting = async () => {
  const flight_ = await api.putFlight(
    flight.value.id, setting.name,
    setting.startTime, setting.launchTime, setting.endTime
  )
  console.log('save', flight_)
  datastore.value.flight = flight_
}

const onLogin = async () => {
  if (!login.form) return

  localStorage.password = login.password
  login.dialog = false

  system.value = await api.getSystem(system.value.id)
}

const required = (v: any) => {
  return !!v || 'Field is required'
}


</script>

<style>
#graphics-panel {
  width: calc(100% - 300px);
}
#packets-panel {
  width: 300px;
}

.v-main>.v-container {
  max-width: 100vw;
}

 .v-footer {
   position: sticky;
   bottom: 0;
 }
</style>
