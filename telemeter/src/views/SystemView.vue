<template>
<div class="system">
  <v-app-bar color="primary" height=40>
    <v-btn icon="mdi-arrow-left" :to="'/systems'"></v-btn>
    <v-app-bar-title>{{system?.name}}</v-app-bar-title>

    <LoginModal :system="system" v-on:login="onLogin" />
  </v-app-bar>
  <v-main>
    
    <v-card class="ma-10">
      <v-list>
        <v-row class="d-flex justify-space-between ma-0">
          <v-list-subheader>
            Active Flight
          </v-list-subheader>
          
          <v-dialog v-if="system?.admin" v-model="newFlight.dialog"
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
              <v-form v-model="newFlight.form" @submit.prevent="onNewFlight">
                <v-text-field v-model="newFlight.name"
                              label="Name"
                              :rules="[required]">
                </v-text-field>

                <v-card-actions>
                  <v-spacer></v-spacer>
                  <v-btn color="primary" @click="newFlight.dialog = false">
                    Cancel</v-btn>
                  <v-btn color="primary" :disabled="!newFlight.name"
                         type="submit">
                    Start
                  </v-btn>
                </v-card-actions>
              </v-form>
            </v-card>
          </v-dialog>
        </v-row>
        
        <v-list-item v-if="activeFlight"
                     :to="'/flight?id=' + activeFlight.id">
          <template v-slot:prepend>
            <v-icon icon="mdi-rocket-launch" color="primary"></v-icon>
          </template>
          <v-list-item-title v-text="activeFlight.name">
          </v-list-item-title>
          <v-list-item-subtitle>
            {{showDatetime(activeFlight.startTime)}}
            ~
            {{showDatetime(activeFlight.endTime)}}
          </v-list-item-subtitle>

          <template v-slot:append v-if="system?.admin">
            <v-btn icon="mdi-location-exit"
                   @click.stop.prevent="deactivateFlight(activeFlight)"></v-btn>
          </template>

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
                <v-text-field v-model="upload.name" label="Name"
                              :rules="[required]">
                </v-text-field>
                <div class="my-5">
                  Start Date
                  <VueDatePicker v-model="upload.startTime" enable-seconds dark>
                  </VueDatePicker>
                </div>
                <v-file-input label="File input" v-model="upload.file"
                              show-size prepend-icon="mdi-file">
                </v-file-input>
                <v-select
                  v-model="upload.version"
                  label="Log file version"
                  :items="['v1', 'v2', 'v3']"
                ></v-select>
                <v-text-field v-model="upload.source"
                              label="Source name"
                              :rules="[required, sourceName]">
                </v-text-field>
                <v-text-field v-model="upload.origin" label="Default origin"
                              :rules="[charactor]">
                </v-text-field>
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

        <v-list-item v-for="flight in pastFlights" :key="flight.name"
                     :to="'/flight?id=' + flight.id">
          <template v-slot:prepend>
            <v-icon icon="mdi-rocket"></v-icon>
          </template>
          <v-list-item-title v-text="flight.name"></v-list-item-title>
          <v-list-item-subtitle>
            {{showDatetime(flight.startTime)}}
            ~
            {{showDatetime(flight.endTime)}}
          </v-list-item-subtitle>
          <template v-slot:append v-if="system?.admin">
            <v-btn icon="mdi-location-enter"
                   @click.stop.prevent="reactivateFlight(flight)"></v-btn>
          </template>
        </v-list-item>
      </v-list>

    </v-card>

    </v-main>
  </div>
</template>

<script setup lang="ts">
import { reactive, ref, onMounted, inject, computed } from 'vue';
import { useRouter } from 'vue-router'
import axios from 'axios';
import VueDatePicker from '@vuepic/vue-datepicker';
import '@vuepic/vue-datepicker/dist/main.css'
import { System, Flight, api } from '../library/api'
import { Packet } from '../library/packet'
import LoginModal from '../components/LoginModal'


const system = ref<System | undefined>(undefined)
const flights = reactive<Flight[]>([])

const activeFlight = computed<Flight | undefined>(() => {
  return flights.find(f => !f.endTime)
})
const pastFlights = computed<Flight[]>(() => {
  return flights.filter(f => f.endTime)
})

const login = reactive({
  dialog: false,
  form: false,
  password: null,
})
const newFlight = reactive({
  dialog: false,
  form: false,
  name: "New Flight",
})
const upload = reactive({
  dialog: false,
  form: false,
  name: null,
  source: 'log',
  startTime: new Date(),
  file: null,
  version: 'v3',
  origin: '',
})

const router = useRouter()

onMounted(async () => {
  const systemId: string = inject('systemId')
  try {
    system.value = await api.getSystem(systemId)
    console.log('system', system.value)

    const allFlights = await api.getFlights(systemId)
    for (const flight of allFlights) {
      flights.push(flight)
    }
    console.log('flights', allFlights)
  } catch (e) {
    console.log('unknown system', systemId, e)
    router.push('/systems')
  }
})

const onLogin = (newSystem) => {
  system.value = newSystem
}


const onNewFlight = async () => {
  if (!newFlight.name) return

  console.log(newFlight.name)
  newFlight.dialog = false

  const flight = await api.postFlight(system.value.id, newFlight.name, true)
  console.log('new flight', flight)

  if (system.value?.activeFlight)
    flights.push(system.value?.activeFlight)
  if (system.value) system.value.activeFlight = flight
}

const onUpload = async () => {
  console.log('file', upload.file)
  if (!upload.file) return

  upload.dialog = false

  const version = {'v1': 1, 'v2': 2, 'v3': 3}[upload.version]
  const packets = await Packet.decodeLogFile(upload.file[0], version)

  for (const packet of packets) {
    packet.from = packet.from ?? upload.origin
  }

  if (packets.length == 0) return

  const flight = await api.postFlight(
    system.value.id, upload.name, upload.startTime,
    new Date(upload.startTime.getTime() + packets.at(-1)?.getTime()))

  console.log('new flight', flight)
  flights.push(flight)

  const connection = await api.connect(flight.id, upload.source)

  for (const packet of packets) {
    connection.send(packet,
                    new Date(upload.startTime.getTime() + packet.getTime()))
  }
  connection.close()
}

const deactivateFlight = async (flight: Flight) => {
  const newFlight = await api.putFlight(flight.id, flight.name, flight.startTime,
                                  flight.launchTime, new Date(), flight.data)
  flights.value = flights.map(f => f.id == newFlight.id ? newFlight : f)
  console.log(flights)
}

const reactivateFlight = async (flight: Flight) => {
  const newFlight = await api.activateFlight(flight.id)
  flights.value = flights.map(f => f.id == newFlight.id ? newFlight : f)
  console.log(flights)
}

const required = (v: any) => {
  return !!v || 'Field is required'
}

const sourceName = (v: string) => {
  return v.length <= 8 || 'Length of source name must be smaller than 8'
}

const charactor = (v: string) => {
  return v.length <= 1 || 'Origin id must be a charactor'
}


const showDatetime = (str: string) => {
  if (!str) return ""
  const date = new Date(str)
  return date.toLocaleString()
}
</script>
