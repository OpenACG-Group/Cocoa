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

export namespace VizPositioner {
    export enum LayoutAttributeMode {
        AUTO,
        INHERIT,
        CONSTANT
    }

    export interface LayoutAttribute {
        mode: LayoutAttributeMode;
        constant?: number;
    }

    export function MakeConstantLayoutAttr(value: number): LayoutAttribute {
        return {
            mode: LayoutAttributeMode.CONSTANT,
            constant: value
        };
    }

    export function MakeAutoLayoutAttr(): LayoutAttribute {
        return {
            mode: LayoutAttributeMode.AUTO
        };
    }

    export function MakeInheritLayoutAttr(): LayoutAttribute {
        return {
            mode: LayoutAttributeMode.INHERIT
        };
    }

    export interface Box {
        left: LayoutAttribute;
        top: LayoutAttribute;
        right: LayoutAttribute;
        bottom: LayoutAttribute;
        
        leftMargin: LayoutAttribute;
        topMargin: LayoutAttribute;
        rightMargin: LayoutAttribute;
        bottomMargin: LayoutAttribute;
    }

    export function MakeUniformBox(value: LayoutAttribute): Box {
        return {
            left:         Object.assign({}, value),
            top:          Object.assign({}, value),
            right:        Object.assign({}, value),
            bottom:       Object.assign({}, value),
            leftMargin:   Object.assign({}, value),
            topMargin:    Object.assign({}, value),
            rightMargin:  Object.assign({}, value),
            bottomMargin: Object.assign({}, value)
        };
    }

    export function MakeInheritBox(): Box {
        return MakeUniformBox({mode: LayoutAttributeMode.AUTO});
    }

    export function MakeAutoBox(): Box {
        return MakeUniformBox({mode: LayoutAttributeMode.INHERIT});
    }

    export function MakeUniformMarginBox(margin: number): Box {
        return {
            left:         {mode: LayoutAttributeMode.AUTO},
            top:          {mode: LayoutAttributeMode.AUTO},
            right:        {mode: LayoutAttributeMode.AUTO},
            bottom:       {mode: LayoutAttributeMode.AUTO},
            leftMargin:   {mode: LayoutAttributeMode.CONSTANT, constant: margin},
            topMargin:    {mode: LayoutAttributeMode.CONSTANT, constant: margin},
            rightMargin:  {mode: LayoutAttributeMode.CONSTANT, constant: margin},
            bottomMargin: {mode: LayoutAttributeMode.CONSTANT, constant: margin}
        };
    }
}
