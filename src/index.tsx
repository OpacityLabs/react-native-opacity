const Opacity = require('./NativeOpacity').default;

export function init(apiKey: string, dryRun: boolean): void {
  return Opacity.init(apiKey, dryRun);
}

export function getUberRiderPorfile(): Promise<string> {
  return Opacity.getRiderProfile();
}
