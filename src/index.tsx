const Opacity = require('./NativeOpacity').default;

export function init(apiKey: string, dryRun: boolean): void {
  return Opacity.init(apiKey, dryRun);
}

export function getUberRiderPorfile(): Promise<string> {
  return Opacity.getRiderProfile();
}

export function getUberRiderTripHistory(
  limit: number,
  offset: number
): Promise<string> {
  return Opacity.getRiderTripHistory(limit, offset);
}

export function getUberRiderTrip(id: string): Promise<string> {
  return Opacity.getRiderTrip(id);
}

export function getUberDriverProfile(): Promise<string> {
  return Opacity.getDriverProfile();
}

export function getUberDriverTrips(
  startDate: string,
  endDate: string,
  cursor: string
): Promise<string> {
  return Opacity.getDriverTrips(startDate, endDate, cursor);
}

export function getRedditAccount(): Promise<string> {
  return Opacity.getRedditAccount();
}

export function getRedditFollowedSubreddits(): Promise<string> {
  return Opacity.getRedditFollowedSubreddits();
}

export function getRedditCommets(): Promise<string> {
  return Opacity.getRedditCommets();
}

export function getRedditPosts(): Promise<string> {
  return Opacity.getRedditPosts();
}

export function getZabkaAccount(): Promise<string> {
  return Opacity.getZabkaAccount();
}

export function getZabkaPoints(): Promise<string> {
  return Opacity.getZabkaPoints();
}
