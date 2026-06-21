import type {
  ApiSettings,
  ElectionType,
  MapResponse,
  PartyInput,
  PollResponse,
  VersionResponse,
  VoterInput,
  VoterRecord
} from './types';

const SETTINGS_KEY = 'voternn-api-settings';
// In dev, prefer relative URLs so Vite dev-server proxy can forward requests to WSL backend.
const DEFAULT_BASE_URL =
  import.meta.env.VITE_API_BASE_URL ?? (import.meta.env.DEV ? '' : 'http://127.0.0.1:8080');

type JsonBody = Record<string, unknown>;

export function loadApiSettings(): ApiSettings {
  const defaults = {
    baseUrl: DEFAULT_BASE_URL,
    token: import.meta.env.VITE_API_TOKEN ?? ''
  };

  if (typeof localStorage === 'undefined') {
    return defaults;
  }

  const saved = localStorage.getItem(SETTINGS_KEY);
  if (!saved) {
    return defaults;
  }

  try {
    return { ...defaults, ...JSON.parse(saved) };
  } catch {
    return defaults;
  }
}

export function saveApiSettings(settings: ApiSettings) {
  localStorage.setItem(SETTINGS_KEY, JSON.stringify(settings));
}

async function request<T>(
  settings: ApiSettings,
  path: string,
  options: { method?: 'GET' | 'POST'; body?: JsonBody; token?: boolean } = {}
): Promise<T> {
  const method = options.method ?? 'POST';
  const rawBase = (settings.baseUrl ?? '').trim().replace(/\/$/, '');

  // If a base URL is explicitly provided, always use it (including localhost:8080).
  // If none is provided, use relative paths in dev so the Vite proxy can forward requests.
  let url: string;
  if (rawBase) {
    url = `${rawBase}${path}`;
  } else if (import.meta.env.DEV) {
    url = path;
  } else {
    throw new Error('Please set a backend URL.');
  }

  if (options.token !== false && !settings.token.trim()) {
    throw new Error('Please set a simulation token.');
  }

  // For GET requests, append token as query parameter (GET requests shouldn't have a body)
  if (method === 'GET' && options.token !== false && settings.token.trim()) {
    const separator = url.includes('?') ? '&' : '?';
    url += `${separator}token=${encodeURIComponent(settings.token.trim())}`;
  }

  const response = await Promise.race([
    fetch(url, {
      method,
      headers:
        method === 'POST'
          ? {
              'Content-Type': 'application/json'
            }
          : undefined,
      body:
        method === 'POST'
          ? JSON.stringify({
              ...(options.token === false ? {} : { token: settings.token.trim() }),
              ...(options.body ?? {})
            })
          : undefined
    }),
    new Promise<Response>((_, reject) =>
      setTimeout(() => reject(new Error(`API ${path} request timeout after 10 seconds`)), 10000)
    )
  ]);

  if (response.status === 204) {
    return undefined as T;
  }

  if (!response.ok) {
    throw new Error(`API ${path} failed: ${response.status} ${response.statusText}`);
  }

  const text = await response.text();
  console.log(`API ${path} response:`, text);
  
  if (!text) {
    return undefined as T;
  }
  
  try {
    return JSON.parse(text) as T;
  } catch (err) {
    // Try to find and parse JSON if there's garbage text before it
    const jsonStart = text.indexOf('{');
    if (jsonStart !== -1) {
      try {
        const json = text.substring(jsonStart);
        console.log(`API ${path} extracted JSON:`, json);
        return JSON.parse(json) as T;
      } catch {
        throw new Error(`API ${path} returned invalid JSON: ${text.substring(0, 200)}`);
      }
    }
    throw new Error(`API ${path} returned invalid JSON: ${text.substring(0, 200)}`);
  }
}

export const api = {
  getVersion: (settings: ApiSettings) =>
    request<VersionResponse>(settings, '/version/', { method: 'GET', token: false }),
  // allow callers to override options (for example: { method: 'GET', token: false })
  getMap: (settings: ApiSettings, options?: { method?: 'POST'; body?: JsonBody; token?: boolean }) =>
    request<MapResponse>(settings, '/map/', options),
  increment: (settings: ApiSettings, days: number) =>
    request<void>(settings, '/increment/', { body: { days } }),
  reset: (settings: ApiSettings) => request<void>(settings, '/reset/'),
  snapElection: (settings: ApiSettings, type: ElectionType) =>
    request<void>(settings, '/snap/', { body: { type } }),
  getPolls: (settings: ApiSettings) => request<{ polls: PollResponse[] }>(settings, '/polling/get'),
  getLastPoll: (settings: ApiSettings) => request<PollResponse>(settings, '/polling/last'),
  runPoll: (settings: ApiSettings) => request<PollResponse>(settings, '/polling/run'),
  getVoter: (settings: ApiSettings, voter_uuid: string) =>
    request<VoterRecord>(settings, '/voter/get', { body: { voter_uuid } }),
  createVoter: (settings: ApiSettings, voter: VoterInput) =>
    request<void>(settings, '/voter/create', { body: voter }),
  editVoter: (settings: ApiSettings, voter_uuid: string, voter: VoterInput) =>
    request<void>(settings, '/voter/edit', { body: { voter_uuid, ...voter } }),
  deleteVoter: (settings: ApiSettings, voter_uuid: string) =>
    request<void>(settings, '/voter/delete', { body: { voter_uuid } }),
  createParty: (settings: ApiSettings, party: PartyInput) =>
    request<{ party_uuid: string }>(settings, '/party/create', { body: party }),
  deleteParty: (settings: ApiSettings, party_uuid: string) =>
    request<void>(settings, '/party/delete', { body: { party_uuid } }),
  addMedia: (settings: ApiSettings, name: string) =>
    request<void>(settings, '/media/create/', { body: { name } }),
  addLaw: (settings: ApiSettings, text: string, force = false) =>
    request<void>(settings, force ? '/law/force/' : '/law/add/', { body: { text } }),
  repealLaw: (settings: ApiSettings, law_uuid: string) =>
    request<void>(settings, '/law/repeal/', { body: { law_uuid } })
};
