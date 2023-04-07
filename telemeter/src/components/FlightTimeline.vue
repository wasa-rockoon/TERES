<template>
  {{timeText}}
	<vue-slider v-model="slider"
              :min="sliderMin" :max="sliderMax" :interval="0.001"
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

const timeText = computed<number>(() => {
  return `T${datastore.value?.currentT > 0 ? '+' : ''}`
       + `${datastore.value?.currentT?.toFixed(3)}`
})


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
