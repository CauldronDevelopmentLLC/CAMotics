import {fileURLToPath, URL} from 'url'
import {defineConfig} from 'vite'
import vue from '@vitejs/plugin-vue'

export default defineConfig({
  plugins: [vue()],
  root: 'dash',
  build: {outDir: '../dist'},
  base: '/wsgi_dashboards/force/',
  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./dash', import.meta.url))
    }
  }
})
