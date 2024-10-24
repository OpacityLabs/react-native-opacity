import Opacity from './NativeOpacity';

type WorkflowResponse = {
  data: string;
  proof: string;
};

export function init(apiKey: string, dryRun: boolean): Promise<void> {
  return Opacity.init(apiKey, dryRun);
}

export function getUberRiderPorfile(): Promise<WorkflowResponse> {
  return Opacity.getUberRiderProfile();
}

export function getUberRiderTripHistory(
  cursor: string
): Promise<WorkflowResponse> {
  return Opacity.getUberRiderTripHistory(cursor);
}

export function getUberRiderTrip(id: string): Promise<WorkflowResponse> {
  return Opacity.getUberRiderTrip(id);
}

export function getUberDriverProfile(): Promise<WorkflowResponse> {
  return Opacity.getUberDriverProfile();
}

export function getUberDriverTrips(
  startDate: string,
  endDate: string,
  cursor: string
): Promise<WorkflowResponse> {
  return Opacity.getUberDriverTrips(startDate, endDate, cursor);
}

export function getRedditAccount(): Promise<WorkflowResponse> {
  return Opacity.getRedditAccount();
}

export function getRedditFollowedSubreddits(): Promise<WorkflowResponse> {
  return Opacity.getRedditFollowedSubreddits();
}

export function getRedditCommets(): Promise<WorkflowResponse> {
  return Opacity.getRedditCommets();
}

export function getRedditPosts(): Promise<WorkflowResponse> {
  return Opacity.getRedditPosts();
}

export function getZabkaAccount(): Promise<WorkflowResponse> {
  return Opacity.getZabkaAccount();
}

export function getZabkaPoints(): Promise<WorkflowResponse> {
  return Opacity.getZabkaPoints();
}

export function getResource(
  alias: 'uber:rider:read:trip_history',
  cursor: string
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'uber:rider:read:profile'
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'uber:rider:read:trip',
  id: string
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'uber:driver:read:profile'
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'uber:rider:read:trips',
  startDate: string,
  endDate: string,
  cursor: string
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'uber:fare:estimate',
  pickupLatitute: number,
  pickupLongitude: number,
  dropoffLatitute: number,
  dropoffLongitude: number
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'reddit:read:account'
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'reddit:read:subreddits'
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'reddit:read:comments'
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'reddit:read:posts'
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'zabka:read:account'
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'zabka:read:points'
): Promise<WorkflowResponse>;
export function getResource(
  alias: string,
  ...args: any[]
): Promise<WorkflowResponse> {
  switch (alias.toLowerCase()) {
    case 'uber:rider:read:profile':
      return Opacity.getUberRiderProfile();
    case 'uber:rider:read:trip_history':
      return Opacity.getUberRiderTripHistory(args[0]);
    case 'uber:rider:read:trip':
      return Opacity.getUberRiderTrip(args[0]);
    case 'uber:driver:read:profile':
      return Opacity.getUberDriverProfile();
    case 'uber:driver:read:trips':
      return Opacity.getUberDriverTrips(args[0], args[1], args[2]);
    case 'reddit:read:account':
      return Opacity.getRedditAccount();
    case 'reddit:read:subreddits':
      return Opacity.getRedditFollowedSubreddits();
    case 'reddit:read:comments':
      return Opacity.getRedditCommets();
    case 'reddit:read:posts':
      return Opacity.getRedditPosts();
    case 'zabka:read:account':
      return Opacity.getZabkaAccount();
    case 'zabka:read:points':
      return Opacity.getZabkaPoints();
    case 'uber:fare:estimate':
      return Opacity.getUberFareEstimate(args[0], args[1], args[2], args[3]);
    default:
      throw Error('Resource not found');
  }
}
