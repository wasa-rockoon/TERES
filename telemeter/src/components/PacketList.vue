<template>

  <v-card v-for="list in packets" :key="list.from">
    <v-card-item>
      <v-card-title>{{list.name}}</v-card-title>
    </v-card-item>
    <v-card-text class="pa-0">
      <v-list v-model:opened="open[list.from]">
        <v-list-group v-for="p in list.data" :key="p.id" :value="p.id">
          <template v-slot:activator="{ props }">
            <v-list-item v-bind="props">
              <v-list-item-title>{{p.format.name}}</v-list-item-title>
              <v-list-item-subtitle v-if="p.packet">
                {{datastore.showT(p.t)}} #{{p.count}} {{p.source}}
              </v-list-item-subtitle>
              <v-list-item-subtitle v-if="!p.packet">
                N/A
              </v-list-item-subtitle>
            </v-list-item>
          </template>

          <v-table density="compact" v-if="p.packet">
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
    </v-card-text>
  </v-card>
</template>

<script setup lang="ts">
import { reactive, ref, onMounted, inject, computed, Ref, watch,
         defineProps, ShallowRef } from 'vue'
import { DataStore } from '../library/datastore'
import * as settings from '../settings'

const props = defineProps(['time'])

const datastore = inject<ShallowRef<DataStore>>('datastore')

const initialOpen = {}
settings.packetList.forEach(list => {
  initialOpen[list.from] = list.ids
})

const open = ref(initialOpen)

const packets = computed(() => {
  if (!datastore.value) return []
  return settings.packetList.map(list => {
    const data = list.ids.map(id =>
      datastore.value.getBy(list.from, id)?.at(props.time ?? new Date())
    ).filter(p => p)
    return { data: data, from: list.from, name: list.name }
  })
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
    prevType = entry.type

    const f = format?.entries[entry.type]
    if (f) {
      let title = f.name
      if (f.index) title += ` (${f.index[index]})`
      if (f.unit) title += ` [${f.unit}]`

      entries.push({title: title, value: entry.format(f)})
    }
    else {
      entries.push({
        title: entry.type,
        value: entry.payload.int32
      })
    }
  })
  return entries
}


</script>

