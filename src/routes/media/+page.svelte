<script lang="ts">
  import MediaPage from '../../pages/MediaPage.svelte';
  import { getSimulationContext } from '$lib/simulation-context';

  const { running, addMedia, addLaw, repealLaw } = getSimulationContext();
</script>

<MediaPage running={$running} {addMedia} {addLaw} {repealLaw} />
