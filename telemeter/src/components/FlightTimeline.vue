<template>
  <v-container id="timeline" class="w-screen ma-0 pa-0">
    <v-row align="center">
      <v-col id="timeline-t">
        <span v-if="datastore">
          {{datastore.showT(currentT)}}
        </span>
      </v-col>
      <v-col align-self="start">
        <vue-slider id="timeline-slider" v-model="slider" ref="sliderRef"
                    :min="sliderMin" :max="sliderMax" :interval="0.001"
                    :process="process" :marks="marks" :dot-options="dotOptions"
                    :order="false" :enable-cross="true" :drag-on-click="true"
                    :tooltip-placement="'top'" :tooltip-formatter="formatter"
                    :dot-size="[10,10]" :duration="0"
                    :contained="true">
        </vue-slider>
      </v-col>
      <v-col id="timeline-control">
        <v-btn-toggle v-model="toggle" divided variant="outlined" color="primary">
          <v-btn icon="mdi-skip-backward"></v-btn>
          <v-btn v-if="!playing" icon="mdi-play"></v-btn>
          <v-btn v-if="playing" icon="mdi-pause"></v-btn>
          <v-btn icon="mdi-skip-forward"></v-btn>
        </v-btn-toggle>
      </v-col>
    </v-row>
  </v-container>
</template>

<script setup lang="ts">
import { reactive, ref, onMounted, inject, computed, Ref, watch,
         defineEmits, ShallowRef, triggerRef } from 'vue';
import VueSlider from 'vue-slider-component'
import 'vue-slider-component/theme/default.css'
import { DataStore } from '../library/datastore'

const datastore = inject<ShallowRef<DataStore>>('datastore')

const playing = ref(false)

const toggle = ref(null)

const sliderRef = ref(null)

const slider = ref([])
const currentT = computed(() => slider.value[1])

const sliderMin = computed<number>(() => (datastore.value?.startT))
const sliderMax = computed<number>(() => datastore.value?.endT
                                         ?? datastore.value?.nowT)

const emit = defineEmits(['change-chart-range', 'change-time'])

const dotOptions = [{ tooltip: 'active' },
                    { tooltip: 'none' },
                    { tooltip: 'active' }, ]

const marks = computed(() => {
  const m = {}
  if (datastore.value) {
    m[currentT.value] = {
      label: '',
      style: {
        width: '18px',
        height: '18px',
        display: 'block',
        backgroundColor: '#2196F3',
        transform: 'translate(-7px, -7px)'
      },

    }
    m[Math.round(datastore.value.launchT).toString()] = {
      label: 'Launch',
      style: {
        width: '8px',
        height: '8px',
        display: 'block',
        backgroundColor: '#303030',
        transform: 'translate(-2px, -2px)'
      },
    }
  }
  return m
})

const process = (dotPos: number) => {
  if (!datastore.value) return []
  const earliestT = datastore.value.earliestT ?? datastore.value.startT
  const latestT = datastore.value.latestT ?? datastore.value.endT
                 ?? datastore.value.nowT
  return [[sliderPercent(earliestT), sliderPercent(latestT),
           { backgroundColor: '#2196F3' }]]
}

const formatter = (t: number) => {
  if (t == Math.min(slider.value[0], slider.value[2]))
    return `Chart min: ${datastore.value?.showT(t)}`
  else if (t == Math.max(slider.value[0], slider.value[2]))
    return `Chart max: ${datastore.value?.showT(t)}`
  else return ''
}

let prevTimestamp = null
let liveT = null

const liveTimeline = (timestamp) => {
  if (prevTimestamp) {
    if (playing.value) {
      liveT = slider.value[1] + (timestamp - prevTimestamp) / 1000.0
      if (liveT > sliderMax.value) liveT = sliderMax.value
      sliderRef.value.setValue([slider.value[0], liveT, slider.value[2]])
    }
  }
  prevTimestamp = timestamp

  requestAnimationFrame(liveTimeline)
}

onMounted(() => {
  requestAnimationFrame(liveTimeline)
})

watch(slider, (cr, prev) => {

  if (cr[1] != prev[1]) {
    if (cr[1] != liveT) {
      // playing.value = false
      // toggle.value = null
    }
    emit('change-time', datastore.value.t2time(currentT.value))
  }

  if (cr[0] != prev[0] || cr[2] != prev[2])
    emit('change-chart-range',
        {min: Math.min(cr[0], cr[2]), max: Math.max(cr[0], cr[2])})
})

watch(toggle, (cr: number, prev: number) => {
  if (cr == undefined) {
    playing.value = false
  }
  else if (cr == 0) {
    playing.value = false
    toggle.value = null
    sliderRef.value.setValue(
      [slider.value[0], datastore.value.startT, slider.value[2]])
  }
  else if (cr == 1 && playing.value) {
    playing.value = false
    toggle.value = null
  }
  else if (cr == 1) playing.value = true
  else if (cr == 2) {
    sliderRef.value.setValue(
      [slider.value[0], datastore.value.endT ?? datastore.value.nowT,
       slider.value[2]])
    playing.value = true
  }
})


watch(datastore, (cr, prev) => {
  if (!prev) {
    // console.log(prev, sliderMin.value, sliderMax.value)
    slider.value = [
      sliderMin.value,
      datastore.value.endTime ? 0 : sliderMax.value,
      sliderMax.value]
  }
})

const sliderPercent = (t: number) => {
  const sliderRange = sliderMax.value - sliderMin.value
  const p = (t - sliderMin.value) / sliderRange * 100.0
  if (p < 0) return 0
  else if (p > 100) return 100
  else return p
}

</script>>


<style>

#timeline {
  max-width: 100vw;
}

#timeline-t {
  min-width: 150px;
  max-width: 200px;
  font-family: monospace;
  font-size: 20px;
}
#timeline-control {
  min-width: 150px;
  max-width: 200px;
}
#timeline-slider {
  min-width: calc(100vw - 400px);
}

#timeline .vue-slider {
  margin-top: 8px;
}

.vue-slider-dot {
  /* transition: none!important; */
}

</style>
