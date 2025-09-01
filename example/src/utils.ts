import { OpacityEnvironment } from '@opacity-labs/react-native-opacity';

export function envToString(env: OpacityEnvironment): string {
  switch (env) {
    case OpacityEnvironment.Test:
      return 'Test';
    case OpacityEnvironment.Local:
      return 'Local';
    case OpacityEnvironment.Staging:
      return 'Staging';
    case OpacityEnvironment.Production:
      return 'Production';
    default:
      return 'Unknown';
  }
}
