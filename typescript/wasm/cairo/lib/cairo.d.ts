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

export interface CairoLib {
    readonly Format: FormatEnumValues;
    readonly Content: ContentEnumValues;
    readonly Antialias: AntialiasEnumValues;
    readonly FillRule: FillRuleEnumValues;
    readonly LineCap: LineCapEnumValues;
    readonly LineJoin: LineJoinEnumValues;
    readonly Operator: OperatorEnumValues;
    readonly Extend: ExtendEnumValues;
    readonly Filter: FilterEnumValues;
    readonly PatternType: PatternTypeEnumValues;
    readonly Status: StatusEnumValues;

    malloc(ctor: TypedArrayConstructor, byteSize: number): MallocMemory;
    free(memory: MallocMemory): void;

    image_surface_create(width: number, height: number, memory: MallocMemory,
                         format: Format, stride: number): ImageSurface;

    recording_surface_create(content: Content, extents: Array<number> | null): RecordingSurface;

    cairo_create(surface: Surface): Cairo;

    pattern_create_rgb(r: number, g: number, b: number): Pattern;

    pattern_create_rgba(r: number, g: number, b: number, a: number): Pattern;

    pattern_create_for_surface(surface: Surface): Pattern;

    pattern_create_linear(x0: number, y0: number, x1: number, y1: number): Pattern;

    pattern_create_radial(cx0: number, cy0: number, radius0: number,
                          cx1: number, cy1: number, radius1: number): Pattern;

    pattern_create_mesh(): Pattern;

    script_interpreter_create(): ScriptInterpreter;
}

interface EmbindEnum {
    readonly values: number[];
}

interface EmbindEnumEntity<T> {
    readonly value: number;
}

export type Status = EmbindEnumEntity<StatusEnumValues>;
interface StatusEnumValues extends EmbindEnum {
    SUCCESS: Status;
    NO_MEMORY: Status;
    INVALID_RESTORE: Status;
    INVALID_POP_GROUP: Status;
    NO_CURRENT_POINT: Status;
    INVALID_MATRIX: Status;
    INVALID_STATUS: Status;
    NULL_POINTER: Status;
    INVALID_STRING: Status;
    INVALID_PATH_DATA: Status;
    READ_ERROR: Status;
    WRITE_ERROR: Status;
    SURFACE_FINISHED: Status;
    SURFACE_TYPE_MISMATCH: Status;
    PATTERN_TYPE_MISMATCH: Status;
    INVALID_CONTENT: Status;
    INVALID_FORMAT: Status;
    INVALID_VISUAL: Status;
    FILE_NOT_FOUND: Status;
    INVALID_DASH: Status;
    INVALID_DSC_COMMENT: Status;
    INVALID_INDEX: Status;
    CLIP_NOT_REPRESENTABLE: Status;
    TEMP_FILE_ERROR: Status;
    INVALID_STRIDE: Status;
    FONT_TYPE_MISMATCH: Status;
    USER_FONT_IMMUTABLE: Status;
    USER_FONT_ERROR: Status;
    NEGATIVE_COUNT: Status;
    INVALID_CLUSTERS: Status;
    INVALID_SLANT: Status;
    INVALID_WEIGHT: Status;
    INVALID_SIZE: Status;
    USER_FONT_NOT_IMPLEMENTED: Status;
    DEVICE_TYPE_MISMATCH: Status;
    DEVICE_ERROR: Status;
    INVALID_MESH_CONSTRUCTION: Status;
    DEVICE_FINISHED: Status;
    JBIG2_GLOBAL_MISSING: Status;
    PNG_ERROR: Status;
    FREETYPE_ERROR: Status;
    WIN32_GDI_ERROR: Status;
    TAG_ERROR: Status;
    DWRITE_ERROR: Status;
    SVG_FONT_ERROR: Status;
}

export type Format = EmbindEnumEntity<FormatEnumValues>;
interface FormatEnumValues extends EmbindEnum {
    INVALID: Format;
    ARGB32: Format;
    RGB24: Format;
    A8: Format;
    A1: Format;
    RGB16_565: Format;
    RGB30: Format;
    RGB96F: Format;
    RGBA128F: Format;
}

export type Content = EmbindEnumEntity<ContentEnumValues>;
interface ContentEnumValues extends EmbindEnum {
    ALPHA: Content;
    COLOR: Content;
    COLOR_ALPHA: Content;
}

