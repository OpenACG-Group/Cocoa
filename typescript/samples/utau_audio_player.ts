/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

import * as std from 'core';
import * as utau from 'utau';

function main(): void {
    if (std.args.length != 1) {
        std.print('Provide a audio file as argument');
        return;
    }

    const decoder = utau.AVStreamDecoder.MakeFromFile(
        std.args[0], {disableVideo: true});

    const info = decoder.getStreamInfo(utau.Constants.STREAM_SELECTOR_AUDIO);

    const inparams: utau.InBufferParameters = {
        name: 'in',
        mediaType: utau.Constants.MEDIA_TYPE_AUDIO,
        sampleFormat: info.sampleFormat,
        channelMode: info.channelMode,
        sampleRate: info.sampleRate
    };

    const DAG = utau.AVFilterDAG.MakeFromDSL(
        'aresample=osf=flt:osr=44100:ochl=stereo',
        [inparams], [{name: 'out', mediaType: utau.Constants.MEDIA_TYPE_AUDIO}]);

    utau.AudioSinkContext.Initialize();

    let decodeBuffer = decoder.decodeNextFrame();
    let lastBufferId = 0;
    while (decodeBuffer.type != utau.Constants.DECODE_BUFFER_EOF) {
        if (decodeBuffer.type == utau.Constants.DECODE_BUFFER_NULL) {
            std.print('Failed in decoding\n');
            return;
        }

        const filtered = DAG.filter([{
            name: 'in',
            mediaType: utau.Constants.MEDIA_TYPE_AUDIO,
            audioBuffer: decodeBuffer.audioBuffer
        }]);

        lastBufferId = utau.AudioSinkContext.Enqueue(filtered[0].audioBuffer);

        decodeBuffer = decoder.decodeNextFrame();
    }

    utau.AudioSinkContext.AddBufferEventListener({
        consumed: (id: number) => {
            if (lastBufferId == id)
            {
                std.print('Playing ended\n');
                utau.AudioSinkContext.Dispose(true);
            }
        }
    });
}

main();
