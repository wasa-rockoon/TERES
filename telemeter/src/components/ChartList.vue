<template>
  <v-card v-for="(chart, i) in charts" :key="i">
    <v-card-item>{{chart.config.title}}</v-card-item>
    <div v-if="chart.data" >
      <Scatter :data="chart.data" :options="chart.options" />
    </div>
  </v-card>
</template>

<script setup lang="ts">
import { computed, inject, Ref } from 'vue'
import {
  Chart as ChartJS,
  Title,
  LinearScale,
  PointElement,
  LineElement,
  Tooltip,
  Legend,
  Colors,
} from 'chart.js'
import { Scatter } from 'vue-chartjs'
import { DataStore } from '../library/datastore'
import * as settings from '../settings'

ChartJS.register(Title, LinearScale, PointElement, LineElement, Tooltip, Legend, Colors)

ChartJS.defaults.color = '#FFFFFF'
ChartJS.defaults.borderColor = '#888888'

const datastore = inject<Ref<DataStore>>('datastore')

const charts = computed(() => {
  if (!datastore.value) return []
  return settings.charts.map(config => {
    return {
      config: config,
      data: {
        datasets: config.y.map(y => {
          const from = y[0]
          const id = y[1]
          const type = y[2]
          const index = y[3] ?? 0
          const format = settings.packetFormats[id]?.entries[type]
          const dataseries = datastore.value.getBy(from, id)
          const data = []
          const times = dataseries.getTimes()
          const values = dataseries.getValues(type, index)
          for (let i = 0; i < times.length; i++) {
            data.push({x: times[i], y: values[i]})
          }
          let name = format?.name
          if (format?.index) name += ` (${format.index[index]})`
          return {
            label: name,
            data: data,
            showLine: true,
            fill: false,
            pointRadius: 0,
            lineTension: 0,
          }
        }),
      },
      options: {
        aspectRatio: 1.5,
        plugins: {
          title: {
            display: false,
            text: config.title,
          },
        },
        scales: {
          x: {
            title: {
              display: true,
              text: 't [s]',
            }
          },
          y: {
            title: {
              display: true,
              text: config.yLabel,
            }
          }
        },

        legend: {
          display: config.y.length > 1
        },
      },
    }
  })
})

</script>
