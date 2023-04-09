<template>
  <span v-if="datastore">{{datastore.showT(datastore.currentT)}}</span>
	<vue-slider v-model="slider"
              :min="sliderMin" :max="sliderMax" :interval="0.001"
              :process="process"
              :enable-cross="false" :drag-on-click="true"
              :contained="true">
  </vue-slider>
</template>

<script setup lang="ts">
import { reactive, ref, onMounted, inject, computed, Ref, watch } from 'vue';
import VueSlider from 'vue-slider-component'
import 'vue-slider-component/theme/default.css'
import { DataStore } from '../library/datastore'

const datastore = inject<Ref<DataStore>>('datastore')

const slider = ref([0.0, 0.0, 100.0])

const sliderMin = computed<number>(() => (datastore.value?.startT))
const sliderMax = computed<number>(() => datastore.value?.endT
                                         ?? datastore.value?.nowT)

const process = dotPos => {
  if (!datastore.value) return []
  const earliestT = datastore.value.earliestT ?? datastore.value.startT
  const latestT = datastore.value.latestT ?? datastore.value.endT
                 ?? datastore.value.nowT
  const sliderRange = sliderMax.value - sliderMin.value
  return [[earliestT / sliderRange * 100.0, latestT / sliderRange * 100.0]]
}

watch(slider, (cr, prev) => {
  datastore.value.currentT = slider.value[1]
  // console.log(datastore.value.currentTime.getTime())
})

watch(datastore, (cr, prev) => {
  if (!prev) {
    // console.log(prev, sliderMin.value, sliderMax.value)
    slider.value = [sliderMin.value, sliderMax.value, sliderMax.value]
  }
})

</script>>
