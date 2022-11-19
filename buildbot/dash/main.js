import {createApp} from 'vue'
import App from './App.vue'

window._create_app_ = function () {
  createApp(App).mount('#force-dash-app')
}

window._create_app_()
