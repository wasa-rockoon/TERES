<template>
  <div class="w-100">
    <span v-if="datastore">{{datastore.showT(slider[1])}}</span>
    <vue-slider v-model="slider"
                :min="sliderMin" :max="sliderMax" :interval="0.001"
                :process="process" :order="false"
                :enable-cross="true" :drag-on-click="true"
                :contained="true">
    </vue-slider>
  </div>
</template>

<script setup lang="ts">
import { reactive, ref, onMounted, inject, computed, Ref, watch,
         defineEmits, ShallowRef, triggerRef } from 'vue';
import VueSlider from 'vue-slider-component'
import 'vue-slider-component/theme/default.css'
import { DataStore } from '../library/datastore'

const datastore = inject<ShallowRef<DataStore>>('datastore')

const slider = ref([])

const sliderMin = computed<number>(() => (datastore.value?.startT))
const sliderMax = computed<number>(() => datastore.value?.endT
                                         ?? datastore.value?.nowT)

const emit = defineEmits(['change-chart-range', 'change-time'])

const process = dotPos => {
  if (!datastore.value) return []
  const earliestT = datastore.value.earliestT ?? datastore.value.startT
  const latestT = datastore.value.latestT ?? datastore.value.endT
                 ?? datastore.value.nowT
  const sliderRange = sliderMax.value - sliderMin.value
  const fit = (t: number) => {
    const p = (t - sliderMin.value) / sliderRange * 100.0
    if (p < 0) return 0
    else if (t > 100) return 100
    else return p
  }
  return [[fit(earliestT), fit(latestT)]]
}

watch(slider, (cr, prev) => {
  if (cr[1] != prev[1])
    emit('change-time', datastore.value.t2time(slider.value[1]))
  if (cr[0] != prev[0] || cr[2] != prev[2])
    emit('change-chart-range', {min: cr[0], max: cr[2]})
})

watch(datastore, (cr, prev) => {
  if (!prev) {
    // console.log(prev, sliderMin.value, sliderMax.value)
    slider.value = [sliderMin.value, sliderMax.value, sliderMax.value]
  }
})

</script>>
