<script lang="ts">
  import { BrainCircuit, Search, UserPlus } from 'lucide-svelte';
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
    'tradition_progress'
  ] as const;

  let voterUuid = '';
  let voter: VoterRecord | null = null;
  let message = '';
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

  async function getVoter() {
    message = '';
    try {
      voter = await api.getVoter(settings, voterUuid);
    } catch (error) {
      message = error instanceof Error ? error.message : 'Could not load voter.';
    }
  }

  async function createVoter() {
    message = '';
    try {
      await api.createVoter(settings, form);
      message = 'Voter created.';
    } catch (error) {
      message = error instanceof Error ? error.message : 'Could not create voter.';
    }
  }
</script>

<svelte:head>
  <title>Voters | VoterNN</title>
</svelte:head>

<section class="model-layout">
  <article class="wide-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">voter</p>
        <h2>Lookup voter NN positions</h2>
      </div>
      <BrainCircuit size={22} />
    </div>

    <label>
      <span>Voter UUID</span>
      <input bind:value={voterUuid} placeholder="voter_uuid" />
    </label>
    <button
      class="primary-action action-block"
      type="button"
      disabled={running || !voterUuid.trim()}
      on:click={getVoter}
    >
      <Search size={18} />
      <span>Load voter</span>
    </button>

    {#if voter}
      <div class="health-strip">
        <div>
          <span>Name</span>
          <strong>{voter.name} {voter.surname}</strong>
        </div>
        <div>
          <span>Age</span>
          <strong>{voter.age}</strong>
        </div>
        <div>
          <span>Future outlook</span>
          <strong>{voter.future_outlook}</strong>
        </div>
      </div>
      <div class="issue-table">
        {#each opinionKeys as key}
          <div class="issue-row two-column">
            <strong>{key}</strong>
            <span style={`--bar:${Math.abs(Number(voter[key]))}%`}>{voter[key]}</span>
          </div>
        {/each}
      </div>
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

    <button class="primary-action action-block" type="button" disabled={running} on:click={createVoter}>
      <UserPlus size={18} />
      <span>Create voter</span>
    </button>

    {#if message}
      <p class="delta-copy">{message}</p>
    {/if}
  </article>
</section>
