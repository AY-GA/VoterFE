<script lang="ts">
  import { onMount } from 'svelte';
  import { BrainCircuit, RefreshCw, Search, UserPlus, Users } from 'lucide-svelte';
  import type { ApiSettings, VoterInput, VoterRecord } from '../lib/types';
  import { api } from '../lib/api';

  export let settings: ApiSettings;
  export let running: boolean;

  const opinionKeys = [
    'security',
    'risk_aversion',
    'public_spending',
    'cost_of_living',
    'minorities_support',
    'stability',
    'job_security',
    'future_outlook',
    'equality_markets',
    'nation_globe',
    'liberty_authority',
    // 'tradition_progress' removed from UI
  ] as const;

  let voterUuid = '';
  let voter: VoterRecord | null = null;
  let message = '';
  let search = '';
  let voters: VoterRecord[] = [];
  let voterTotal = 0;
  let loadingVoters = false;
  let lastListKey = '';
  let mounted = false;
  let creatingVoter = false;
  let form: VoterInput = {
    name: '',
    middlename: '',
    surname: '',
    age: 30,
    security: 0,
    risk_aversion: 0,
    public_spending: 0,
    cost_of_living: 0,
    minorities_support: 0,
    stability: 0,
    job_security: 0,
    future_outlook: 0,
    equality_markets: 0,
    nation_globe: 0,
    liberty_authority: 0,
    tradition_progress: 0
  };

  $: listKey = `${settings.baseUrl}|${settings.token}`;
  $: if (mounted && listKey !== lastListKey) {
    lastListKey = listKey;
    void listVoters();
  }

  function fullName(row: VoterRecord) {
    return [row.name, row.middlename, row.surname].filter(Boolean).join(' ') || 'Unnamed voter';
  }

  async function listVoters() {
    if (!settings.token.trim()) {
      voters = [];
      voterTotal = 0;
      return;
    }

    loadingVoters = true;
    message = '';
    try {
      const response = await api.listVoters(settings, search, 50);
      voters = response.voters;
      voterTotal = response.total ?? response.voters.length;
    } catch (error) {
      voters = [];
      voterTotal = 0;
      setMessage(error instanceof Error ? error.message : 'Could not load voters.');
    } finally {
      loadingVoters = false;
    }
  }

  async function getVoter() {
    try {
      voter = await api.getVoter(settings, voterUuid);
      voterUuid = voter.voter_uuid ?? voterUuid;
      setMessage('Voter loaded.');
    } catch (error) {
      setMessage(error instanceof Error ? error.message : 'Could not load voter.');
    }
  }

  async function selectVoter(row: VoterRecord) {
    const id = row.voter_uuid ?? '';
    if (voter && id && id === voter.voter_uuid) {
      unselectVoter();
      return;
    }
    voterUuid = id;
    voter = row;
    if (voterUuid) {
      await getVoter();
    }
  }

  function unselectVoter() {
    voter = null;
    voterUuid = '';
    setMessage('Voter unselected.');
  }

  async function createVoter() {
    creatingVoter = true;
    try {
      await api.createVoter(settings, form);
      setMessage('Voter created.');
      await listVoters();
    } catch (error) {
      setMessage(error instanceof Error ? error.message : 'Could not create voter.');
    } finally {
      // keep the button disabled briefly to avoid duplicate submissions
      setTimeout(() => {
        creatingVoter = false;
      }, 1000);
    }
  }

  function sentenceCase(text: string) {
    if (!text || text.length === 0) return text;
    return text.charAt(0).toUpperCase() + text.slice(1);
  }

  function setMessage(text: string) {
    message = sentenceCase(text);
  }

  function barStyle(value: unknown) {
    const raw = Number(value ?? 0);
    const absRaw = Math.abs(raw);
    // Support values in -1..1 or -100..100 ranges
    const percent = Math.min(100, absRaw > 1 ? absRaw : absRaw * 100);
    const color = raw < 0 ? 'rgba(235,87,87,0.35)' : 'rgba(31,102,229,0.25)';
    const bg = raw < 0 ? '#ffecec' : '#f5f8fc';
    return `--bar:${percent}%;--bar-color:${color};--bar-bg:${bg}`;
  }

  onMount(() => {
    mounted = true;
    lastListKey = listKey;
    void listVoters();
  });
