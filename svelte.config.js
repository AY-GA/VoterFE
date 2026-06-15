import adapter from '@sveltejs/adapter-auto';
import { vitePreprocess } from '@sveltejs/vite-plugin-svelte';

/** @type {import('@sveltejs/kit').Config} */
const config = {
  preprocess: vitePreprocess(),
  kit: {
    adapter: adapter(),
    // Expose Vite dev-server options here so SvelteKit's dev server
    // sets CORS headers and uses the same proxy as vite.config.ts.
    // This helps avoid browser CORS errors when the app is configured
    // to call the backend via relative paths during development.
    vite: {
      server: {
        cors: true,
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
    }
  }
};

export default config;
