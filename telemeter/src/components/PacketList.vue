<template>
  Packets
  <v-expansion-panels v-model="panel" multiple variant="accordion">
    <v-expansion-panel v-for="packet in packets" :key="packet.id" value="packet.id">
    <v-expansion-panel-title>
    {{packet[2]}}
    </v-expansion-panel-title>
    <v-expansion-panel-text>
      {{packet[0]}}
    </v-expansion-panel-text>
  </v-expansion-panel>
  </v-expansion-panels>

</template>>


<script setup lang="ts">
import { reactive, ref, onMounted, inject, computed, Ref, watch } from 'vue';
import { DataStore } from '../library/datastore'
import * as settings from '../settings'

const datastore = inject<Ref<DataStore>>('datastore')

const panel = ref<string[]>(settings.packetList)

const packets = computed(() => {
  if (!datastore.value) return []
  return settings.packetList.map(id => datastore.value.getCurrent(id))
                 .filter(p => p)
})

watch(packets, () => {
  // console.log('packets', packets.value[0])
})


</script>

