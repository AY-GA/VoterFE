<script lang="ts">
  import PopulationPage from '../../pages/PopulationPage.svelte';
  import { getSimulationContext } from '$lib/simulation-context';

  const { map } = getSimulationContext();
</script>

<PopulationPage map={$map} />