</script>

<svelte:head>
  <title>Voters | VoterNN</title>
</svelte:head>

<section class="model-layout">
  <article class="wide-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">voter</p>
        <h2>Lookup voters</h2>
      </div>
      <BrainCircuit size={22} />
    </div>

    <div class="lookup-bar">
      <label>
        <span>Name or UUID</span>
        <input bind:value={search} placeholder="Search voters" on:keydown={(event) => event.key === 'Enter' && listVoters()} />
      </label>
      <button class="secondary-action" type="button" disabled={running || loadingVoters} on:click={listVoters}>
        <Search size={18} />
        <span>Search</span>
      </button>
      <button class="icon-button" type="button" title="Refresh voters" disabled={running || loadingVoters} on:click={listVoters}>
        <RefreshCw size={18} />
      </button>
    </div>

    <div class="voter-browser">
      <div class="voter-list" aria-label="Voters">
        <div class="list-summary">
          <Users size={18} />
          <span>{loadingVoters ? 'Loading voters...' : `${voters.length} shown${voterTotal > voters.length ? ` of ${voterTotal}` : ''}`}</span>
        </div>

        {#if voters.length}
          {#each voters as row}
            <button
              class:active={!!row.voter_uuid && row.voter_uuid === voter?.voter_uuid}
              type="button"
              on:click={() => selectVoter(row)}
            >
              <strong>{fullName(row)}</strong>
              <span>{row.voter_uuid}</span>
            </button>
          {/each}
        {:else}
          <p class="delta-copy">No voters found.</p>
        {/if}
      </div>

      <div class="voter-detail">
        <label>
          <span>UUID</span>
          <input value={voterUuid} readonly placeholder="voter_uuid" />
        </label>

        {#if voter}
          <div class="health-strip">
            <div>
              <span>Name</span>
              <strong>{fullName(voter)}</strong>
            </div>
            <div>
              <span>Age</span>
              <strong>{Number(voter.age).toFixed(0)}</strong>
            </div>
            <div>
              <span>Future outlook</span>
              <strong>{Number(voter.future_outlook).toFixed(2)}</strong>
            </div>
          </div>
          <div class="issue-table">
            {#each opinionKeys as key}
              <div class="issue-row two-column">
                <strong>{key}</strong>
                <span style={barStyle(voter[key])}>{Number(voter[key]).toFixed(2)}</span>
              </div>
            {/each}
          </div>
        {:else}
          <p class="delta-copy">Select a voter to inspect opinion positions.</p>
        {/if}
      </div>
    </div>

    {#if message}
      <p class="delta-copy">{message}</p>
    {/if}
  </article>

  <article class="wide-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">voter</p>
        <h2>Create synthetic voter</h2>
      </div>
      <UserPlus size={22} />
    </div>

    <div class="training-grid">
      <label>
        <span>Name</span>
        <input bind:value={form.name} />
      </label>
      <label>
        <span>Middle name</span>
        <input bind:value={form.middlename} />
      </label>
      <label>
        <span>Surname</span>
        <input bind:value={form.surname} />
      </label>
      <label>
        <span>Age</span>
        <input bind:value={form.age} min="18" max="120" type="number" />
      </label>
    </div>

    <div class="training-grid">
      {#each opinionKeys as key}
        <label class="range-field">
          <span>{key}: {form[key]}</span>
          <input bind:value={form[key]} min="-100" max="100" type="range" />
        </label>
      {/each}
    </div>

    <button class="primary-action action-block" type="button" disabled={running || creatingVoter} on:click={createVoter}>
      <UserPlus size={18} />
      <span>Create voter</span>
    </button>
  </article>
</section>
