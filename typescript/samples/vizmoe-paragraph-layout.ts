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
import * as gl from 'glamor';
import * as Para from 'paragraph';
import {
    ToplevelWindow,
    CloseRequestEvent,
    KeyboardKeyEvent,
    KeyboardKey
} from '../vizmoe/render/toplevel-window';
import { CompositeRenderNode, PaintRenderNode } from '../vizmoe/render/render-node';
import { DrawContextSubmitter } from '../vizmoe/render/draw-context-submitter';
import { Rect } from '../vizmoe/render/rectangle';
import { Vector2f } from '../vizmoe/render/vector';

const CONTENTS = [
    // Text from:
    // Antoine de Saint-Exupéry: The Little Prince, Chapter 1
    `The grown-ups' response, this time, was to advise me to lay aside my drawings of boa \
constrictors, whether from the inside or the outside, and devote myself instead to geography, \
history, arithmetic and grammar. That is why, at the age of six, I gave up what might have \
been a magnificent career as a painter. I had been disheartened by the failure of my Drawing \
Number One and my Drawing Number Two. Grown-ups never understand anything by themselves, \
and it is tiresome for children to be always and forever explaining things to them.`,

    // Text from:
    // Chinese ancient poetry collection "The Book of Odes":
    // 《诗经·小雅·采薇》
    `采薇采薇，薇亦作止。曰归曰归，岁亦莫止。靡室靡家，猃狁之故。不遑启居，猃狁之故。\
采薇采薇，薇亦柔止。曰归曰归，心亦忧止。忧心烈烈，载饥载渴。我戍未定，靡使归聘。\
采薇采薇，薇亦刚止。曰归曰归，岁亦阳止。王事靡盬，不遑启处。忧心孔疚，我行不来！\
彼尔维何？维常之华。彼路斯何？君子之车。戎车既驾，四牡业业。岂敢定居？一月三捷。\
驾彼四牡，四牡骙骙。君子所依，小人所腓。四牡翼翼，象弭鱼服。岂不日戒？猃狁孔棘！\
昔我往矣，杨柳依依。今我来思，雨雪霏霏。行道迟迟，载渴载饥。我心伤悲，莫知我哀！`,

    // Text from:
    // 「永訣の朝 (the morning when we are apart forever)」 written by 宮沢賢治
    `けふのうちに / とほくへいってしまふわたくしのいもうとよ / \
みぞれがふっておもてはへんにあかるいのだ / （あめゆじゅとてちてけんじゃ） / \
うすあかくいっさう阴惨な云から / みぞれはびちょびちょふってくる / \
（あめゆじゅとてちてけんじゃ） / 青い莼菜のもやうのついた / \
これらふたつのかけた陶椀に / おまへがたべるあめゆきをとらうとして / \
わたくしはまがったてっぽうだまのやうに / このくらいみぞれのなかに飞びだした / \
（あめゆじゅとてちてけんじゃ） / 苍铅いろの暗い云から / \
みぞれはびちょびちょ沈んでくる / ああとし子 / \
死ぬといふいまごろになって / わたくしをいっしゃうあかるくするために / \
こんなさっぱりした雪のひとわんを / おまへはわたくしにたのんだのだ / \
ありがたうわたくしのけなげないもうとよ / わたくしもまっすぐにすすんでいくから`

];

const SIDE_PADDING = 20;

class RenderContext {
    private readonly fWidth: number;
    private readonly fHeight: number;
    private readonly fParagraphs: Para.Paragraph[];
    private readonly fRootNode: CompositeRenderNode;
    private readonly fPaintNode: PaintRenderNode;
    private fCurrentText: number;

    constructor(width: number, height: number) {
        this.fCurrentText = 0;
        this.fWidth = width;
        this.fHeight = height;

        const textStyle = new Para.TextStyle();
        textStyle.color = [0, 0, 0, 1];
        textStyle.fontSize = 20;
        textStyle.fontStyle = gl.CkFontStyle.MakeNormal();
        textStyle.setFontFamilies(['Times New Roman', 'KaiTi']);

        const paragraphStyle = new Para.ParagraphStyle();
        paragraphStyle.textStyle = textStyle;
        paragraphStyle.textAlign = Para.Constants.TEXT_ALIGN_LEFT;
        paragraphStyle.maxLines = 50;
        paragraphStyle.setEllipsis('...');

        this.fParagraphs = [];
        for (const content of CONTENTS) {
            const paragraph = Para.ParagraphBuilder.Make(paragraphStyle, gl.defaultFontMgr)
                .addText(content)
                .build()

            paragraph.layout(width - SIDE_PADDING * 2);
            this.fParagraphs.push(paragraph);
        }

        this.fRootNode = new CompositeRenderNode();

        const bgNode = new PaintRenderNode();
        bgNode.update(Rect.MakeWH(width, height), C => C.clear([1, 1, 1, 1]));
        this.fRootNode.appendChild(bgNode);

        this.fPaintNode = new PaintRenderNode();

        this.fRootNode.appendChild(this.fPaintNode);
    }

    public render(submitter: DrawContextSubmitter): void {
        const para = this.fParagraphs[this.fCurrentText];
        const bounds = Rect.MakeXYWH(SIDE_PADDING, SIDE_PADDING, para.maxWidth, para.height);
        this.fPaintNode.update(bounds, C => para.paint(C, 0, 0) );

        submitter.submit(this.fRootNode, null, false);
    }

    public nextContent(): void {
        this.fCurrentText = (this.fCurrentText + 1) % this.fParagraphs.length;
    }
}

gl.RenderHost.Initialize({name: 'Paragraph', major: 1, minor: 0, patch: 0});

gl.RenderHost.Connect().then((display) => {
    return ToplevelWindow.Create(display, 500, 600, true)
}).then((window) => {
    const ctx = new RenderContext(window.width, window.height);

    window.addEventListener(CloseRequestEvent, async event => {
        const display = window.display;
        await window.close();
        await display.close();
        gl.RenderHost.Dispose();
    });

    window.addEventListener(KeyboardKeyEvent, event => {
        if (event.pressed || event.key != KeyboardKey.kENTER) {
            return;
        }
        ctx.nextContent();
        ctx.render(event.window.drawContext.submitter);
    });

    ctx.render(window.drawContext.submitter);
});
