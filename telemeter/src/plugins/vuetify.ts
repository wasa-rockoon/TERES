// Styles
import '@mdi/font/css/materialdesignicons.css'
import 'vuetify/styles'

// Vuetify
import { createVuetify } from 'vuetify'

const customTheme = {
  dark: true,
  colors: {
    primary: '#2196F3',
    background: '#121212',
    surface: '#303030',
    'primary-darken-1': '#00FF00',
    secondary: '#0000FF',
    'secondary-darken-1': '#FF0000',
    error: '#F44336',
    info: '#009688',
    success: '#FF0000',
    warning: '#FF0000',

  }
}

export default createVuetify({
  theme: {
    defaultTheme: 'customTheme',
    themes: {
      customTheme,
    }
  },
  // icons: {
  //   iconfont: 'mdi'
  // }
})
