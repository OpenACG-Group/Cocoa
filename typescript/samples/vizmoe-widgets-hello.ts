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

import * as gl from 'glamor';
import * as std from 'core';
import { CloseRequestEvent, ToplevelWindow } from "../vizmoe/render/toplevel-window";

import { LayoutConstraint } from '../vizmoe/render/widget-base';
import { View, ConstraintBox, Center, CenterAnchor, Align, AlignMode, VBoxLayout, HBoxLayout, HBoxLayoutOrder, VBoxLayoutOrder } from '../vizmoe/render/widgets-trivial-container';
import { Solid } from '../vizmoe/render/widgets-drawable-solid';
import { Space } from '../vizmoe/render/widgets-helper';
import { EdgeInsets } from '../vizmoe/render/edge-insets';
import * as color from '../vizmoe/render/color';
import { Rect } from '../vizmoe/render/rectangle';
import { Vector2f } from '../vizmoe/render/vector';

gl.RenderHost.Initialize({name: 'Widgets', major: 1, minor: 0, patch: 0});

gl.RenderHost.Connect().then((display) => {
    return ToplevelWindow.Create(display, 800, 600, true);
}).then((window) => {
    window.addEventListener(CloseRequestEvent, async event => {
        const display = window.display;
        await window.close();
        await display.close();
        gl.RenderHost.Dispose();
    });

    const NewConstraintSolid = (color: color.Color4f, w: number, h: number) => {
        return new ConstraintBox({
            widthScalar: w,
            heightScalar: h,
            child: new Solid({ color: color, margin: EdgeInsets.Uniform(3) })
        });
    };

    const view = new View({
        child: new Center({
            anchor: CenterAnchor.kBidirectional,
            child: new ConstraintBox({
                widthScalar: 0.5,
                heightScalar: 0.5,
                child: new VBoxLayout({
                    children: [
                        NewConstraintSolid(color.Const.kColorMagentaF, 1, 0.3),
                        new HBoxLayout({
                            order: HBoxLayoutOrder.kRTL,
                            children: [
                                NewConstraintSolid(color.Const.kColorRedF, 0.2, 1),
                                NewConstraintSolid(color.Const.kColorBlueF, 0.25, 1),
                                // NewConstraintSolid(color.Const.kColorGreenF, 0.33),

                                new Solid({color: color.Const.kColorCyanF, margin: EdgeInsets.Uniform(3) })
                            ]
                        })
                    ]
                })
            })
        })
    });

    view.layout(new LayoutConstraint(0, 0, 800, 600));
    window.drawContext.submitter.submit(view.render(new Vector2f(0, 0)), undefined, false, std.print);
});
