<template>
  Packets
  <v-list v-model:opened="open" density="compact">
    <v-list-group :value="p.packet.id" v-for="p in packets" :key="p.packet.id">
      <template v-slot:activator="{ props }">
        <v-list-item v-bind="props" :title="p.format.name"
                     :subtitle="datastore.showT(p.t)">
        </v-list-item>
      </template>

      <v-table density="compact">
        <tbody>
          <tr
            v-for="(entry, i) in packetEntries(p.format, p.packet)"
            :key="i"
          >
            <td class="text-left">{{ entry.title }}</td>
            <td class="text-right">{{ entry.value }}</td>
          </tr>
        </tbody>
      </v-table>
    </v-list-group>
  </v-list>
</template>>


<script setup lang="ts">
import { reactive, ref, onMounted, inject, computed, Ref, watch } from 'vue';
import { DataStore } from '../library/datastore'
import * as settings from '../settings'

const datastore = inject<Ref<DataStore>>('datastore')

const open = ref<string[]>(settings.packetList)

const packets = computed(() => {
  if (!datastore.value) return []
  return settings.packetList.map(id => {
    return datastore.value.getById(id)
          ?.at(datastore.value.currentTime)
  })?.filter(p => p)
})


watch(packets, () => {
  // console.log('packets', packets.value)
})


const packetEntries = (format, packet) => {
  const entries = []
  let index = 0;
  let prevType = ''
  packet.entries.forEach((entry, n) => {
    if (n == packet.entries.length - 1 && entry.type == "t") return

    if (entry.type == prevType) index++
    else index = 0

    const f = format?.entries[entry.type]
    if (f) {
      let title = f.name
      if (f.index) title += ` (${f.index[index]})`
      if (f.unit) title += ` [${f.unit}]`

      const value = entry.payload[f.datatype]
      let valueStr = String(value)
      if (f.datatype == 'float16') valueStr = value.toPrecision(3)
      else if (f.datatype == 'float32') valueStr = value.toPrecision(7)

      entries.push({title: title, value: valueStr})
    }
    else {
      entries.push({
        title: entry.type,
        value: entry.payload.int32
      })
    }
    prevType = entry.type
  })
  return entries
}


</script>

