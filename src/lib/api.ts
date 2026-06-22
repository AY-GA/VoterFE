import type {
  ApiSettings,
  District,
  ElectionType,
  MapResponse,
  PartyInput,
  PollResponse,
  VersionResponse,
  VoterInput,
  VoterListResponse,
  VoterRecord
} from './types';

const SETTINGS_KEY = 'voternn-api-settings';
// In dev, prefer relative URLs so Vite dev-server proxy can forward requests to WSL backend.
const DEFAULT_BASE_URL =
  import.meta.env.VITE_API_BASE_URL ?? (import.meta.env.DEV ? '' : 'http://127.0.0.1:8080');

type JsonBody = Record<string, unknown>;

function asRecord(value: unknown): Record<string, unknown> {
  return value && typeof value === 'object' && !Array.isArray(value) ? (value as Record<string, unknown>) : {};
}

function asArray(value: unknown): unknown[] {
  return Array.isArray(value) ? value : [];
}

function asString(value: unknown, fallback = '') {
  return typeof value === 'string' ? value : fallback;
}

function asNumber(value: unknown, fallback = 0) {
  const next = typeof value === 'number' ? value : Number(value);
  return Number.isFinite(next) ? next : fallback;
}

function normalizePollResult(value: unknown) {
  const row = asRecord(value);
  const partyUuid = asString(
    row.party_uuid ?? row.partyUuid ?? row.uuid ?? row.party ?? row.name,
    'unknown-party'
  );

  return {
    party_uuid: partyUuid,
    party_name: asString(row.party_name ?? row.partyName ?? row.name, partyUuid),
    percentage: asNumber(row.percentage ?? row.percent ?? row.share ?? row.value, 0),
    votes: row.votes === undefined ? undefined : asNumber(row.votes, 0)
  };
}

function normalizePollResponse(value: unknown): PollResponse {
  const data = asRecord(value);
  const nationalSource = data.national ?? data.results ?? data.parties;
  const districtSource = data.districts ?? data.district_results ?? data.districtResults;

  return {
    national: asArray(nationalSource).map(normalizePollResult),
    districts: asArray(districtSource).map((district) => {
      const row = asRecord(district);
      const uuid = asString(row.uuid ?? row.district_uuid ?? row.districtUuid);
      return {
        ...row,
        uuid,
        district_uuid: asString(row.district_uuid ?? row.districtUuid, uuid),
        name: asString(row.name ?? row.district_name ?? row.districtName, uuid || 'District'),
        results: asArray(row.results ?? row.national ?? row.parties).map(normalizePollResult)
      };
    })
  };
}

function normalizeVoter(value: unknown): VoterRecord {
  const row = asRecord(value);
  return {
    voter_uuid: asString(row.voter_uuid ?? row.voterUuid ?? row.uuid),
    name: asString(row.name),
    middlename: asString(row.middlename ?? row.middle_name ?? row.middleName),
    surname: asString(row.surname ?? row.last_name ?? row.lastName),
    age: asNumber(row.age, 0),
    security: asNumber(row.security, 0),
    risk_aversion: asNumber(row.risk_aversion, 0),
    public_spending: asNumber(row.public_spending, 0),
    cost_of_living: asNumber(row.cost_of_living, 0),
    minorities_support: asNumber(row.minorities_support, 0),
    stability: asNumber(row.stability, 0),
    job_security: asNumber(row.job_security, 0),
    future_outlook: asNumber(row.future_outlook, 0),
    equality_markets: asNumber(row.equality_markets, 0),
    nation_globe: asNumber(row.nation_globe, 0),
    liberty_authority: asNumber(row.liberty_authority, 0),
    tradition_progress: asNumber(row.tradition_progress, 0)
  };
}

