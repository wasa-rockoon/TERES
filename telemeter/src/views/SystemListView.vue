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
import axios from 'axios';
import { onMounted, reactive, ref } from 'vue';


const systems = ref([])
const dialog = ref(false)
const form = ref(false)
const new_system = reactive({
  id: null,
  name: null,
  password: null,
  client: null,
})


onMounted(() => {
  axios.get('systems/')
    .then(response => {
      systems.value = response.data.systems
      console.log(response.data.systems)
    })
})

const onRegisterNewSystem = () => {
  if (!form.value) return

  console.log(new_system)
  dialog.value = false

  axios.put(`systems/${new_system.id}`, null, {
    params: {
      name: new_system.name,
      password: new_system.password,
      client: new_system.client || ''
    }
  })
    .then(response => {
      systems.value.push(response.data)
      localStorage.password = new_system.password
      console.log(response.data)
    })
}
const required = (v: any) => {
  return !!v || 'Field is required'
}
</script>
