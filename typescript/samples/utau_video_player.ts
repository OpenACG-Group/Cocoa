import * as utau from 'utau';
import * as GL from 'glamor';
import * as std from 'core';

// Read file
const d = utau.AVStreamDecoder.MakeFromFile(std.args[0], {useHWDecoding: false});

const vinfo = d.getStreamInfo(utau.Constants.STREAM_SELECTOR_VIDEO);
const ainfo = d.getStreamInfo(utau.Constants.STREAM_SELECTOR_AUDIO);

let vpW = vinfo.width, vpH = vinfo.height;

// Prepare GL context: display, surface, and blender
GL.RenderHost.Initialize({
    name: 'Video Player',
    major: 1,
    minor: 0,
    patch: 0
});

const display = await GL.RenderHost.Connect();
display.connect('closed', () => {
    GL.RenderHost.Dispose();
});

const surface = await display.createHWComposeSurface(vpW, vpH);
const blender = await surface.createBlender();

surface.setTitle(std.args[0]);

// Multimedia processing

const audioDevice = utau.AudioDevice.ConnectPipeWire();
const audioStream = audioDevice.createSinkStream('Playback');
audioDevice.unref();
audioStream.connect(ainfo.sampleFormat, ainfo.channelMode, ainfo.sampleRate, true);

function dispose(): void {
    blender.dispose();
    surface.close();
    display.close();
    audioStream.disconnect();
    audioStream.dispose();
}

const filter = GL.CkImageFilter.MakeFromDSL('blur(3, 3, _, _)', {});

const dispatcher = new utau.MediaFramePresentDispatcher(d, audioStream);
dispatcher.onPresentVideoBuffer = (buffer, pts) => {
    const scene = new GL.SceneBuilder(vpW, vpH)
        .pushOffset(0, 0)
        // .pushImageFilter(filter)
        .addVideoBuffer(buffer, 0, 0, vpW, vpH, GL.Constants.SAMPLING_FILTER_LINEAR)
        .build();

    buffer.dispose();
    blender.update(scene).then(() => { scene.dispose(); });
};

dispatcher.onErrorOrEOF = () => {
    dispatcher.dispose();
    dispose();
};

surface.connect('close', () => {
    dispatcher.pause();
    dispatcher.dispose();
    dispose();
});

surface.connect('keyboard-key', (key: GL.KeyboardKey,
                                 modifier: GL.KeyboardModifiers,
                                 pressed: boolean) => {
    if (!pressed)
        return;
    if (key == GL.Constants.KEY_P) {
        dispatcher.pause();
    } else if (key == GL.Constants.KEY_C) {
        dispatcher.play();
    }
});

surface.connect('configure', (w, h) => {
    vpW = w;
    vpH = h;
    surface.resize(vpW, vpH);
});

std.print(`Press C to play, P to pause...\n`);
