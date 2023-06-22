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
import * as GL from 'glamor';
import * as Utau from 'utau';

const decoder = Utau.AVStreamDecoder.MakeFromFile(std.args[0], { useHWDecoding: false });
const audioStreamInfo = decoder.getStreamInfo(Utau.Constants.STREAM_SELECTOR_AUDIO);
const videoStreamInfo = decoder.getStreamInfo(Utau.Constants.STREAM_SELECTOR_VIDEO);

const width = videoStreamInfo.width, height = videoStreamInfo.height;

GL.RenderHost.Initialize({ name: 'MediaPresent', major: 1, minor: 0, patch: 0 });
const display = await GL.RenderHost.Connect();
const surface = await display.createHWComposeSurface(width, height);
const blender = await surface.createBlender();

const audioDevice = Utau.AudioDevice.ConnectPipeWire();
const audioSinkStream = audioDevice.createSinkStream('Playback');
audioDevice.unref();

audioSinkStream.connect(audioStreamInfo.sampleFormat,
                        audioStreamInfo.channelMode,
                        audioStreamInfo.sampleRate,
                        true);

let videoFilter: Utau.AVFilterDAG = null;

const dispatcher = new Utau.MediaFramePresentDispatcher(decoder, audioSinkStream);
dispatcher.onPresentVideoBuffer = (buffer: Utau.VideoBuffer) => {

    /*
    if (videoFilter == null) {
        videoFilter = Utau.AVFilterDAG.MakeFromDSL(
            '[in1] [out1]',
            [
                {
                    name: 'in1',
                    mediaType: Utau.Constants.MEDIA_TYPE_VIDEO,
                    pixelFormat: videoStreamInfo.pixelFormat,
                    width: videoStreamInfo.width,
                    height: videoStreamInfo.height,
                    hwFrameContextFrom: null,
                    timeBase: videoStreamInfo.timeBase,
                    SAR: videoStreamInfo.SAR
                }
            ],
            [
                {
                    name: 'out1',
                    mediaType: Utau.Constants.MEDIA_TYPE_VIDEO
                }
            ]
        );
    }

    const filtered = videoFilter.filter([{
        name: 'in1',
        mediaType: Utau.Constants.MEDIA_TYPE_VIDEO,
        videoBuffer: buffer
    }]);

    buffer.dispose();

    const filteredVideoBuffer = filtered[0].videoBuffer;
    */

    const filteredVideoBuffer = buffer;

    const scene = new GL.SceneBuilder(width, height)
        .pushOffset(0, 0)
        .addVideoBuffer(filteredVideoBuffer, 0, 0, width, height, GL.Constants.SAMPLING_FILTER_LINEAR)
        .build();
    blender.update(scene).then(() => {
        scene.dispose();
    });

    filteredVideoBuffer.dispose();
};

surface.connect('keyboard-key', (key: GL.KeyboardKey, modifiers: GL.KeyboardModifiers, pressed: boolean) => {
    if (key == GL.Constants.KEY_P && pressed) {
        dispatcher.pause();
    } else if (key == GL.Constants.KEY_C && pressed) {
        dispatcher.play();
    }
});

surface.connect('close', () => {
    dispatcher.dispose();
    audioSinkStream.disconnect();
    audioSinkStream.dispose();
    blender.dispose().then(() => {
        surface.close();
    });
});

surface.connect('closed', () => {
    display.close();
});

display.connect('closed', () => {
    GL.RenderHost.Dispose();
});
