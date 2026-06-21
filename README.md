# VoterNN Simulation UI

A small SvelteKit control panel for the VoterNN society simulation API. The app lets you connect to a backend, load simulation state, advance time, run polls, trigger elections, manage media, and inspect voter/model data.

## Tech Stack

- SvelteKit
- Svelte 5
- Vite
- TypeScript
- lucide-svelte icons

## Getting Started

Install dependencies:

```bash
npm install
```

Start the development server:

```bash
npm run dev
```

Build for production:

```bash
npm run build
```

Preview a production build:

```bash
npm run preview
```

Run type checks and a production build:

```bash
npm run validate
```

## API Configuration

The UI defaults to `http://127.0.0.1:8080` for the backend. You can set defaults with environment variables:

```bash
VITE_API_BASE_URL=http://127.0.0.1:8080
VITE_API_TOKEN=your-simulation-token
```

The backend URL and token can also be saved from the app UI; those settings are stored in browser local storage.

## Project Structure

- `src/routes/` - SvelteKit routes and application shell
- `src/lib/api.ts` - API client and local settings helpers
- `src/lib/simulation-context.ts` - shared simulation state for route pages
- `src/lib/types.ts` - shared TypeScript types
- `src/pages/` - dashboard and feature pages
- `src/styles.css` - global styling
