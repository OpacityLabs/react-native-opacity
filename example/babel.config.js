const path = require('path');
const { getConfig } = require('react-native-builder-bob/babel-config');
const pkg = require('../package.json');

const root = path.resolve(__dirname, '..');

module.exports = getConfig(
  {
    plugins: [['module:react-native-dotenv']],
    presets: ['module:@react-native/babel-preset'],
  },
  { root, pkg }
);
