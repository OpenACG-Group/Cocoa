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
import * as GLUtils from '../GL/utils';
import {Window} from '../GL/window';

const MEDIA_FILE = '/home/sora/Music/CloudMusic/折戸伸治 - 縁.mp3';
const MEDIA_TITLE = '折戸伸治 - 縁';
const RENDER_FONT = '/home/sora/.local/share/fonts/Windows/萝莉体.ttc';

const COVER_WIDTH = 250, COVER_HEIGHT = 250;
const COVER_UPPER_MARGIN = 50;

const PANEL_BOTTOM_MARGIN = 240;
const PANEL_WIDTH = 500, PANEL_HEIGHT = 250;
const PANEL_PROGRESS_BAR_WIDTH = 400;

function extractMediaCoverImage(file: string): GL.CkImage {
    const decoder = Utau.AVStreamDecoder.MakeFromFile(file, {
        disableAudio: true
    });
    
    const coverFrame = decoder.decodeNextFrame();
    if (coverFrame.type != Utau.Constants.DECODE_BUFFER_VIDEO) {
        std.print('Media file has no cover image\n');
        return null;
    }

    return GL.CkImage.MakeFromVideoBuffer(coverFrame.videoBuffer);
}

function generateBackgroundImage(coverImage: GL.CkImage, w: number, h: number): GL.CkImage {
    const surface = GL.CkSurface.MakeRaster({
        alphaType: GL.Constants.ALPHA_TYPE_OPAQUE,
        colorType: GL.Constants.COLOR_TYPE_BGRA8888,
        colorSpace: GL.Constants.COLOR_SPACE_SRGB,
        width: w,
        height: h
    });
    const canvas = surface.getCanvas();
    const paint = new GL.CkPaint();
    paint.setImageFilter(GL.CkImageFilter.MakeFromDSL('blur(50, 50, _, _)', {}));

    canvas.drawImageRect(coverImage, [0, 0, coverImage.width, coverImage.height],
                         [0, 0, w, h], GL.Constants.SAMPLING_FILTER_LINEAR, paint,
                         GL.Constants.CANVAS_SRC_RECT_CONSTRAINT_FAST);
    
    return surface.makeImageSnapshot(null);
}

const typeface = GL.CkTypeface.MakeFromFile(RENDER_FONT, 0);
const titleFont = GL.CkFont.MakeFromSize(typeface, 18);
const progressBarFont = GL.CkFont.MakeFromSize(typeface, 14);

function measureText(text: string, font: GL.CkFont, paint: GL.CkPaint): GL.CkRect {
    return font.measureTextBounds(std.Buffer.MakeFromString(text, std.Buffer.ENCODE_UTF8),
                                  GL.Constants.TEXT_ENCODING_UTF8, paint);
}

function drawControlPanel(timestamp: number, totalTime: number, title: string): GL.CkPicture {
    const currentTimeStr = `${(timestamp / 60) | 0}:${(timestamp % 60) | 0}`;
    const totalTimeStr = `${(totalTime / 60) | 0}:${(totalTime % 60) | 0}`;

    const recorder = new GL.CkPictureRecorder();
    const canvas = recorder.beginRecording([0, 0, PANEL_WIDTH, PANEL_HEIGHT]);

    const paint = new GL.CkPaint();
    paint.setColor4f([1, 1, 1, 0.9]);
    paint.setAntiAlias(true);

    const titleBounds = measureText(MEDIA_TITLE, titleFont, paint);
    canvas.drawString(MEDIA_TITLE, (PANEL_WIDTH - titleBounds[2]) / 2, titleBounds[3], titleFont, paint);

    const curTimeBounds = measureText(currentTimeStr, progressBarFont, paint);
    const totalTimeBounds = measureText(totalTimeStr, progressBarFont, paint);

    const progressBarUpper = titleBounds[3] + 50;
    canvas.drawString(currentTimeStr, 0, progressBarUpper, progressBarFont, paint);
    canvas.drawString(totalTimeStr, PANEL_WIDTH - totalTimeBounds[2], progressBarUpper, progressBarFont, paint);

    paint.setStrokeWidth(2);
    paint.setStrokeCap(GL.Constants.PAINT_CAP_SQUARE);

    const t = timestamp / totalTime;
    const p1: GL.CkPoint = [(PANEL_WIDTH - PANEL_PROGRESS_BAR_WIDTH) / 2, progressBarUpper - 4];
    const p2: GL.CkPoint = [(PANEL_WIDTH - PANEL_PROGRESS_BAR_WIDTH) / 2 + t * PANEL_PROGRESS_BAR_WIDTH, progressBarUpper - 4];
    const p3: GL.CkPoint = [(PANEL_WIDTH + PANEL_PROGRESS_BAR_WIDTH) / 2, progressBarUpper - 4];
    canvas.drawLine(p1, p2, paint);

    paint.setColor4f([1, 1, 1, 0.5]);
    canvas.drawLine(p2, p3, paint);

    paint.setColor4f([1, 1, 1, 0.9]);
    canvas.drawCircle(p2[0], p2[1], 6, paint);

    return recorder.finishRecordingAsPicture();
}

