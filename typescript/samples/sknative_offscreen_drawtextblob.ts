import * as std from 'core';
import * as GL from 'glamor';

const surface = GL.CkSurface.MakeRaster({
    colorType: GL.Constants.COLOR_TYPE_BGRA8888,
    alphaType: GL.Constants.ALPHA_TYPE_OPAQUE,
    colorSpace: GL.Constants.COLOR_SPACE_SRGB,
    width: 512,
    height: 512
});

const canvas = surface.getCanvas();

const typeface = GL.defaultFontMgr.matchFamilyStyle('KaiTi', GL.CkFontStyle.MakeNormal());
const font = GL.CkFont.MakeFromSize(typeface, 25);
const blob = GL.CkTextBlob.MakeFromText(std.Buffer.MakeFromString('江流宛转绕芳甸，月照花林皆似霰。', std.Buffer.ENCODE_UTF8),
                                        font, GL.Constants.TEXT_ENCODING_UTF8);

const paint = new GL.CkPaint();
paint.setAntiAlias(true);
paint.setColor4f([0, 0, 0, 1]);
paint.setStyle(GL.Constants.PAINT_STYLE_FILL);

canvas.clear([1, 1, 1, 1]);
canvas.drawTextBlob(blob, 10, 100, paint);

const image = surface.makeImageSnapshot(null);
std.File.WriteFileSync('result.png', image.encodeToData(GL.Constants.FORMAT_PNG, 100));
