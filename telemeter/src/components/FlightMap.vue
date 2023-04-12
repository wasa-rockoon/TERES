<template>
  <v-card class="w-100" style="padding-top: 100%; position: relative">
    <l-map ref="map" v-model:zoom="zoom" :center="center"
           style="position: absolute; top: 0">
      <l-tile-layer
        url="https://cyberjapandata.gsi.go.jp/xyz/seamlessphoto/{z}/{x}/{y}.jpg"
        layer-type="base"
        name="OpenStreetMap"
      ></l-tile-layer>
      <l-polyline v-for="(path, i) in paths" :key="i"
                  :lat-lngs="path.latlons"
                  :color="path.color">
      </l-polyline>
      <l-circle-marker v-for="(marker, i) in markers" :key="i"
                       :lat-lng="marker.latlon"
                       :radius="15"
                       :color="marker.color">
      </l-circle-marker>
      <l-control-scale position="topright" :imperial="false" :metric="true">
      </l-control-scale>
    </l-map>
  </v-card>
</template>

<script setup lang="ts">
import { ref, reactive, inject, ShallowRef, defineProps, computed, watch, watchEffect
} from 'vue'
import "leaflet/dist/leaflet.css";
import { LMap, LTileLayer, LControlScale, LPolyline, LCircleMarker,
} from "@vue-leaflet/vue-leaflet";
import { DataStore } from '../library/datastore'
import * as settings from '../settings'

const props = defineProps(['time'])

const datastore = inject<ShallowRef<DataStore>>('datastore')

const map = ref(null)

const zoom = ref(18)
const center = ref([35.7061, 139.7071])

const markers = computed(() => {
  if (!datastore.value) return []
  return settings.mapPaths.flatMap(path => {
    const packet = datastore.value.getBy(path.from, path.id)
                            .at(props.time ?? new Date())?.packet
    if (!packet) return []
    const format = settings.packetFormats[path.id]
    const lat = packet.get(path.lat)?.formatNumber(format.entries[path.lat])
    const lon = packet.get(path.lon)?.formatNumber(format.entries[path.lon])
    if (!lat || !lon) return []
    return [{
      latlon: [lat, lon],
      color: path.markerColor ?? path.color,
    }]
  })
})

const paths = computed(() => {
  if (!datastore.value) return []
  return settings.mapPaths.map(path => {
    const ds = datastore.value.getBy(path.from, path.id)
    const lats = ds.getValues(path.lat)
    const lons = ds.getValues(path.lon)
    return {
      latlons: lats.map((l, i) => [l, lons[i]]),
      name: path.name,
      color: path.color,
    }
  })
})

let initialized = false

watchEffect(() => {
  if (!initialized) {
    if (paths.value[0] && paths.value[0].latlons[0]
        && map.value && map.value.leafletObject) {
      map.value.leafletObject.flyTo(paths.value[0].latlons[0], 18)
      initialized = true
    }
  }
})



</script>
