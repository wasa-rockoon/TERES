<template>
  <div class="system h-screen overflow-hidden">
    <v-app-bar color="primary" height=40>
    <v-btn icon="mdi-arrow-left" :to="'/'"></v-btn>
    <v-spacer></v-spacer>

    <FlightSettings :flight="flight" v-on:update="onUpdateFlight"/>

    <v-spacer></v-spacer>

    <LoginModal :system="system" v-on:login="onLogin" />

    </v-app-bar>
    <v-main v-if="flight" class="h-100 overflow-hidden" >
      <v-container class="ma-0 w-screen h-100">
        <v-row class="h-100">
          <v-container id="graphics-panel" class="ma-0 pa-0 h-100">
            <v-row class="h-100 ma-0">
              <v-col class="pa-3 pr-1">
                <v-row class="w-100 pb-4 ma-0">
                  <VideoPlayer v-if="flight.data?.videos?.at(0)"
                               :video="flight.data.videos[0]"
                               :time="currentTime"/>
                </v-row>
                <v-row class="w-100 pb-4 ma-0">
                  <FlightMap :time="currentTime"/>
                </v-row>
              </v-col>
              <v-col class="h-100 pa-3 pr-1"
                     style="display: flex; flex-direction: column;">
                <v-row class="w-100 pb-4 ma-0" style="flex-grow: 0">
                  <VideoPlayer v-if="flight.data?.videos?.at(1)"
                               :video="flight.data.videos[1]"
                               :time="currentTime"/>
                </v-row>
                <v-row class="overflow-y-auto w-100 ma-0"
                       style="flex-grow: 1; padding-bottom: 50px">
                  <ChartList :range="chartRange" />
                </v-row>
              </v-col>
            </v-row>
          </v-container>
          <v-col id="packets-panel" class="overflow-y-auto h-100 pb-16">
            <v-card>
              <PacketList :time="currentTime"/>
            </v-card>
          </v-col>
        </v-row>
      </v-container>

    </v-main>

    <v-footer fixed elevation="20">
      <FlightTimeline v-on:change-chart-range="onChangeChartRange"
                      v-on:change-time="onChangeTime" />
    </v-footer>
  </div>
</template>

<script setup lang="ts">
import { reactive, ref, onMounted, inject, provide, computed,
         getCurrentInstance, watch, shallowRef, triggerRef} from 'vue';
import { useRoute, useRouter, onBeforeRouteLeave } from 'vue-router'
import axios from 'axios'
import { Flight, System, api } from '../library/api'
import { DataStore } from '../library/datastore'
import '@/library/packet'
import { Packet } from '../library/packet'
import LoginModal from '../components/LoginModal'
import FlightSettings from '../components/FlightSettings'
import FlightTimeline from '../components/FlightTimeline'
import PacketList from '../components/PacketList'
import ChartList from '../components/ChartList'
import FlightMap from '../components/FlightMap'
import VideoPlayer from '../components/VideoPlayer'

const datastore = shallowRef<DataStore | undefined>(undefined)

const flight = computed<Flight>(() => datastore.value?.flight)
const system = computed<System | undefined>(() => flight.value?.system)

provide('datastore', datastore)

const currentTime = ref(new Date())
const chartRange = ref({})

let connection = undefined

const route = useRoute()
const router = useRouter()

const instance = getCurrentInstance()

onMounted(async () => {
  try {
    const flight = await api.getFlight(route.query.id)
    datastore.value = new DataStore(flight)
    datastore.value.currentTime = flight.startTime

    connection = await api.connect(flight.id, 'web',
                                   flight.startTime, undefined,
                                   onReceive)
    console.log('flight', flight, connection)
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
  triggerRef(datastore)
}

const onUpdateFlight = async (flight_) => {
  datastore.value.flight = flight_
  triggerRef(datastore)
}

const onLogin = (newSystem) => {
  flight.value.system = newSystem
}

const onChangeTime = (time) => {
  currentTime.value = time
}

const onChangeChartRange = (range) => {
  chartRange.value = range
}


</script>

<style>
#graphics-panel {
  width: calc(100% - 320px);
}
#packets-panel {
  width: 320px;
}

.v-main>.v-container {
  max-width: 100vw;
}

 .v-footer {
   position: sticky;
   bottom: 0;
 }
</style>
