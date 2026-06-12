<script lang="ts">
  import DashboardPage from '../pages/DashboardPage.svelte';
  import { getSimulationContext } from '$lib/simulation-context';

  const { version, map, poll } = getSimulationContext();
</script>

<DashboardPage version={$version} map={$map} poll={$poll} />
