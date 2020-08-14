/**
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 * @format
 */

'use strict';

import {SchemaType} from 'react-native-tscodegen';

type FilesOutput = Map<string, string>;

export function createComponentSchemaGenerator() {
  return (
    _libraryName: string,
    schema: SchemaType,
    _moduleSpecName: string,
  ): FilesOutput => {
    let files = new Map<string, string>();

    const components = Object.keys(schema.modules)
      .map(moduleName => {
        const modules = schema.modules[moduleName].components;
        return modules ? modules : undefined;
      })
      .filter(Boolean)
      .reduce((acc, c) => Object.assign(acc, c), {});

    if (components) {
      Object.keys(components).forEach(name => {
        console.log(`Generating ${name}Schema.json`);
        files.set(
          `${name}Schema.json`,
          JSON.stringify(components[name], null, 2),
        );
      });
    }

    return files;
  };
}
