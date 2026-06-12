<script lang="ts">
  import { BarChart3, CirclePlay, Vote } from 'lucide-svelte';
  import type { ElectionType, PollResponse } from '../lib/types';

  export let poll: PollResponse | null;
  export let running: boolean;
  export let electionType: ElectionType;
  export let runPoll: () => Promise<void>;
  export let snapElection: () => Promise<void>;

  $: national = [...(poll?.national ?? [])].sort((a, b) => b.percentage - a.percentage);
</script>

<svelte:head>
  <title>Polling | VoterNN</title>
</svelte:head>

<section class="politics-layout">
  <article class="wide-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">polling</p>
        <h2>National results</h2>
      </div>
      <BarChart3 size={22} />
    </div>

    <button class="primary-action" type="button" on:click={runPoll} disabled={running}>
      <CirclePlay size={18} />
      <span>Run poll</span>
    </button>

    <div class="party-list padded-list">
      {#each national as result}
        <article class="party-row" style={`--party:hsl(${result.party_uuid.length * 23} 65% 46%)`}>
          <div>
            <strong>{result.party_uuid}</strong>
            <span>party UUID</span>
          </div>
          <div class="party-meter">
            <span>{result.percentage.toFixed(1)} %</span>
            <progress value={result.percentage} max="100"></progress>
          </div>
          <b>{Math.round(result.percentage)}</b>
        </article>
      {/each}
    </div>
  </article>

  <article class="control-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">snap</p>
        <h2>Call elections</h2>
      </div>
      <Vote size={22} />
    </div>

    <label>
      <span>Election type</span>
      <select bind:value={electionType}>
        <option value="parliament">parliament</option>
        <option value="senate">senate</option>
        <option value="president">president</option>
        <option value="general">general</option>
        <option value="all">all</option>
      </select>
    </label>

    <button class="secondary-action action-block" type="button" on:click={snapElection} disabled={running}>
      <Vote size={18} />
      <span>Call snap election</span>
    </button>
  </article>

  <article class="wide-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">district polling</p>
        <h2>District results</h2>
      </div>
      <BarChart3 size={22} />
    </div>

    <div class="stack-list">
      {#each poll?.districts ?? [] as district}
        <article class="item-card">
          <strong>{district.name ?? district.uuid ?? district.district_uuid ?? 'District'}</strong>
          {#each district.results.slice(0, 3) as result}
            <span>{result.party_uuid}: {result.percentage.toFixed(1)} %</span>
          {/each}
        </article>
      {/each}
    </div>
  </article>
</section>
