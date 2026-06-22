export type ApiSettings = {
  baseUrl: string;
  token: string;
};

export type VersionResponse = {
  version_string: string;
  major: number;
  minor: number;
  build: number;
};

export type District = {
  uuid: string;
  name: string;
  gdp?: number;
  [key: string]: unknown;
};

export type GridMapping = {
  x: number;
  y: number;
  uuid: string;
};

export type MapResponse = {
  districts: District[];
  gridmapping: GridMapping[];
  xs: number;
  ys: number;
};

export type PollResult = {
  party_uuid: string;
  party_name?: string;
  percentage: number;
  votes?: number;
};

export type DistrictPoll = {
  uuid?: string;
  district_uuid?: string;
  name?: string;
  results: PollResult[];
  [key: string]: unknown;
};

export type PollResponse = {
  national: PollResult[];
  districts: DistrictPoll[];
};

export type ElectionType = 'parliament' | 'senate' | 'president' | 'general' | 'all';

export type SimulationControls = {
  days: number;
  electionType: ElectionType;
};

export type OpinionPosition = {
  security: number;
  risk_aversion: number;
  public_spending: number;
  cost_of_living: number;
  minorities_support: number;
  stability: number;
  job_security: number;
  future_outlook: number;
  equality_markets: number;
  nation_globe: number;
  liberty_authority: number;
  tradition_progress: number;
};

export type VoterInput = OpinionPosition & {
  name: string;
  middlename: string;
  surname: string;
  age: number;
};

export type VoterRecord = VoterInput & {
  voter_uuid?: string;
};

export type VoterListResponse = {
  voters: VoterRecord[];
  total?: number;
};

export type PartyInput = OpinionPosition & {
  name: string;
};

export type ApiMessage = {
  tone: 'success' | 'error' | 'info';
  text: string;
};
