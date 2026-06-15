<script lang="ts">
  import { BarChart3, Gauge, Map, Server } from 'lucide-svelte';
  import { getSimulationContext } from '$lib/simulation-context';

  const { version, map, poll } = getSimulationContext();

  let leadingParty = $poll?.national?.length
    ? [...$poll.national].sort((a, b) => b.percentage - a.percentage)[0]
    : null;
</script>

<svelte:head>
  <title>Overview | VoterNN</title>
</svelte:head>

<section class="dashboard-grid">
  <article class="overview-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">backend</p>
        <h2>Connection status</h2>
      </div>
      <Server size={22} />
    </div>

    <div class="metric-grid">
      <div>
        <span>Version</span>
        <strong>{$version?.version_string ?? 'not loaded'}</strong>
      </div>
      <div>
        <span>Major</span>
        <strong>{$version?.major ?? '--'}</strong>
      </div>
      <div>
        <span>Minor</span>
        <strong>{$version?.minor ?? '--'}</strong>
      </div>
      <div>
        <span>Build</span>
        <strong>{$version?.build ?? '--'}</strong>
      </div>
    </div>
  </article>

  <article class="network-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">map</p>
        <h2>District grid</h2>
      </div>
      <Map size={22} />
    </div>

    <div class="metric-grid">
      <div>
        <span>Districts</span>
        <strong>{$map?.districts.length ?? '--'}</strong>
      </div>
      <div>
        <span>Width</span>
        <strong>{$map?.xs ?? '--'}</strong>
      </div>
      <div>
        <span>Height</span>
        <strong>{$map?.ys ?? '--'}</strong>
      </div>
      <div>
        <span>Tiles</span>
        <strong>{$map?.gridmapping.length ?? '--'}</strong>
      </div>
    </div>

    {#if $map}
      <p class="delta-copy">
        First district: {$map.districts[0]?.name ?? 'none'}.
      </p>
    {/if}
  </article>

  <article class="forecast-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">polling</p>
        <h2>Latest national poll</h2>
      </div>
      <BarChart3 size={22} />
    </div>

    {#if $poll?.national?.length}
      <div class="forecast-bars">
        {#each $poll.national.slice(0, 6) as result}
          <div style={`--bar:${Math.min(result.percentage, 100)}%`}>
            <span>{result.party_uuid}</span>
            <strong>{result.percentage.toFixed(1)} %</strong>
          </div>
        {/each}
      </div>
    {:else}
      <p class="delta-copy">No poll data loaded yet.</p>
    {/if}
  </article>

  <article class="event-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">elections</p>
        <h2>Current leader</h2>
      </div>
      <Gauge size={22} />
    </div>
    {#if leadingParty}
      <div class="support-summary">
        <span>{leadingParty.party_uuid}</span>
        <strong>{leadingParty.percentage.toFixed(1)} %</strong>
        <small>according to the latest polling</small>
      </div>
    {:else}
      <p class="delta-copy">Run a poll or set a simulation token.</p>
    {/if}
  </article>
</section>
