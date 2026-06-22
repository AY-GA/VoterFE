<script lang="ts">
  import { FilePlus2, Newspaper, Scale, Edit3, Trash2 } from 'lucide-svelte';

  export let running: boolean;
  export let addMedia: (name: string) => Promise<void>;
  export let addLaw: (text: string, force: boolean) => Promise<void>;
  export let repealLaw: (lawUuid: string) => Promise<void>;

  import { onMount } from 'svelte';
  import { api, loadApiSettings } from '$lib/api';

  let mediaList: { uuid: string; name: string }[] = [];
  let lawsList: { uuid: string; text: string }[] = [];

  async function fetchMap() {
    const settings = loadApiSettings();
    try {
      const map = await api.getMap(settings);
      mediaList = (map.media ?? []) as { uuid: string; name: string }[];
      lawsList = (map.laws ?? []) as { uuid: string; text: string }[];
    } catch (e) {
      mediaList = [];
      lawsList = [];
    }
  }

  async function deleteMedia(uuid: string) {
    if (!confirm('Delete this media provider?')) return;
    const settings = loadApiSettings();
    try {
      await api.deleteMedia(settings, { uuid });
      await fetchMap();
    } catch (e) {
      // ignore for now
    }
  }

  async function renameMedia(uuid: string, currentName: string) {
    const next = prompt('Rename provider', currentName);
    if (!next || !next.trim()) return;
    const settings = loadApiSettings();
    try {
      await api.editMedia(settings, { uuid, name: next.trim() });
      await fetchMap();
    } catch (e) {
      // ignore for now
    }
  }

  onMount(() => {
    void fetchMap();
  });

  let mediaName = '';
  let lawText = '';
  let forceLaw = false;
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
      on:click={async () => {
        await addMedia(mediaName);
        mediaName = '';
        await fetchMap();
      }}
    >
      <Newspaper size={18} />
      <span>Add media</span>
    </button>
  </article>

  <article class="wide-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">media</p>
        <h2>Existing providers</h2>
      </div>
      <Newspaper size={22} />
    </div>

    {#if mediaList.length === 0}
      <p class="muted">No providers yet.</p>
    {:else}
      <ul class="media-list">
        {#each mediaList as m}
          <li>
            <div class="media-card">
              <div>
                <strong>{m.name}</strong>
                <small class="mono">{m.uuid}</small>
              </div>
              <div style="display:flex;gap:8px;align-items:center">
                <button class="secondary-action" type="button" on:click={() => renameMedia(m.uuid, m.name)}>
                  <Edit3 size={14} />
                </button>
                <button class="danger-action" type="button" on:click={() => deleteMedia(m.uuid)}>
                  <Trash2 size={14} />
                </button>
              </div>
            </div>
          </li>
        {/each}
      </ul>
    {/if}
  </article>

  <article class="wide-panel">
    <div class="panel-heading">
      <div>
        <p class="eyebrow">law</p>
        <h2>Existing laws</h2>
      </div>
      <FilePlus2 size={22} />
    </div>

    {#if lawsList.length === 0}
      <p class="muted">No laws yet.</p>
    {:else}
      <ul class="media-list">
        {#each lawsList as l}
          <li>
            <div class="media-card">
              <div>
                <strong>{l.text}</strong>
                <small class="mono">{l.uuid}</small>
              </div>
              <div style="display:flex;gap:8px;align-items:center">
                <button
                  class="danger-action"
                  type="button"
                  on:click={async () => {
                    if (!confirm('Repeal this law?')) return;
                    await repealLaw(l.uuid);
                    await fetchMap();
                  }}
                >
                  <Scale size={14} />
                </button>
              </div>
            </div>
          </li>
        {/each}
      </ul>
    {/if}
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
      on:click={async () => {
        await addLaw(lawText, forceLaw);
        lawText = '';
        await fetchMap();
      }}
    >
      <Scale size={18} />
      <span>Add law</span>
    </button>
  </article>

</section>