export type Antialias = EmbindEnumEntity<ContentEnumValues>;
interface AntialiasEnumValues extends EmbindEnum {
    DEFAULT: Antialias;
    NONE: Antialias;
    GRAY: Antialias;
    SUBPIXEL: Antialias;
    FAST: Antialias;
    GOOD: Antialias;
    BEST: Antialias;
}

export type FillRule = EmbindEnumEntity<FillRuleEnumValues>;
interface FillRuleEnumValues extends EmbindEnum {
    EVEN_ODD: FillRule;
    WINDING: FillRule;
}

export type LineCap = EmbindEnumEntity<LineCapEnumValues>;
interface LineCapEnumValues extends EmbindEnum {
    BUTT: LineCap;
    SQUARE: LineCap;
    ROUND: LineCap;
}

export type LineJoin = EmbindEnumEntity<LineJoinEnumValues>;
interface LineJoinEnumValues extends EmbindEnum {
    BEVEL: LineJoin;
    MITER: LineJoin;
    ROUND: LineJoin;
}

export type Operator = EmbindEnumEntity<OperatorEnumValues>;
interface OperatorEnumValues extends EmbindEnum {
    CLEAR: Operator;
    SOURCE: Operator;
    OVER: Operator;
    IN: Operator;
    OUT: Operator;
    ATOP: Operator;
    DEST: Operator;
    DEST_OVER: Operator;
    DEST_IN: Operator;
    DEST_OUT: Operator;
    DEST_ATOP: Operator;
    XOR: Operator;
    ADD: Operator;
    SATURATE: Operator;
    MULTIPLY: Operator;
    SCREEN: Operator;
    OVERLAY: Operator;
    DARKEN: Operator;
    LIGHTEN: Operator;
    COLOR_DODGE: Operator;
    COLOR_BURN: Operator;
    HARD_LIGHT: Operator;
    SOFT_LIGHT: Operator;
    DIFFERENCE: Operator;
    EXCLUSION: Operator;
    HSL_HUE: Operator;
    HSL_SATURATION: Operator;
    HSL_COLOR: Operator;
    HSL_LUMINOSITY: Operator;
}

export type Extend = EmbindEnumEntity<ExtendEnumValues>;
interface ExtendEnumValues extends EmbindEnum {
    NONE: Extend;
    REPEAT: Extend;
    REFLECT: Extend;
    PAD: Extend;
}

export type Filter = EmbindEnumEntity<FilterEnumValues>;
interface FilterEnumValues extends EmbindEnum {
    FAST: Filter;
    GOOD: Filter;
    BEST: Filter;
    NEAREST: Filter;
    BILINEAR: Filter;
    GAUSSIAN: Filter;
}

export type PatternType = EmbindEnumEntity<PatternTypeEnumValues>;
interface PatternTypeEnumValues extends EmbindEnum {
    SOLID: PatternType;
    SURFACE: PatternType;
    LINEAR: PatternType;
    RADIAL: PatternType;
    MESH: PatternType;
    RASTER_SOURCE: PatternType;
}

export type TypedArray = Float32Array | Int32Array | Uint32Array | Int16Array | Uint16Array
                       | Int8Array | Uint8Array;
export type TypedArrayConstructor = Float32ArrayConstructor
                                  | Int32ArrayConstructor
                                  | Uint32ArrayConstructor
                                  | Int16ArrayConstructor
                                  | Uint16ArrayConstructor
                                  | Int8ArrayConstructor
                                  | Uint8ArrayConstructor;

export interface MallocMemory {
    readonly length: number;
    readonly byteOffset: number;
    subarray(start: number, end: number): TypedArray;
    toTypedArray(): TypedArray;
}

export interface EmbindObject<T extends EmbindObject<T>> {
    clone(): T;
    delete(): void;
    deleteLater(): void;
    isAliasOf(other: any): boolean;
    isDeleted(): boolean;
}

export interface Surface extends EmbindObject<Surface> {
    flush(): void;
    finish(): void;
    mark_dirty(): void;
}

export interface ImageSurface extends Surface {
    get_format(): Format;
    get_width(): number;
    get_height(): number;
}

export interface RecordingSurface extends Surface {
    ink_extents(): Array<number>;
    get_extents(): Array<number> | null;
}

