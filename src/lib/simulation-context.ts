import { getContext } from 'svelte';
import type { Writable } from 'svelte/store';
import type {
  ApiMessage,
  ApiSettings,
  MapResponse,
  PollResponse,
  SimulationControls,
  VersionResponse
} from './types';

export const SIMULATION_CONTEXT = Symbol('simulation-context');

export type SimulationContext = {
  settings: Writable<ApiSettings>;
  controls: Writable<SimulationControls>;
  version: Writable<VersionResponse | null>;
  map: Writable<MapResponse | null>;
  poll: Writable<PollResponse | null>;
  loading: Writable<boolean>;
  running: Writable<boolean>;
  message: Writable<ApiMessage | null>;
  updateSettings: (patch: Partial<ApiSettings>) => void;
  updateControls: (patch: Partial<SimulationControls>) => void;
  saveSettingsAndReload: () => void;
  loadBackendState: () => Promise<void>;
  incrementSimulation: () => Promise<void>;
  resetSimulation: () => Promise<void>;
  runPoll: () => Promise<void>;
  snapElection: () => Promise<void>;
  addMedia: (name: string) => Promise<void>;
  addLaw: (text: string, force: boolean) => Promise<void>;
  repealLaw: (lawUuid: string) => Promise<void>;
};

export function getSimulationContext() {
  return getContext<SimulationContext>(SIMULATION_CONTEXT);
}
