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
        info: '#29b6f6',
        success: '#66bb6a',
        error: '#f44336',
        warning: '#ffa726',

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
