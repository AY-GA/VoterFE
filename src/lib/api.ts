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
  const baseUrl = settings.baseUrl.trim().replace(/\/$/, '');
  const url = baseUrl ? `${baseUrl}${path}` : path;

  if (!baseUrl) {
    throw new Error('Nastavte URL backendu.');
  }

  if (method === 'POST' && options.token !== false && !settings.token.trim()) {
    throw new Error('Nastavte token simulace.');
  }

  const response = await fetch(url, {
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
  });

  if (response.status === 204) {
    return undefined as T;
  }

  if (!response.ok) {
    throw new Error(`API ${path} selhalo: ${response.status} ${response.statusText}`);
  }

  const text = await response.text();
  return (text ? JSON.parse(text) : undefined) as T;
}

export const api = {
  getVersion: (settings: ApiSettings) =>
    request<VersionResponse>(settings, '/version/', { method: 'GET', token: false }),
  // allow callers to override options (for example: { method: 'GET', token: false })
  getMap: (settings: ApiSettings, options?: { method?: 'GET' | 'POST'; body?: JsonBody; token?: boolean }) =>
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
