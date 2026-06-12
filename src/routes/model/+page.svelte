<script lang="ts">
  import ModelPage from '../../pages/ModelPage.svelte';
  import { getSimulationContext } from '$lib/simulation-context';

  const { settings, running } = getSimulationContext();
</script>

<ModelPage settings={$settings} running={$running} />
