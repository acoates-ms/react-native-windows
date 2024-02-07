// Change the below to true for autolink to target playground-win32 instead of playground.
// Then run `npx react-native autolink-windows` to actually run the autolink.

const target = 'win32' // 'win32' |'composition' | 'uwp'

module.exports = {
  project: {
    windows: {
      sourceDir: 'windows',
      solutionFile: (target === 'win32' ? 'playground-win32.sln' : (target === 'composition' ? 'playground-composition.sln' : 'playground.sln')),
      project: {
        projectFile: (target === 'win32' ? 'playground-win32\\playground-win32.vcxproj' : (target === 'composition' ? 'playground-composition\\playground-composition.vcxproj' : 'playground\\playground.vcxproj'))
      },
    },
  },
};
