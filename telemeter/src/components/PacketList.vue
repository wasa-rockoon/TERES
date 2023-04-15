<template>
  <v-card v-for="(list, i) in packets" :key="i">
    <v-card-item>
      <template v-slot:prepend>
        <v-icon v-if="list.data.some(p => !p.packet)" icon="mdi-help-circle">
        </v-icon>
        <v-icon v-else-if="list.data.some(error)" color="error"
                icon="mdi-alert-circle"></v-icon>
        <v-icon v-else-if="list.data.some(warning)" color="warning"
                icon="mdi-alert"></v-icon>
        <v-icon v-else-if="list.data.some(timeout)" color="warning"
                icon="mdi-clock-alert-outline"></v-icon>
        <v-icon v-else icon="mdi-check" color="primary"></v-icon>
      </template>

      <v-card-title>{{list.name}}</v-card-title>

      <template v-slot:append>
        <v-btn density="compact" icon="mdi-dots-vertical"></v-btn>
      </template>
    </v-card-item>
    <v-card-text class="pa-0">
      <v-list v-model:opened="open[list.from]">
        <v-list-group v-for="p in list.data" :key="p.id" :value="p.id">

          <template v-slot:activator="{ props }">
            <v-list-item v-bind="props">
              <template v-slot:prepend>
                <v-icon v-if="!p.packet" icon="mdi-help-circle"></v-icon>
                <v-icon v-else-if="error(p)" color="error"
                        icon="mdi-alert-circle"></v-icon>
                <v-icon v-else-if="warning(p)" color="warning"
                        icon="mdi-alert"></v-icon>
                <v-icon v-else-if="timeout(p)" color="warning"
                        icon="mdi-clock-alert-outline"></v-icon>
                <v-icon v-else icon="mdi-check" color="primary"></v-icon>
              </template>
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
              <v-tooltip
                v-for="(entry, i) in packetEntries(p.format, p.packet)"
                :key="i"
                :text="entry.error || entry.warning || ''"
              >
                <template v-slot:activator="{ props }">
                  <tr v-bind="(entry.error || entry.warning) && props">
                    <td class="text-left">{{ entry.title }}</td>
                    <td v-if="entry.error" class="text-right text-error">
                      {{ entry.value }}</td>
                    <td v-else-if="entry.warning" class="text-right text-warning">
                      {{ entry.value }}</td>
                    <td v-else class="text-right">{{ entry.value }}</td>
                  </tr>
                </template>
              </v-tooltip>

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
import { DataStore, PacketInfo } from '../library/datastore'
import { Packet } from '../library/packet'
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


const packetEntries = (format: any, packet: Packet) => {
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

      const value = entry.format(f)

      entries.push({
        title: title,
        value: value,
        error: f.error && f.error(value),
        warning: f.warning && f.warning(value),
      })
    }
    else {
      entries.push({
        title: entry.type,
        value: entry.payload.int32,
        error: "Unknown entry",
        warning: null,
      })
    }
  })
  return entries
}

const timeout = (p: PacketInfo) => {
  if (!p.format.timeout) return 0
  const t = datastore.value.time2t(props.time) - p.t
  if (t > p.format.timeout) return t
  else return 0
}

const error = (p: PacketInfo) => {
  if (p.format.error && p.format.error(p.packet)) return true
  return p.packet.entries.some(entry => {
    const f = p.format.entries[entry.type]
    if (entry.type != 't' && !f) return true
    return f && f.error && f.error(entry.format(f))
  })
}


const warning = (p: PacketInfo) => {
  if (p.format.warning && p.format.warning(p.packet)) return true
  return p.packet.entries.some(entry => {
    const f = p.format.entries[entry.type]
    return f && f.warning && f.warning(entry.format(f))
  })
}

</script>