export interface Pattern extends EmbindObject<Pattern> {
    add_color_stop_rgb(offset: number, r: number, g: number, b: number): void;
    add_color_stop_rgba(offset: number, r: number, g: number, b: number, a: number): void;
    get_color_stop_count(): number;
    get_surface(): Surface;
    mesh_begin_patch(): void;
    mesh_end_patch(): void;
    mesh_move_to(x: number, y: number): void;
    mesh_line_to(x: number, y: number): void;
    mesh_curve_to(x1: number, y1: number, x2: number, y2: number, x3: number, y3: number): void;
    mesh_set_control_point(pointNum: number, x: number, y: number): void;
    mesh_set_corner_color_rgb(cornerNum: number, r: number, g: number, b: number): void;
    mesh_set_corner_color_rgba(cornerNum: number, r: number, g: number, b: number, a: number): void;
    mesh_get_patch_count(): number;
    set_extend(extend: Extend): void;
    set_filter(filter: Filter): void;
    get_extend(): Extend;
    get_filter(): Filter;
    get_type(): PatternType;
}

export interface Cairo extends EmbindObject<Cairo> {
    get_target(): Surface;

    save(): void;
    restore(): void;

    push_group(): void;
    push_group_with_content(content: Content): void;
    pop_group(): Pattern;
    pop_group_to_source(): void;
    get_group_target(): Surface;

    set_source_rgb(r: number, g: number, b: number): void;
    set_source_rgba(r: number, g: number, b: number, a: number): void;
    set_source(pattern: Pattern): void;
    get_source(): Pattern;
    set_source_surface(surface: Surface, x: number, y: number): void;

    set_antialias(antialias: Antialias): void;
    get_antialias(): Antialias;
    set_dash(dashes: Array<number>, offset: number): void;
    get_dash_count(): number;
    set_fill_rule(rule: FillRule): void;
    get_fill_rule(): FillRule;
    set_line_cap(cap: LineCap): void;
    get_line_cap(): LineCap;
    set_line_join(join: LineJoin): void;
    get_line_join(): LineJoin;
    set_line_width(width: number): void;
    get_line_width(): number;
    set_miter_limit(limit: number): void;
    get_miter_limit(): number;
    set_operator(op: Operator): void;
    get_operator(): Operator;
    set_tolerance(tolerance: number): void;
    get_tolerance(): number;

    clip(): void;
    clip_preserve(): void;
    clip_extents(): Array<number>;
    in_clip(x: number, y: number): boolean;
    reset_clip(): void;
    fill(): void;
    fill_preserve(): void;
    fill_extents(): Array<number>;
    in_fill(x: number, y: number): boolean;
    mask(pattern: Pattern): void;
    mask_surface(surface: Surface, surfaceX: number, surfaceY: number): void;
    paint(): void;
    paint_with_alpha(a: number): void;
    stroke(): void;
    stroke_preserve(): void;
    stroke_extents(): Array<number>;
    in_stroke(x: number, y: number): boolean;
    copy_page(): void;
    show_page(): void;

    translate(tx: number, ty: number): void;
    scale(sx: number, sy: number): void;
    rotate(angle: number): void;
    identity_matrix(): void;

    new_path(): void;
    move_to(x: number, y: number): void;
    new_sub_path(): void;
    line_to(x: number, y: number): void;
    curve_to(x1: number, y1: number,  x2: number, y2: number,  x3: number, y3: number): void;
    arc(xc: number, yc: number, radius: number, angle1: number, angle2: number): void;
    arc_negative(xc: number, yc: number, radius: number, angle1: number, angle2: number): void;
    rel_move_to(dx: number, dy: number): void;
    rel_line_to(dx: number, dy: number): void;
    rel_curve_to(dx1: number, dy1: number, dx2: number, dy2: number, dx3: number, dy3: number): void;
    rectangle(x: number, y: number, width: number, height: number): void;
    close_path(): void;
    path_extents(): Array<number>;

    tag_begin(tagName: string, attributes: string): void;
    tag_end(tagName: string): void;
}

export interface ScriptInterpreterHooks {
    surface_create?: (content: Content, width: number, height: number, uid: number) => Surface;
}

export interface ScriptInterpreter extends EmbindObject<ScriptInterpreter> {
    install_hooks(hooks: ScriptInterpreterHooks): void;
    feed_string(source: string): Status;
    finish(): Status;
}