class Application {
    width: number;
    height: number;
    display: GL.Display;
    window: Window;

    coverImage: GL.CkImage;
    coverImageTex: GL.TextureId;
    backgroundImageTex: GL.TextureId;

    totalTimeSec: number;
    lastTimestampSec: number;
    audioSinkStream: Utau.AudioSinkStream;
    dispatcher: Utau.MediaFramePresentDispatcher;

    constructor(width: number, height: number) {
        this.width = width;
        this.height = height;
        this.coverImage = null;
        this.backgroundImageTex = null;
    }

    private closeApplication(): void {
        this.dispatcher.dispose();
        this.audioSinkStream.disconnect();
        this.audioSinkStream.dispose();
        this.window.close().then(() => {
            this.display.close();
        });
    }

    private async createWindow(): Promise<void> {
        GLUtils.Initialize({name: 'Direct Music Player'});

        this.display = await GL.RenderHost.Connect();
        this.display.connect('closed', GL.RenderHost.Dispose);

        this.window = await Window.Create(this.display, this.width, this.height, true);
        this.window.surface.setTitle('Direct Music Player');
        this.window.surface.setMaxSize(this.width, this.height);
        this.window.surface.setMinSize(this.width, this.height);

        this.window.eventHandler = {
            onCloseRequest: () => {
                this.closeApplication();
            },
        };
    }

    render(timestamp: number): void {
        if (Math.floor(timestamp) == Math.floor(this.lastTimestampSec)) {
            return;
        }

        const pict = drawControlPanel(timestamp, this.totalTimeSec, MEDIA_FILE);

        const coverShadowFilter = GL.CkImageFilter.MakeFromDSL('drop_shadow(10, 10, 5, 5, [0, 0, 0, 0.7], _)', {});
        const coverLeft = (this.width - COVER_WIDTH) / 2;
        const coverTop = COVER_UPPER_MARGIN;
        const scene = new GL.SceneBuilder(this.width, this.height)
            .pushOffset(0, 0)
            .addTexture(this.backgroundImageTex, 0, 0, this.width, this.height, GL.Constants.SAMPLING_FILTER_LINEAR)

            .pushOffset(coverLeft, coverTop)
            .pushImageFilter(coverShadowFilter)
            .pushRRectClip({ rect: [0, 0, COVER_WIDTH, COVER_HEIGHT], uniformRadii: true, borderRadii: [20] }, true)
            .addTexture(this.coverImageTex, 0, 0, COVER_WIDTH, COVER_HEIGHT, GL.Constants.SAMPLING_FILTER_LINEAR)
            .pop()
            .pop()
            .pop()

            .pushOffset((this.width - PANEL_WIDTH) / 2, this.height - PANEL_BOTTOM_MARGIN)
            .addPicture(pict, false, 0, 0)

            .build();
        
        this.window.blender.update(scene).then(() => {
            scene.dispose();
        });

        this.lastTimestampSec = timestamp;
    }

    async run(): Promise<void> {
        await this.createWindow();

        this.coverImage = extractMediaCoverImage(MEDIA_FILE);
        if (this.coverImage) {
            const image = generateBackgroundImage(this.coverImage, this.width, this.height);
            this.coverImageTex = await this.window.blender.createTextureFromImage(this.coverImage, '#cover-image');
            this.backgroundImageTex = await this.window.blender.createTextureFromImage(image, '#cover-background');
        }

        const decoder = Utau.AVStreamDecoder.MakeFromFile(MEDIA_FILE, {disableVideo: true});
        const streamInfo = decoder.getStreamInfo(Utau.Constants.STREAM_SELECTOR_AUDIO);
        
        this.totalTimeSec = streamInfo.duration * streamInfo.timeBase.num / streamInfo.timeBase.denom;
        std.print(`Stream duration: ${this.totalTimeSec}s\n`);

        const audioDevice = Utau.AudioDevice.ConnectPipeWire();
        this.audioSinkStream = audioDevice.createSinkStream('Playback');
        audioDevice.unref();

        this.audioSinkStream.connect(streamInfo.sampleFormat, streamInfo.channelMode, streamInfo.sampleRate, true);

        this.dispatcher = new Utau.MediaFramePresentDispatcher(decoder, this.audioSinkStream);
        this.dispatcher.onErrorOrEOF = () => {
            std.print('EOF\n');
        };

        this.dispatcher.onAudioPresentNotify = (pts: number) => {
            this.render(pts);
        };

        this.dispatcher.play();
    }
}

const app = new Application(800, 600);
await app.run();
