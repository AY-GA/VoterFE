<script lang="ts">
  import { onMount, setContext } from 'svelte';
  import { get, writable, type Writable } from 'svelte/store';
  import { page } from '$app/stores';
  import {
    Activity,
    BrainCircuit,
    CalendarPlus,
    CirclePlay,
    Gauge,
    Landmark,
    Newspaper,
    RefreshCw,
    RotateCcw,
    Users,
    Vote
  } from 'lucide-svelte';
  import '../styles.css';
  import { api, loadApiSettings, saveApiSettings } from '$lib/api';
  import { SIMULATION_CONTEXT } from '$lib/simulation-context';
  import type { SimulationContext } from '$lib/simulation-context';
  import type { ApiMessage, ElectionType, SimulationControls } from '$lib/types';

  type Route = {
    path: string;
    label: string;
    icon: typeof Gauge;
  };

  const routes: Route[] = [
    { path: '/', label: 'Overview', icon: Gauge },
    { path: '/population', label: 'Districts', icon: Users },
    { path: '/politics', label: 'Polling', icon: Landmark },
    { path: '/media', label: 'Media', icon: Newspaper },
    { path: '/model', label: 'Voters', icon: BrainCircuit }
  ];

  const settings = writable(loadApiSettings());
  const controls = writable<SimulationControls>({
    days: 7,
    electionType: 'parliament'
  });
  const version = writable<SimulationContext['version'] extends Writable<infer T> ? T : never>(null);
  const map = writable<SimulationContext['map'] extends Writable<infer T> ? T : never>(null);
  const poll = writable<SimulationContext['poll'] extends Writable<infer T> ? T : never>(null);
  const loading = writable(true);
  const running = writable(false);
  const message = writable<ApiMessage | null>(null);

  $: currentPath = normalizePath($page.url.pathname);

  function normalizePath(path: string) {
    if (!path || path === '/index.html') return '/';
    return path.endsWith('/') && path.length > 1 ? path.slice(0, -1) : path;
  }

  function inputValue(event: Event) {
    return (event.currentTarget as HTMLInputElement).value;
  }

  function numberInputValue(event: Event) {
    return Number((event.currentTarget as HTMLInputElement).value);
  }

  function electionInputValue(event: Event) {
    return (event.currentTarget as HTMLSelectElement).value as ElectionType;
  }

  function updateSettings(patch: Partial<SimulationContext['settings'] extends Writable<infer T> ? T : never>) {
    settings.update((current) => {
      const next = { ...current, ...patch };
      try {
        saveApiSettings(next);
      } catch {
        // ignore localStorage errors
      }
      return next;
    });
  }

  function updateControls(patch: Partial<SimulationControls>) {
    controls.update((current) => ({ ...current, ...patch }));
  }

  function showMessage(tone: ApiMessage['tone'], text: string) {
    message.set({ tone, text });
  }


  // Auto-load backend state when connection settings change (debounced).
  let settingsTimer: ReturnType<typeof setTimeout> | null = null;
  let lastBaseUrl = (get(settings).baseUrl ?? '').trim();
  let lastToken = get(settings).token.trim();

  settings.subscribe((current) => {
    const base = (current.baseUrl ?? '').trim();
    const token = current.token.trim();
    if (base === lastBaseUrl && token === lastToken) return;
    lastBaseUrl = base;
    lastToken = token;
    if (settingsTimer) clearTimeout(settingsTimer);
    settingsTimer = setTimeout(() => {
      void loadBackendState();
    }, 500);
  });

  async function loadBackendState() {
    loading.set(true);
    message.set(null);

    const currentSettings = get(settings);

    try {
      let versionInfo: Awaited<ReturnType<typeof api.getVersion>> | null = null;
      try {
        versionInfo = await api.getVersion(currentSettings);
        version.set(versionInfo);
      } catch {
        version.set(null);
      }

      if (!currentSettings.token.trim()) {
        map.set(null);
        poll.set(null);
        if (versionInfo) {
          showMessage('info', 'Connected to backend. Paste the simulation token from the backend terminal.');
        }
        return;
      }

      try {
        map.set(await api.getMap(currentSettings));
      } catch (mapErr) {
        map.set(null);
        showMessage('error', mapErr instanceof Error ? mapErr.message : 'Map failed.');
      }

      try {
        poll.set(await api.getLastPoll(currentSettings));
      } catch {
        poll.set(null);
      }
    } catch (error) {
      version.set(null);
      map.set(null);
      poll.set(null);
      showMessage('error', error instanceof Error ? error.message : 'Could not connect to API.');
    } finally {
      loading.set(false);
    }
  }

  async function runAction(action: () => Promise<void>, success: string, reload = true) {
    running.set(true);
    message.set(null);

    try {
      await action();
      if (reload) {
        await loadBackendState();
      }
      showMessage('success', success);
    } catch (error) {
      showMessage('error', error instanceof Error ? error.message : 'API action failed.');
    } finally {
      running.set(false);
    }
  }

  async function incrementSimulation() {
    const currentControls = get(controls);
    await runAction(
      () => api.increment(get(settings), currentControls.days),
      `Simulation advanced by ${currentControls.days} days.`
    );
  }

  async function resetSimulation() {
    await runAction(() => api.reset(get(settings)), 'Simulation reset. Districts were regenerated.');
  }

  async function runPoll() {
    running.set(true);
    message.set(null);

    try {
      const nextPoll = await api.runPoll(get(settings));
      poll.set(nextPoll);
      const resultCount = nextPoll.national.length + nextPoll.districts.length;
      showMessage(
        resultCount ? 'success' : 'info',
        resultCount ? 'Polling finished.' : 'Polling finished, but the backend returned no result rows.'
      );
    } catch (error) {
      showMessage('error', error instanceof Error ? error.message : 'Polling failed.');
    } finally {
      running.set(false);
    }
  }

  async function snapElection() {
    await runAction(
      () => api.snapElection(get(settings), get(controls).electionType),
      `${get(controls).electionType} election called.`
    );
  }

  async function addMedia(name: string) {
    await runAction(() => api.addMedia(get(settings), name), `Media provider "${name}" added.`, false);
  }

  async function addLaw(text: string, force: boolean) {
    await runAction(() => api.addLaw(get(settings), text, force), force ? 'Law forced.' : 'Law proposed.', false);
  }

  async function repealLaw(lawUuid: string) {
    await runAction(() => api.repealLaw(get(settings), lawUuid), 'Law repealed.', false);
  }

  setContext<SimulationContext>(SIMULATION_CONTEXT, {
    settings,
    controls,
    version,
    map,
    poll,
    loading,
    running,
    message,
    updateSettings,
    updateControls,
    loadBackendState,
    incrementSimulation,
    resetSimulation,
    runPoll,
    snapElection,
    addMedia,
    addLaw,
    repealLaw
  });

  onMount(() => {
    void loadBackendState();
  });
