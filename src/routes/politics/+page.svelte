<script lang="ts">
  import PoliticsPage from '../../pages/PoliticsPage.svelte';
  import { getSimulationContext } from '$lib/simulation-context';
  import type { ElectionType } from '$lib/types';

  const { poll, running, controls, updateControls, runPoll, snapElection } = getSimulationContext();

  let electionType: ElectionType = 'parliament';

  $: electionType = $controls.electionType;
  $: if (electionType !== $controls.electionType) {
    updateControls({ electionType });
  }
</script>

<PoliticsPage poll={$poll} running={$running} bind:electionType {runPoll} {snapElection} />
