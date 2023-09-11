# Playground

Playground is a sample standalone application that allows testing of various JS files, including RNTester.

# Building

You'll need to install [node](https://nodejs.org/en/download), and [classic yarn](https://classic.yarnpkg.com/en/docs/install#windows-stable) if you dont have them. A more complete set of dependencies can be installed using our [dependencies script](https://microsoft.github.io/react-native-windows/docs/next/rnw-dependencies) 

## Steps to build/run:
download WinAppSDK nuget from https://artprodcussu4.artifacts.visualstudio.com/Acb55739e-4afe-46a3-970f-1b49d8ee7564/55e8140e-57ac-4e5f-8f9c-c7c15b51929d/_apis/artifact/cGlwZWxpbmVhcnRpZmFjdDovL21pY3Jvc29mdC9wcm9qZWN0SWQvNTVlODE0MGUtNTdhYy00ZTVmLThmOWMtYzdjMTViNTE5MjlkL2J1aWxkSWQvNzg1NjQ1ODUvYXJ0aWZhY3ROYW1lL1dpbmRvd3NBcHBTREtfTnVnZXRfQW5kX01TSVg1/content?format=file&subPath=%2FNugetPackages%2FMicrosoft.WindowsAppSDK.1.5.230906000-experimental.nupkg, and put it in a local directory.
Edit `NuGet.Config` in the root of the repo and add the location of the above nuget to the config.
`yarn`
open `packages\playground-fabric\windows\Playground-composition.sln`
In devenv: build and run the Playground-composition project
Back in a command prompt:
`cd packages\playground-fabric`
`yarn start`

Then in the app, 
`File` -> `Open JavaScript File`
Samples\rntester
`OK`
Click on `Custom Native Fabric Component`



# Issues
If you get an error trying to load the bundle, and asking if the packager is running.
 - Metro will often crash when rebuilding the native app - Check your command prompt where you ran `yarn start`.  If the command ended, rerun it.

The version of WinAppSDK being used has not been published, you'll need to put the nuget in a location, and update `Nuget.Config` in the root of the repo to point to a location of the nuget.

The path to `Microsoft.UI.Dispatching.Interop.h` is hardcoded, since it wasn't ending up in the projects include path for some reason.  You'll likely need to modify that include path