</script>

<svelte:head>
  <title>VoterNN</title>
</svelte:head>

<main class="app-shell">
  <aside class="sidebar" aria-label="Application navigation">
    <a class="brand-lockup" href="/">
      <div class="brand-mark" aria-hidden="true">
        <BrainCircuit size={24} />
      </div>
      <div>
        <strong>VoterNN</strong>
        <span>simulation console</span>
      </div>
    </a>

    <nav class="nav-list">
      {#each routes as route}
        <a
          class:active={currentPath === route.path}
          href={route.path}
          aria-current={currentPath === route.path ? 'page' : undefined}
          aria-label={route.label}
          title={route.label}
        >
          <svelte:component this={route.icon} size={19} />
          <span>{route.label}</span>
        </a>
      {/each}
    </nav>

    <div class="sidebar-status">
      <span>API</span>
      <strong>{$version ? `${$version.major}.${$version.minor}` : '--'}</strong>
      <small>{$settings.token ? 'token set' : 'token missing'}</small>
    </div>
  </aside>

  <section class="workspace">
    <header class="topbar">
      <div>
        <p class="eyebrow">VoterNN reference API</p>
        <h1>Simulation control panel</h1>
      </div>
      <div class="topbar-actions">
        <button class="icon-button" type="button" title="Reload API state" on:click={loadBackendState}>
          <RefreshCw size={19} />
        </button>
        <button class="primary-action" type="button" on:click={incrementSimulation} disabled={$running}>
          <CirclePlay size={19} />
          <span>Increment</span>
        </button>
      </div>
    </header>

    <section class="api-settings" aria-label="API connection settings">
      <label>
        <span>Backend URL</span>
        <input
          value={$settings.baseUrl}
          placeholder="http://127.0.0.1:8080"
          on:input={(event) => updateSettings({ baseUrl: inputValue(event) })}
        />
      </label>
      <label>
        <span>Simulation token</span>
        <input
          value={$settings.token}
          placeholder="token from backend UI"
          //type="password"
          on:input={(event) => updateSettings({ token: inputValue(event) })}
        />
      </label>
      <!-- Settings are auto-saved and loaded when changed in dev; no explicit save button. -->
    </section>

    {#if $message}
      <div class={`alert ${$message.tone}`} role="status">{$message.text}</div>
    {/if}

    {#if $loading}
      <div class="loading-panel">
        <Activity size={24} />
        <span>Loading VoterNN API state...</span>
      </div>
    {:else}
      <section class="control-strip" aria-label="Simulation actions">
        <label>
          <span>Days</span>
          <input
            value={$controls.days}
            min="1"
            max="365"
            type="number"
            on:input={(event) => updateControls({ days: numberInputValue(event) })}
          />
        </label>

        <label>
          <span>Election type</span>
          <select
            value={$controls.electionType}
            on:change={(event) => updateControls({ electionType: electionInputValue(event) })}
          >
            <option value="parliament">parliament</option>
            <option value="senate">senate</option>
            <option value="president">president</option>
            <option value="general">general</option>
            <option value="all">all</option>
          </select>
        </label>

        <button class="secondary-action" type="button" on:click={runPoll} disabled={$running}>
          <Vote size={18} />
          <span>Run poll</span>
        </button>

        <button class="secondary-action" type="button" on:click={snapElection} disabled={$running}>
          <CalendarPlus size={18} />
          <span>Snap election</span>
        </button>

        <button class="danger-action" type="button" on:click={resetSimulation} disabled={$running}>
          <RotateCcw size={18} />
          <span>Reset</span>
        </button>
      </section>

      <slot />
    {/if}
  </section>
</main>
