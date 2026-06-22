<script lang="ts">
  import PoliticsPage from '../../pages/PoliticsPage.svelte';
  import { getSimulationContext } from '$lib/simulation-context';
  import type { ElectionType } from '$lib/types';

  const { poll, running, controls, updateControls, runPoll, snapElection } = getSimulationContext();

  function setElectionType(electionType: ElectionType) {
    updateControls({ electionType });
  }
</script>

<PoliticsPage
  poll={$poll}
  running={$running}
  electionType={$controls.electionType}
  {setElectionType}
  {runPoll}
  {snapElection}
/>
