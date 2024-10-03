import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';
import type { WorkflowArgs } from './index';

export interface Spec extends TurboModule {
  init(apiKey: string, dryRun: boolean): Promise<void>;
  getUberRiderProfile(): Promise<{ data: string; proof: string }>;
  getUberRiderTripHistory(
    limit: number,
    offset: number
  ): Promise<{ data: string; proof: string }>;
  getUberRiderTrip(id: string): Promise<{ data: string; proof: string }>;
  getUberDriverProfile(): Promise<{ data: string; proof: string }>;
  getUberDriverTrips(
    startDate: string,
    endDate: string,
    cursor: string
  ): Promise<{ data: string; proof: string }>;
  getRedditAccount(): Promise<{ data: string; proof: string }>;
  getRedditFollowedSubreddits(): Promise<{ data: string; proof: string }>;
  getRedditCommets(): Promise<{ data: string; proof: string }>;
  getRedditPosts(): Promise<{ data: string; proof: string }>;
  getZabkaAccount(): Promise<{ data: string; proof: string }>;
  getZabkaPoints(): Promise<{ data: string; proof: string }>;
  callResource(
    alias: string,
    args?: WorkflowArgs
  ): Promise<{ data: string; proof: string }>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('Opacity');
