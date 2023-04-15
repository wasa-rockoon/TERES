<template>
	<v-dialog v-model="dialog" width="auto" class="overflow-visible">
    <template v-slot:activator="{ props }">
      <v-btn v-bind="props" size="x-large" class="text-capitalize">
        {{p.flight?.system?.name}} - {{p.flight?.name}}
      </v-btn>
    </template>

    <v-card class="px-10" width=600 min-height=800>
      <v-card-title class="my-5">
        Settings
      </v-card-title>
      <v-form v-model="form" @submit.prevent="onSaveSetting">
        <v-text-field v-model="name" label="Name"
                      :rules="[required]">
        </v-text-field>
        <div class="my-5">
          Start time
          <VueDatePicker v-model="startTime" enable-seconds dark>
          </VueDatePicker>
        </div>
        <div class="my-5">
          Launch time
          <VueDatePicker v-model="launchTime" enable-seconds dark>
          </VueDatePicker>
        </div>
        <div class="my-5">
          End time
          <VueDatePicker v-model="endTime" enable-seconds dark>
          </VueDatePicker>
        </div>

        <div class="my-5">
          Video 1
          <v-text-field v-model="videos[0].url" label="URL"></v-text-field>
          <VueDatePicker v-model="videos[0].startTime" enable-seconds dark>
          </VueDatePicker>
        </div>

        <div class="my-5">
          Video 2
          <v-text-field v-model="videos[1].url" label="URL"></v-text-field>
          <VueDatePicker v-model="videos[1].startTime" enable-seconds dark>
          </VueDatePicker>
        </div>

        <v-card-actions>
          <v-spacer></v-spacer>
          <v-btn color="primary" type="submit">
            Save
          </v-btn>
        </v-card-actions>
      </v-form>
    </v-card>
  </v-dialog>
</template>>

<script setup lang="ts">
import { ref, defineProps, defineEmits, watch } from 'vue';
import VueDatePicker from '@vuepic/vue-datepicker';
import { api } from '../library/api'

const dialog = ref(false)
const form = ref(false)
const name = ref('')
const startTime = ref(new Date())
const launchTime = ref(null)
const endTime = ref(null)
const videos = ref([{}, {}])


const p = defineProps(['flight'])

const emit = defineEmits(['update'])

const onSaveSetting = async () => {
  console.log('save', videos.value)
  const flight_ = await api.putFlight(
    p.flight.id, name.value,
    startTime.value, launchTime.value, endTime.value,
    { videos: videos.value }
  )
  console.log('save', flight_)
  emit('update', flight_)
}

watch(p, (cr, prev) => {
  name.value = cr.flight?.name
  startTime.value = cr.flight.startTime
  launchTime.value = cr.flight.launchTime
  endTime.value = cr.flight.endTime
  videos.value = cr.flight.data.videos ?? [{}, {}]
})

const required = (v: any) => {
  return !!v || 'Field is required'
}



</script>
