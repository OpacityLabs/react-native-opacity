import Opacity from './NativeOpacity';

type WorkflowResponse = {
  data: string;
  proof?: string;
};

export enum OpacityEnvironment {
  Test = 0,
  Local = 1,
  Staging = 2,
  Production = 3,
}

export function init(
  apiKey: string,
  dryRun: boolean,
  environment: OpacityEnvironment
): Promise<void> {
  return Opacity.init(apiKey, dryRun, environment);
}

export function get(
  name: string,
  params?: Record<string, any>
): Promise<WorkflowResponse> {
  return Opacity.getInternal(name, params ? JSON.stringify(params) : null);
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
  alias: 'uber:rider:read:fare_estimate',
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
  alias: 'github:read:profile'
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'instagram:read:profile'
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'instagram:read:likes'
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'instagram:read:comments'
): Promise<WorkflowResponse>;
export function getResource(
  alias: 'instagram:read:saved_posts'
): Promise<WorkflowResponse>;
export function getResource(
  alias: string,
  ...args: any[]
): Promise<WorkflowResponse> {
  switch (alias.toLowerCase()) {
    // case 'uber:rider:read:profile':
    //   return Opacity.getUberRiderProfile();
    // case 'uber:rider:read:trip_history':
    //   return Opacity.getUberRiderTripHistory(args[0]);
    // case 'uber:rider:read:trip':
    //   return Opacity.getUberRiderTrip(args[0]);
    // case 'uber:driver:read:profile':
    //   return Opacity.getUberDriverProfile();
    // case 'uber:driver:read:trips':
    //   return Opacity.getUberDriverTrips(args[0], args[1], args[2]);
    // case 'reddit:read:account':
    //   return Opacity.getRedditAccount();
    // case 'reddit:read:subreddits':
    //   return Opacity.getRedditFollowedSubreddits();
    // case 'reddit:read:comments':
    //   return Opacity.getRedditComments();
    // case 'reddit:read:posts':
    //   return Opacity.getRedditPosts();
    // case 'zabka:read:account':
    //   return Opacity.getZabkaAccount();
    // case 'zabka:read:points':
    //   return Opacity.getZabkaPoints();
    // case 'uber:rider:read:fare_estimate':
    //   return Opacity.getUberFareEstimate(args[0], args[1], args[2], args[3]);
    // case 'github:read:profile':
    //   return Opacity.getGithubProfile();
    // case 'instagram:read:profile':
    //   return Opacity.getInstagramProfile();
    // case 'instagram:read:likes':
    //   return Opacity.getInstagramLikes();
    // case 'instagram:read:comments':
    //   return Opacity.getInstagramComments();
    // case 'instagram:read:saved_posts':
    //   return Opacity.getInstagramSavedPosts();
    // case 'gusto:read:members_table':
    //   return Opacity.getGustoMembersTable();
    // case 'gusto:read:payroll_admin_id':
    //   return Opacity.getGustoPayrollAdminId();
    default:
      throw Error('Resource not found');
  }
}
