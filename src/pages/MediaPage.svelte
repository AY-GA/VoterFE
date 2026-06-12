<script lang="ts">
  import { FilePlus2, Newspaper, Scale } from 'lucide-svelte';

  export let running: boolean;
  export let addMedia: (name: string) => Promise<void>;
  export let addLaw: (text: string, force: boolean) => Promise<void>;
  export let repealLaw: (lawUuid: string) => Promise<void>;

  let mediaName = '';
  let lawText = '';
  let forceLaw = false;
  let lawUuid = '';
</script>

<svelte:head>
  <title>Media & Laws | VoterNN</title>
</svelte:head>

<section class="media-layout">
  <article class="wide-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">media</p>
        <h2>Add media provider</h2>
      </div>
      <Newspaper size={22} />
    </div>

    <label>
      <span>Name</span>
      <input bind:value={mediaName} placeholder="Agora Stream" />
    </label>
    <button
      class="primary-action action-block"
      type="button"
      disabled={running || !mediaName.trim()}
      on:click={() => addMedia(mediaName)}
    >
      <Newspaper size={18} />
      <span>Add media</span>
    </button>
  </article>

  <article class="wide-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">law</p>
        <h2>Add law text</h2>
      </div>
      <FilePlus2 size={22} />
    </div>

    <label>
      <span>Law text</span>
      <textarea bind:value={lawText} rows="10" placeholder="Full law text"></textarea>
    </label>
    <label class="check-field">
      <input bind:checked={forceLaw} type="checkbox" />
      <span>force pass without vote</span>
    </label>
    <button
      class="primary-action action-block"
      type="button"
      disabled={running || !lawText.trim()}
      on:click={() => addLaw(lawText, forceLaw)}
    >
      <Scale size={18} />
      <span>Add law</span>
    </button>
  </article>

  <article class="control-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">law</p>
        <h2>Force repeal</h2>
      </div>
      <Scale size={22} />
    </div>
    <label>
      <span>Law UUID</span>
      <input bind:value={lawUuid} placeholder="law_uuid" />
    </label>
    <button
      class="secondary-action action-block"
      type="button"
      disabled={running || !lawUuid.trim()}
      on:click={() => repealLaw(lawUuid)}
    >
      <Scale size={18} />
      <span>Repeal law</span>
    </button>
  </article>
</section>