function normalizeVoterList(value: unknown): VoterListResponse {
  const data = asRecord(value);
  const voters = asArray(data.voters ?? data.results ?? data.items).map(normalizeVoter);
  return {
    voters,
    total: data.total === undefined ? voters.length : asNumber(data.total, voters.length)
  };
}

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

  const requestBody =
    method === 'POST'
      ? JSON.stringify({
          ...(options.token === false ? {} : { token: settings.token.trim() }),
          ...(options.body ?? {})
        })
      : undefined;

  const response = await Promise.race([
    fetch(url, {
      method,
      headers: method === 'POST' ? { 'Content-Type': 'application/json' } : undefined,
      body: requestBody
    }),
    new Promise<Response>((_, reject) =>
      setTimeout(() => reject(new Error(`API ${path} request timeout after 10 seconds`)), 10000)
    )
  ]);

  const text = await response.text();

  if (response.status === 204) {
    return undefined as T;
  }

  if (!response.ok) {
    const preview = text ? text.substring(0, 1000) : '';
    throw new Error(`API ${path} failed: ${response.status} ${response.statusText} - ${preview}`);
  }

  try {
    return JSON.parse(text) as T;
  } catch {
    const jsonStart = text.indexOf('{');
    if (jsonStart !== -1) {
      try {
        return JSON.parse(text.substring(jsonStart)) as T;
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

  getMap: (settings: ApiSettings) => request<MapResponse>(settings, '/map/'),

  increment: (settings: ApiSettings, days: number) =>
    request<void>(settings, '/increment/', { body: { days } }),

  reset: (settings: ApiSettings) => request<void>(settings, '/reset/'),

  snapElection: (settings: ApiSettings, type: ElectionType) =>
    request<void>(settings, '/snap/', { body: { type } }),

  setSystem: (settings: ApiSettings, system: string) =>
    request<void>(settings, '/system/', { body: { system } }),

  setPresidentialSystem: (settings: ApiSettings, system: string) =>
    request<void>(settings, '/presidential/', { body: { system } }),

  getPolls: async (settings: ApiSettings) => {
    const response = await request<{ polls?: unknown[] }>(settings, '/polling/get/');
    return { polls: asArray(response.polls).map(normalizePollResponse) };
  },

  getLastPoll: async (settings: ApiSettings) =>
    normalizePollResponse(await request<unknown>(settings, '/polling/last/')),

  runPoll: async (settings: ApiSettings) => {
    const response = await request<unknown>(settings, '/polling/run/');
    const result = normalizePollResponse(response);

    // If backend returned an empty result, try sensible fallbacks:
    // 1) `last` poll endpoint
    // 2) `get` polls list (take latest)
    if ((result.national?.length ?? 0) === 0 && (result.districts?.length ?? 0) === 0) {
      try {
        const last = normalizePollResponse(await request<unknown>(settings, '/polling/last/'));
        if ((last.national?.length ?? 0) > 0 || (last.districts?.length ?? 0) > 0) return last;
      } catch {}

      try {
        const got = await request<{ polls?: unknown[] }>(settings, '/polling/get/');
        const polls = asArray(got.polls).map(normalizePollResponse);
        if (polls.length) return polls[polls.length - 1];
      } catch {}
    }

    return result;
  },

  getVoter: (settings: ApiSettings, voter_uuid: string) =>
    request<unknown>(settings, '/voter/get/', { body: { voter_uuid } }).then(normalizeVoter),

  listVoters: (settings: ApiSettings, search = '', limit = 40) =>
    request<unknown>(settings, '/voter/list/', { body: { search, limit } }).then(normalizeVoterList),

  createVoter: (settings: ApiSettings, voter: VoterInput) =>
    request<void>(settings, '/voter/create/', { body: voter }),

  editVoter: (settings: ApiSettings, voter_uuid: string, voter: VoterInput) =>
    request<void>(settings, '/voter/edit/', { body: { voter_uuid, ...voter } }),

  deleteVoter: (settings: ApiSettings, voter_uuid: string) =>
    request<void>(settings, '/voter/delete/', { body: { voter_uuid } }),

  getDistrict: (settings: ApiSettings, district_uuid: string) =>
    request<District>(settings, '/district/get/', { body: { district_uuid } }),

  createDistrict: (settings: ApiSettings, name: string) =>
    request<{ district_uuid: string }>(settings, '/district/create/', { body: { name } }),

  editDistrict: (settings: ApiSettings, district_uuid: string, patch: JsonBody) =>
    request<void>(settings, '/district/edit/', { body: { district_uuid, ...patch } }),

  deleteDistrict: (settings: ApiSettings, district_uuid: string) =>
    request<void>(settings, '/district/delete/', { body: { district_uuid } }),

  nukeDistrict: (settings: ApiSettings, district_uuid: string) =>
    request<void>(settings, '/district/nuke/', { body: { district_uuid } }),

  mergeDistricts: (settings: ApiSettings, district_uuid_1: string, district_uuid_2: string) =>
    request<void>(settings, '/district/merge/', { body: { district_uuid_1, district_uuid_2 } }),

  reassignDistrictTile: (
    settings: ApiSettings,
    district_uuid: string,
    x: number,
    y: number
  ) => request<void>(settings, '/district/reassign/', { body: { district_uuid, x, y } }),

  createParty: (settings: ApiSettings, party: PartyInput) =>
    request<{ party_uuid: string }>(settings, '/party/create/', { body: party }),

  editParty: (settings: ApiSettings, party_uuid: string, party: PartyInput) =>
    request<void>(settings, '/party/edit/', { body: { party_uuid, ...party } }),

  deleteParty: (settings: ApiSettings, party_uuid: string) =>
    request<void>(settings, '/party/delete/', { body: { party_uuid } }),

  mergeParties: (
    settings: ApiSettings,
    party_uuid_1: string,
    party_uuid_2: string,
    name = ''
  ) => request<void>(settings, '/party/merge/', { body: { party_uuid_1, party_uuid_2, name } }),

  addCompany: (settings: ApiSettings, body: JsonBody) =>
    request<void>(settings, '/company/add/', { body }),

  editCompany: (settings: ApiSettings, body: JsonBody) =>
    request<void>(settings, '/company/edit/', { body }),

  deleteCompany: (settings: ApiSettings, body: JsonBody) =>
    request<void>(settings, '/company/delete/', { body }),

  addProduct: (settings: ApiSettings, body: JsonBody) =>
    request<void>(settings, '/product/add/', { body }),

  editProduct: (settings: ApiSettings, body: JsonBody) =>
    request<void>(settings, '/product/edit/', { body }),

  deleteProduct: (settings: ApiSettings, body: JsonBody) =>
    request<void>(settings, '/product/delete/', { body }),

  editGdp: (settings: ApiSettings, body: JsonBody) =>
    request<void>(settings, '/statistics/edit/gdp/', { body }),

  createCrisis: (settings: ApiSettings, body: JsonBody) =>
    request<void>(settings, '/crisis/create/', { body }),

  deleteCrisis: (settings: ApiSettings, body: JsonBody) =>
    request<void>(settings, '/crisis/delete/', { body }),

  addMedia: (settings: ApiSettings, name: string) =>
    request<void>(settings, '/media/create/', { body: { name } }),

  editMedia: (settings: ApiSettings, body: JsonBody) =>
    request<void>(settings, '/media/edit/', { body }),

  deleteMedia: (settings: ApiSettings, body: JsonBody) =>
    request<void>(settings, '/media/delete/', { body }),

  addLaw: (settings: ApiSettings, text: string, force = false) =>
    request<void>(settings, force ? '/law/force/' : '/law/add/', { body: { text } }),

  repealLaw: (settings: ApiSettings, law_uuid: string) =>
    request<void>(settings, '/law/repeal/', { body: { law_uuid } }),

  addLawArticle: (settings: ApiSettings, law_uuid: string, text: string) =>
    request<void>(settings, '/law/article/add/', { body: { law_uuid, text } }),

  repealLawArticle: (settings: ApiSettings, law_uuid: string, article: number) =>
    request<void>(settings, '/law/article/repeal/', { body: { law_uuid, article } }),

  addConstitutionArticle: (settings: ApiSettings, body: JsonBody) =>
    request<void>(settings, '/constitution/article/add/', { body }),

  removeConstitutionArticle: (settings: ApiSettings, body: JsonBody) =>
    request<void>(settings, '/constitution/article/remove/', { body }),

  redraftConstitution: (settings: ApiSettings, body: JsonBody = {}) =>
    request<void>(settings, '/constitution/redraft/', { body })
};
