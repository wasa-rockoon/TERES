<template>
  <v-dialog v-model="dialog" width="auto">
    <template v-slot:activator="{ props }">
      <v-btn icon="mdi-lock" v-if="!(p.system?.admin)"
             v-bind="props">
      </v-btn>
      <v-btn icon="mdi-check" v-if="p.system?.admin"
             v-bind="props">
      </v-btn>
    </template>
    <v-card class="px-10" width=600>
      <v-card-title class="my-5">
        Login with Password
      </v-card-title>
      <v-form v-model="form" @submit.prevent="onLogin">
        <v-text-field v-model="password"
                      label="Password" type="password" :rules="[required]">
        </v-text-field>
        <v-card-actions>
          <v-spacer></v-spacer>
          <v-btn color="primary" :disabled="!form" type="submit">
            Login
          </v-btn>
        </v-card-actions>
      </v-form>
    </v-card>
  </v-dialog>

</template>>


<script setup lang="ts">
import { ref, defineProps, defineEmits } from 'vue';
import { api } from '../library/api'

const dialog = ref(false)
const form = ref(false)
const password = ref('')

const p = defineProps(['system'])

const emit = defineEmits(['login'])


const onLogin = async () => {
  if (!form.value) return

  localStorage.password = password.value
  dialog.value = false

  const system = await api.getSystem(p.system.id)
  emit('login', system)
}

const required = (v: any) => {
  return !!v || 'Password is required'
}



</script>
