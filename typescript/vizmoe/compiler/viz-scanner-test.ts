import * as std from 'core';
import * as Parser from './viz-parser';
import { print } from '../../core/pretty_printer';

const CONTENT = `
@Window {
    @Geometry {
        @Positioner {}
        @Positioner {}
    }
    dev: @Positioner {};
    file: @File { inner: @Inner {}; };
}
`;

try {
    const TU = Parser.VizParse(CONTENT);
    print(TU);
} catch (except: any) {
    std.print(`ERROR: ${except.token.lexemeRange}: ${except.message}\n`);
}
