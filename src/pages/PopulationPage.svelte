<script lang="ts">
  import { GitBranch, Map as MapIcon } from 'lucide-svelte';
  import type { MapResponse, District } from '../lib/types';

  export let map: MapResponse | null;

  // Derive number of columns from the backend value or from the grid mapping if missing
  $: derivedXs = (() => {
    if (!map) return 0;
    if (typeof map.xs === 'number' && Number.isFinite(map.xs)) return map.xs;
    if (!map.gridmapping || map.gridmapping.length === 0) return 0;
    return Math.max(...map.gridmapping.map((t) => t.x)) + 1;
  })();

  // Build a map of district metadata. If the backend didn't provide `districts`,
  // fall back to generating names from the unique uuids found in `gridmapping`.
  $: districtNames = (() => {
    const m = new Map<string, District & { index: number }>();
    const provided = map?.districts ?? [];
    provided.forEach((d, i) => m.set(d.uuid, { ...d, index: i }));

    if (map?.gridmapping) {
      const seen = new Set(m.keys());
      let idx = m.size;
      for (const t of map.gridmapping) {
        if (!seen.has(t.uuid)) {
          seen.add(t.uuid);
          m.set(t.uuid, { uuid: t.uuid, name: `District ${idx + 1}`, index: idx });
          idx++;
        }
      }
    }

    return m;
  })();

  function tileColor(uuid: string) {
    const index = districtNames.get(uuid)?.index ?? 0;
    const hue = (index * 47) % 360;
    // Use comma-separated HSL for broader compatibility
    return `hsl(${hue}, 62%, 48%)`;
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
        style={`grid-template-columns: repeat(${derivedXs}, minmax(12px, 1fr));`}
        aria-label="District tile map"
      >
        {#each map.gridmapping as tile}
          <span
            style={`--district:${tileColor(tile.uuid)}; background: ${tileColor(tile.uuid)}; grid-column: ${tile.x + 1}; grid-row: ${tile.y + 1};`}
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
