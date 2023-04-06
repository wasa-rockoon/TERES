<template>
<div class="systemlist">
  <v-app-bar color="primary" height=40 elevation=10>
    <v-app-bar-title>Systems</v-app-bar-title>
  </v-app-bar>
  <v-main>
    <v-container fluid class="d-flex flex-wrap justify-center">
      <v-card width=400 v-for="system in systems" :key="system.id"
              :href="system.client" elevation=6 class="ma-2">
        <v-img :src="system.client + '/image'" height=200></v-img>
        <v-card-title class="text-h5">{{system.name}}</v-card-title>
        <v-card-subtitle>{{system.id}}</v-card-subtitle>
        <v-card-text></v-card-text>
      </v-card>
    </v-container>
    <v-row>
      <v-dialog v-model="dialog" width="auto">
        <template v-slot:activator="{ props }">
          <v-col>
            <v-btn icon="mdi-plus" color="primary" v-bind="props"></v-btn>
          </v-col>
        </template>
        <v-card class="px-10" width=600>
          <v-card-title class="my-5">
            Register New System
          </v-card-title>
          <v-form
            v-model="form"
            @submit.prevent="onRegisterNewSystem"
            >
            <v-text-field v-model="new_system.id"
                          label="ID" :rules="[required]">
            </v-text-field>
            <v-text-field v-model="new_system.name"
                          label="Name" :rules="[required]"></v-text-field>
            <v-text-field v-model="new_system.password"
                          label="Password" type="password" :rules="[required]">
            </v-text-field>
            <v-text-field v-model="new_system.client"
                          label="Client URL"></v-text-field>
            <v-card-actions>
              <v-spacer></v-spacer>
              <v-btn color="primary" @click="dialog = false">Cancel</v-btn>
              <v-btn color="primary" :disabled="!form" type="submit">
                Register
              </v-btn>
            </v-card-actions>
            </v-form>
        </v-card>
      </v-dialog>
 
    </v-row>
  </v-main>
  </div>
</template>

<script setup lang="ts">
import axios from 'axios'
import { onMounted, reactive, ref } from 'vue'
import { api } from '@/library/api'


const systems = ref([])
const dialog = ref(false)
const form = ref(false)
const new_system = reactive({
  id: null,
  name: null,
  password: null,
  client: null,
})


onMounted(async () => {
  systems.value = await api.getSystems()
  console.log('systems', systems.value)
})

const onRegisterNewSystem = async () => {
  if (!form.value) return

  dialog.value = false

  const system = await api.putSystem(new_system)
  systems.value.push(system)
  localStorage.password = new_system.password
  console.log('new system', new_system, system)
}
const required = (v: any) => {
  return !!v || 'Field is required'
}
</script>
