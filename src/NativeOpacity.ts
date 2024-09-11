import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';

export interface Spec extends TurboModule {
  init(apiKey: string, dryRun: boolean): void;
  getRiderProfile(): Promise<{ data: string; proof: string }>;
  getRiderTripHistory(
    limit: number,
    offset: number
  ): Promise<{ data: string; proof: string }>;
  getRiderTrip(id: string): Promise<{ data: string; proof: string }>;
  getDriverProfile(): Promise<{ data: string; proof: string }>;
  getDriverTrips(
    startDate: string,
    endDate: string,
    cursor: string
  ): Promise<{ data: string; proof: string }>;
  getRedditAccount(): Promise<{ data: string; proof: string }>;
  getRedditFollowedSubreddits(): Promise<{ data: string; proof: string }>;
  getRedditCommets(): Promise<{ data: string; proof: string }>;
  getRedditPosts(): Promise<{ data: string; proof: string }>;
  zabkaGetAccount(): Promise<{ data: string; proof: string }>;
  zabkaGetPoints(): Promise<{ data: string; proof: string }>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('Opacity');
