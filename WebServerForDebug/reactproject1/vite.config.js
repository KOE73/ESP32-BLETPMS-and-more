import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import path from 'path'; 

// https://vitejs.dev/config/
export default defineConfig({
    plugins: [react()],
    server: {
        host: '127.0.0.1',
        port: 59171,
    },
    build: {
        outDir: 'www', // Out files
        assetsDir: '',   // No /assets
        rollupOptions: {
            output: {
                //entryFileNames: '[name].js',         // JS-файлы без хеширования и вложенных папок
                //chunkFileNames: '[name].js',         // Чанки без вложенных папок
                //assetFileNames: '[name].[ext]',      // Статические файлы (CSS, изображения) без папки
            },
        },
    },
    resolve: {
        alias: {
            '@': path.resolve(__dirname, './src'),              // Алиас для src
            '@components': path.resolve(__dirname, './src/components'), // Алиас для components
            '@UI': path.resolve(__dirname, './src/components/UI'),     // Алиас для UI
            '@layout': path.resolve(__dirname, './src/components/Layout'), // Алиас для Layout
            '@views': path.resolve(__dirname, './src/views'),          // Алиас для pages
        },
    },
})