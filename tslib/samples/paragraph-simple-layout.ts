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
import * as Paragraph from 'paragraph';
import {Window} from '../GL/window';
import {Initialize} from '../GL/utils';

const WINDOW_WIDTH = 512;
const WINDOW_HEIGHT = 512;

Initialize({name: 'SkNative'});
const display = await GL.RenderHost.Connect();
display.connect('closed', GL.RenderHost.Dispose);

const win = await Window.Create(display, WINDOW_WIDTH, WINDOW_HEIGHT, true);
win.eventHandler = {
    onCloseRequest: () => {
        win.close().then(() => {
            display.close();
        });
    },
    onPointerMotion: (x, y) => {
        render(x, y);
    }
};

const textStyle = new Paragraph.TextStyle();
textStyle.color = [0, 0, 0, 1];
textStyle.fontSize = 20;
textStyle.fontStyle = GL.CkFontStyle.MakeNormal();
textStyle.setFontFamilies(['Times New Roman']);

const paragraphStyle = new Paragraph.ParagraphStyle();
paragraphStyle.textStyle = textStyle;
paragraphStyle.textAlign = Paragraph.Constants.TEXT_ALIGN_LEFT;
paragraphStyle.maxLines = 100;
paragraphStyle.setEllipsis('...');

// Text from:
// Antoine de Saint-Exupéry: The Little Prince, Chapter 1
const CONTENT = 'The grown-ups\' response, this time, was to advise me to lay aside my drawings of boa constrictors,'
              + ' whether from the inside or the outside, and devote myself instead to geography, history, arithmetic and grammar.'
              + ' That is why, at the age of six, I gave up what might have been a magnificent career as a painter.'
              + ' I had been disheartened by the failure of my Drawing Number One and my Drawing Number Two.'
              + ' Grown-ups never understand anything by themselves,'
              + ' and it is tiresome for children to be always and forever explaining things to them. ';

const paragraph = Paragraph.ParagraphBuilder.Make(paragraphStyle, GL.defaultFontMgr)
    .addText(CONTENT)
    .build();

paragraph.layout(WINDOW_WIDTH - 20 * 2);

const font = GL.CkFont.MakeFromSize(GL.defaultFontMgr.matchFamilyStyle('Consolas', GL.CkFontStyle.MakeNormal()), 15);
const fontPaint = new GL.CkPaint();
fontPaint.setAntiAlias(true);
fontPaint.setColor4f([0, 0, 0, 1]);

function render(x: number, y: number): void {
    const recorder = new GL.CkPictureRecorder();
    const canvas = recorder.beginRecording([0, 0, 800, 600]);

    canvas.clear([1, 1, 1, 1]);
    paragraph.paint(canvas, 20, 20);

    const glyph = paragraph.getGlyphPositionAtCoordinate(x - 20, y - 20).position;
    canvas.drawString(`At (${x}, ${y}): glyph_position=${glyph} '${CONTENT[glyph]}'`,
                      20, WINDOW_HEIGHT - 50, font, fontPaint);

    const scene = new GL.SceneBuilder(800, 600)
        .pushOffset(0, 0)
        .addPicture(recorder.finishRecordingAsPicture(), false, 0, 0)
        .build();

    win.blender.update(scene).then(() => {
        scene.dispose();
    });
}

render(0, 0);
