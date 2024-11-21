import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';

export interface Spec extends TurboModule {
  init(apiKey: string, dryRun: boolean, environment: number): Promise<void>;
  getUberRiderProfile(): Promise<{ data: string; proof: string }>;
  getUberRiderTripHistory(
    cursor: string
  ): Promise<{ data: string; proof: string }>;
  getUberRiderTrip(id: string): Promise<{ data: string; proof: string }>;
  getUberDriverProfile(): Promise<{ data: string; proof: string }>;
  getUberDriverTrips(
    startDate: string,
    endDate: string,
    cursor: string
  ): Promise<{ data: string; proof: string }>;
  getUberFareEstimate(
    pickupLatitude: number,
    pickupLongitude: number,
    dropoffLatitude: number,
    dropoffLongitude: number
  ): Promise<{ data: string; proof: string }>;
  getRedditAccount(): Promise<{ data: string; proof: string }>;
  // reddit
  getRedditFollowedSubreddits(): Promise<{ data: string; proof: string }>;
  getRedditComments(): Promise<{ data: string; proof: string }>;
  getRedditPosts(): Promise<{ data: string; proof: string }>;
  // zabka
  getZabkaAccount(): Promise<{ data: string; proof: string }>;
  getZabkaPoints(): Promise<{ data: string; proof: string }>;
  // carta
  getCartaProfile(): Promise<{ data: string; proof: string }>;
  getCartaOrganizations(): Promise<{ data: string; proof: string }>;
  getCartaPortfolioInvestments(
    firmId: string,
    accountId: string
  ): Promise<{ data: string; proof: string }>;
  getCartaHoldingsCompanies(
    accountId: string
  ): Promise<{ data: string; proof: string }>;
  getCartaCorporationSecurities(
    accountId: string,
    corporationId: string
  ): Promise<{ data: string; proof: string }>;
  // github
  getGithubProfile(): Promise<{ data: string; proof: string }>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('Opacity');
