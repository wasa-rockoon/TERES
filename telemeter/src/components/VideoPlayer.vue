<template>
  <video ref="videoElement" :src="video.url" class="w-100">
  </video>

</template>

<script setup lang="ts">
import { ref, defineProps, watch, onMounted } from 'vue'

const props = defineProps(['video', 'time'])

const videoElement = ref(null)

let playing = false

const controlPlaying = () => {
  if (!videoElement.value) return
  let requestT = (props.time.getTime()
                - new Date(props.video.startTime).getTime()) / 1000.0
  if (requestT < 0) requestT = 0
  else if (requestT > videoElement.value.duration)
    requestT = videoElement.value.duration

  const playingT = videoElement.value.currentTime

  if (Math.abs(requestT - playingT) > 0.1) {
    videoElement.value.currentTime = requestT
    if (playing) {
      videoElement.value.pause()
      playing = false
    }
  }
}

onMounted(() => {
  setInterval(controlPlaying, 500)
})

let lastTimeChanged = 0
watch(() => props.time, (cr, prev) => {
  if (!playing) {
    const now = new Date().getTime()
    if (Math.abs((cr.getTime() - prev.getTime()) - (now - lastTimeChanged))
      < 10) {
      let requestT = (props.time.getTime()
                    - new Date(props.video.startTime).getTime()) / 1000.0
      if (0 <= requestT && requestT < videoElement.value.duration) {
        videoElement.value.play()
        playing = true
      }
    }
    lastTimeChanged = now
  }
})


</script>
