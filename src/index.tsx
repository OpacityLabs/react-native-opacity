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
  limit: number,
  offset: number
): Promise<WorkflowResponse> {
  return Opacity.getUberRiderTripHistory(limit, offset);
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
