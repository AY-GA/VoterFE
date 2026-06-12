<script lang="ts">
  import { GitBranch, Map as MapIcon } from 'lucide-svelte';
  import type { MapResponse } from '../lib/types';

  export let map: MapResponse | null;

  $: districtNames = new Map(
    map?.districts.map((district, index) => [district.uuid, { ...district, index }]) ?? []
  );

  function tileColor(uuid: string) {
    const index = districtNames.get(uuid)?.index ?? 0;
    const hue = (index * 47) % 360;
    return `hsl(${hue} 62% 48%)`;
  }
</script>

<svelte:head>
  <title>Districts | VoterNN</title>
</svelte:head>

<section class="split-layout">
  <article class="wide-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">map</p>
        <h2>District assignment</h2>
      </div>
      <MapIcon size={22} />
    </div>

    {#if map}
      <div
        class="district-grid"
        style={`grid-template-columns: repeat(${map.xs}, minmax(12px, 1fr));`}
        aria-label="District tile map"
      >
        {#each map.gridmapping as tile}
          <span
            style={`--district:${tileColor(tile.uuid)}`}
            title={`${districtNames.get(tile.uuid)?.name ?? tile.uuid} (${tile.x}, ${tile.y})`}
          ></span>
        {/each}
      </div>
    {:else}
      <p class="delta-copy">District map is not loaded yet.</p>
    {/if}
  </article>

  <div class="stack-list">
    <div class="panel-heading compact-heading">
      <div>
        <p class="eyebrow">districts</p>
        <h2>List</h2>
      </div>
      <GitBranch size={20} />
    </div>

    {#each map?.districts ?? [] as district}
      <article class="item-card">
        <strong>{district.name}</strong>
        <span>{district.uuid}</span>
        {#if district.gdp}
          <small>GDP: {district.gdp}</small>
        {/if}
      </article>
    {/each}
  </div>
</section>
