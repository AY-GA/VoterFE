import { sveltekit } from '@sveltejs/kit/vite';
import { defineConfig } from 'vite';

export default defineConfig({
  plugins: [sveltekit()],
  server: {
    proxy: {
      '/version': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/map': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/increment': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/reset': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/snap': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/polling': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/voter': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/party': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/media': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/law': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/district': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/company': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/product': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/statistics': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/crisis': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false },
      '/constitution': { target: 'http://127.0.0.1:8080', changeOrigin: true, secure: false }
    }
  }
});
