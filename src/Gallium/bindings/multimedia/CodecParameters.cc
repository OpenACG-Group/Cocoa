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

#include <map>
#include <vector>

#include "fmt/format.h"

#include "Core/Utils.h"
#include "Core/Errors.h"
#include "Gallium/bindings/multimedia/CodecParameters.h"
#include "Gallium/bindings/multimedia/AChannelLayout.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

namespace {

struct NamedCodec
{
    AVCodecID id;
    const char *name;
};

const std::vector<NamedCodec> g_codec_names{
    { AV_CODEC_ID_MPEG1VIDEO, "mpeg1video" },
    { AV_CODEC_ID_MPEG2VIDEO, "mpeg2video" },
    { AV_CODEC_ID_H261, "h261" },
    { AV_CODEC_ID_H263, "h263" },
    { AV_CODEC_ID_RV10, "rv10" },
    { AV_CODEC_ID_RV20, "rv20" },
    { AV_CODEC_ID_MJPEG, "mjpeg" },
    { AV_CODEC_ID_MJPEGB, "mjpegb" },
    { AV_CODEC_ID_LJPEG, "ljpeg" },
    { AV_CODEC_ID_SP5X, "sp5x" },
    { AV_CODEC_ID_JPEGLS, "jpegls" },
    { AV_CODEC_ID_MPEG4, "mpeg4" },
    { AV_CODEC_ID_RAWVIDEO, "rawvideo" },
    { AV_CODEC_ID_MSMPEG4V1, "msmpeg4v1" },
    { AV_CODEC_ID_MSMPEG4V2, "msmpeg4v2" },
    { AV_CODEC_ID_MSMPEG4V3, "msmpeg4v3" },
    { AV_CODEC_ID_WMV1, "wmv1" },
    { AV_CODEC_ID_WMV2, "wmv2" },
    { AV_CODEC_ID_H263P, "h263p" },
    { AV_CODEC_ID_H263I, "h263i" },
    { AV_CODEC_ID_FLV1, "flv1" },
    { AV_CODEC_ID_SVQ1, "svq1" },
    { AV_CODEC_ID_SVQ3, "svq3" },
    { AV_CODEC_ID_DVVIDEO, "dvvideo" },
    { AV_CODEC_ID_HUFFYUV, "huffyuv" },
    { AV_CODEC_ID_CYUV, "cyuv" },
    { AV_CODEC_ID_H264, "h264" },
    { AV_CODEC_ID_INDEO3, "indeo3" },
    { AV_CODEC_ID_VP3, "vp3" },
    { AV_CODEC_ID_THEORA, "theora" },
    { AV_CODEC_ID_ASV1, "asv1" },
    { AV_CODEC_ID_ASV2, "asv2" },
    { AV_CODEC_ID_FFV1, "ffv1" },
    { AV_CODEC_ID_4XM, "4xm" },
    { AV_CODEC_ID_VCR1, "vcr1" },
    { AV_CODEC_ID_CLJR, "cljr" },
    { AV_CODEC_ID_MDEC, "mdec" },
    { AV_CODEC_ID_ROQ, "roq" },
    { AV_CODEC_ID_INTERPLAY_VIDEO, "interplay_video" },
    { AV_CODEC_ID_XAN_WC3, "xan_wc3" },
    { AV_CODEC_ID_XAN_WC4, "xan_wc4" },
    { AV_CODEC_ID_RPZA, "rpza" },
    { AV_CODEC_ID_CINEPAK, "cinepak" },
    { AV_CODEC_ID_WS_VQA, "ws_vqa" },
    { AV_CODEC_ID_MSRLE, "msrle" },
    { AV_CODEC_ID_MSVIDEO1, "msvideo1" },
    { AV_CODEC_ID_IDCIN, "idcin" },
    { AV_CODEC_ID_8BPS, "8bps" },
    { AV_CODEC_ID_SMC, "smc" },
    { AV_CODEC_ID_FLIC, "flic" },
    { AV_CODEC_ID_TRUEMOTION1, "truemotion1" },
    { AV_CODEC_ID_VMDVIDEO, "vmdvideo" },
    { AV_CODEC_ID_MSZH, "mszh" },
    { AV_CODEC_ID_ZLIB, "zlib" },
    { AV_CODEC_ID_QTRLE, "qtrle" },
    { AV_CODEC_ID_TSCC, "tscc" },
    { AV_CODEC_ID_ULTI, "ulti" },
    { AV_CODEC_ID_QDRAW, "qdraw" },
    { AV_CODEC_ID_VIXL, "vixl" },
    { AV_CODEC_ID_QPEG, "qpeg" },
    { AV_CODEC_ID_PNG, "png" },
    { AV_CODEC_ID_PPM, "ppm" },
    { AV_CODEC_ID_PBM, "pbm" },
    { AV_CODEC_ID_PGM, "pgm" },
    { AV_CODEC_ID_PGMYUV, "pgmyuv" },
    { AV_CODEC_ID_PAM, "pam" },
    { AV_CODEC_ID_FFVHUFF, "ffvhuff" },
    { AV_CODEC_ID_RV30, "rv30" },
    { AV_CODEC_ID_RV40, "rv40" },
    { AV_CODEC_ID_VC1, "vc1" },
    { AV_CODEC_ID_WMV3, "wmv3" },
    { AV_CODEC_ID_LOCO, "loco" },
    { AV_CODEC_ID_WNV1, "wnv1" },
    { AV_CODEC_ID_AASC, "aasc" },
    { AV_CODEC_ID_INDEO2, "indeo2" },
    { AV_CODEC_ID_FRAPS, "fraps" },
    { AV_CODEC_ID_TRUEMOTION2, "truemotion2" },
    { AV_CODEC_ID_BMP, "bmp" },
    { AV_CODEC_ID_CSCD, "cscd" },
    { AV_CODEC_ID_MMVIDEO, "mmvideo" },
    { AV_CODEC_ID_ZMBV, "zmbv" },
    { AV_CODEC_ID_AVS, "avs" },
    { AV_CODEC_ID_SMACKVIDEO, "smackvideo" },
    { AV_CODEC_ID_NUV, "nuv" },
    { AV_CODEC_ID_KMVC, "kmvc" },
    { AV_CODEC_ID_FLASHSV, "flashsv" },
    { AV_CODEC_ID_CAVS, "cavs" },
    { AV_CODEC_ID_JPEG2000, "jpeg2000" },
    { AV_CODEC_ID_VMNC, "vmnc" },
    { AV_CODEC_ID_VP5, "vp5" },
    { AV_CODEC_ID_VP6, "vp6" },
    { AV_CODEC_ID_VP6F, "vp6f" },
    { AV_CODEC_ID_TARGA, "targa" },
    { AV_CODEC_ID_DSICINVIDEO, "dsicinvideo" },
    { AV_CODEC_ID_TIERTEXSEQVIDEO, "tiertexseqvideo" },
    { AV_CODEC_ID_TIFF, "tiff" },
    { AV_CODEC_ID_GIF, "gif" },
    { AV_CODEC_ID_DXA, "dxa" },
    { AV_CODEC_ID_DNXHD, "dnxhd" },
    { AV_CODEC_ID_THP, "thp" },
    { AV_CODEC_ID_SGI, "sgi" },
    { AV_CODEC_ID_C93, "c93" },
    { AV_CODEC_ID_BETHSOFTVID, "bethsoftvid" },
    { AV_CODEC_ID_PTX, "ptx" },
    { AV_CODEC_ID_TXD, "txd" },
    { AV_CODEC_ID_VP6A, "vp6a" },
    { AV_CODEC_ID_AMV, "amv" },
    { AV_CODEC_ID_VB, "vb" },
    { AV_CODEC_ID_PCX, "pcx" },
    { AV_CODEC_ID_SUNRAST, "sunrast" },
    { AV_CODEC_ID_INDEO4, "indeo4" },
    { AV_CODEC_ID_INDEO5, "indeo5" },
    { AV_CODEC_ID_MIMIC, "mimic" },
    { AV_CODEC_ID_RL2, "rl2" },
    { AV_CODEC_ID_ESCAPE124, "escape124" },
    { AV_CODEC_ID_DIRAC, "dirac" },
    { AV_CODEC_ID_BFI, "bfi" },
    { AV_CODEC_ID_CMV, "cmv" },
    { AV_CODEC_ID_MOTIONPIXELS, "motionpixels" },
    { AV_CODEC_ID_TGV, "tgv" },
    { AV_CODEC_ID_TGQ, "tgq" },
    { AV_CODEC_ID_TQI, "tqi" },
    { AV_CODEC_ID_AURA, "aura" },
    { AV_CODEC_ID_AURA2, "aura2" },
    { AV_CODEC_ID_V210X, "v210x" },
    { AV_CODEC_ID_TMV, "tmv" },
    { AV_CODEC_ID_V210, "v210" },
    { AV_CODEC_ID_DPX, "dpx" },
    { AV_CODEC_ID_MAD, "mad" },
    { AV_CODEC_ID_FRWU, "frwu" },
    { AV_CODEC_ID_FLASHSV2, "flashsv2" },
    { AV_CODEC_ID_CDGRAPHICS, "cdgraphics" },
    { AV_CODEC_ID_R210, "r210" },
    { AV_CODEC_ID_ANM, "anm" },
    { AV_CODEC_ID_BINKVIDEO, "binkvideo" },
    { AV_CODEC_ID_IFF_ILBM, "iff_ilbm" },
    { AV_CODEC_ID_IFF_BYTERUN1, "iff_byterun1" },
    { AV_CODEC_ID_KGV1, "kgv1" },
    { AV_CODEC_ID_YOP, "yop" },
    { AV_CODEC_ID_VP8, "vp8" },
    { AV_CODEC_ID_PICTOR, "pictor" },
    { AV_CODEC_ID_ANSI, "ansi" },
    { AV_CODEC_ID_A64_MULTI, "a64_multi" },
    { AV_CODEC_ID_A64_MULTI5, "a64_multi5" },
    { AV_CODEC_ID_R10K, "r10k" },
    { AV_CODEC_ID_MXPEG, "mxpeg" },
    { AV_CODEC_ID_LAGARITH, "lagarith" },
    { AV_CODEC_ID_PRORES, "prores" },
    { AV_CODEC_ID_JV, "jv" },
    { AV_CODEC_ID_DFA, "dfa" },
    { AV_CODEC_ID_WMV3IMAGE, "wmv3image" },
    { AV_CODEC_ID_VC1IMAGE, "vc1image" },
    { AV_CODEC_ID_UTVIDEO, "utvideo" },
    { AV_CODEC_ID_BMV_VIDEO, "bmv_video" },
    { AV_CODEC_ID_VBLE, "vble" },
    { AV_CODEC_ID_DXTORY, "dxtory" },
    { AV_CODEC_ID_V410, "v410" },
    { AV_CODEC_ID_XWD, "xwd" },
    { AV_CODEC_ID_CDXL, "cdxl" },
    { AV_CODEC_ID_XBM, "xbm" },
    { AV_CODEC_ID_ZEROCODEC, "zerocodec" },
    { AV_CODEC_ID_MSS1, "mss1" },
    { AV_CODEC_ID_MSA1, "msa1" },
    { AV_CODEC_ID_TSCC2, "tscc2" },
    { AV_CODEC_ID_MTS2, "mts2" },
    { AV_CODEC_ID_CLLC, "cllc" },
    { AV_CODEC_ID_MSS2, "mss2" },
    { AV_CODEC_ID_VP9, "vp9" },
    { AV_CODEC_ID_AIC, "aic" },
    { AV_CODEC_ID_ESCAPE130, "escape130" },
    { AV_CODEC_ID_G2M, "g2m" },
    { AV_CODEC_ID_WEBP, "webp" },
    { AV_CODEC_ID_HNM4_VIDEO, "hnm4_video" },
    { AV_CODEC_ID_HEVC, "hevc" },
    { AV_CODEC_ID_H265, "h265" },
    { AV_CODEC_ID_FIC, "fic" },
    { AV_CODEC_ID_ALIAS_PIX, "alias_pix" },
    { AV_CODEC_ID_BRENDER_PIX, "brender_pix" },
    { AV_CODEC_ID_PAF_VIDEO, "paf_video" },
    { AV_CODEC_ID_EXR, "exr" },
    { AV_CODEC_ID_VP7, "vp7" },
    { AV_CODEC_ID_SANM, "sanm" },
    { AV_CODEC_ID_SGIRLE, "sgirle" },
    { AV_CODEC_ID_MVC1, "mvc1" },
    { AV_CODEC_ID_MVC2, "mvc2" },
    { AV_CODEC_ID_HQX, "hqx" },
    { AV_CODEC_ID_TDSC, "tdsc" },
    { AV_CODEC_ID_HQ_HQA, "hq_hqa" },
    { AV_CODEC_ID_HAP, "hap" },
    { AV_CODEC_ID_DDS, "dds" },
    { AV_CODEC_ID_DXV, "dxv" },
    { AV_CODEC_ID_SCREENPRESSO, "screenpresso" },
    { AV_CODEC_ID_RSCC, "rscc" },
    { AV_CODEC_ID_AVS2, "avs2" },
    { AV_CODEC_ID_PGX, "pgx" },
    { AV_CODEC_ID_AVS3, "avs3" },
    { AV_CODEC_ID_MSP2, "msp2" },
    { AV_CODEC_ID_VVC, "vvc" },
    { AV_CODEC_ID_H266, "h266" },
    { AV_CODEC_ID_Y41P, "y41p" },
    { AV_CODEC_ID_AVRP, "avrp" },
    { AV_CODEC_ID_012V, "012v" },
    { AV_CODEC_ID_AVUI, "avui" },
    { AV_CODEC_ID_TARGA_Y216, "targa_y216" },
    { AV_CODEC_ID_V308, "v308" },
    { AV_CODEC_ID_V408, "v408" },
    { AV_CODEC_ID_YUV4, "yuv4" },
    { AV_CODEC_ID_AVRN, "avrn" },
    { AV_CODEC_ID_CPIA, "cpia" },
    { AV_CODEC_ID_XFACE, "xface" },
    { AV_CODEC_ID_SNOW, "snow" },
    { AV_CODEC_ID_SMVJPEG, "smvjpeg" },
    { AV_CODEC_ID_APNG, "apng" },
    { AV_CODEC_ID_DAALA, "daala" },
    { AV_CODEC_ID_CFHD, "cfhd" },
    { AV_CODEC_ID_TRUEMOTION2RT, "truemotion2rt" },
    { AV_CODEC_ID_M101, "m101" },
    { AV_CODEC_ID_MAGICYUV, "magicyuv" },
    { AV_CODEC_ID_SHEERVIDEO, "sheervideo" },
    { AV_CODEC_ID_YLC, "ylc" },
    { AV_CODEC_ID_PSD, "psd" },
    { AV_CODEC_ID_PIXLET, "pixlet" },
    { AV_CODEC_ID_SPEEDHQ, "speedhq" },
    { AV_CODEC_ID_FMVC, "fmvc" },
    { AV_CODEC_ID_SCPR, "scpr" },
    { AV_CODEC_ID_CLEARVIDEO, "clearvideo" },
    { AV_CODEC_ID_XPM, "xpm" },
    { AV_CODEC_ID_AV1, "av1" },
    { AV_CODEC_ID_BITPACKED, "bitpacked" },
    { AV_CODEC_ID_MSCC, "mscc" },
    { AV_CODEC_ID_SRGC, "srgc" },
    { AV_CODEC_ID_SVG, "svg" },
    { AV_CODEC_ID_GDV, "gdv" },
    { AV_CODEC_ID_FITS, "fits" },
    { AV_CODEC_ID_IMM4, "imm4" },
    { AV_CODEC_ID_PROSUMER, "prosumer" },
    { AV_CODEC_ID_MWSC, "mwsc" },
    { AV_CODEC_ID_WCMV, "wcmv" },
    { AV_CODEC_ID_RASC, "rasc" },
    { AV_CODEC_ID_HYMT, "hymt" },
    { AV_CODEC_ID_ARBC, "arbc" },
    { AV_CODEC_ID_AGM, "agm" },
    { AV_CODEC_ID_LSCR, "lscr" },
    { AV_CODEC_ID_VP4, "vp4" },
    { AV_CODEC_ID_IMM5, "imm5" },
    { AV_CODEC_ID_MVDV, "mvdv" },
    { AV_CODEC_ID_MVHA, "mvha" },
    { AV_CODEC_ID_CDTOONS, "cdtoons" },
    { AV_CODEC_ID_MV30, "mv30" },
    { AV_CODEC_ID_NOTCHLC, "notchlc" },
    { AV_CODEC_ID_PFM, "pfm" },
    { AV_CODEC_ID_MOBICLIP, "mobiclip" },
    { AV_CODEC_ID_PHOTOCD, "photocd" },
    { AV_CODEC_ID_IPU, "ipu" },
    { AV_CODEC_ID_ARGO, "argo" },
    { AV_CODEC_ID_CRI, "cri" },
    { AV_CODEC_ID_SIMBIOSIS_IMX, "simbiosis_imx" },
    { AV_CODEC_ID_SGA_VIDEO, "sga_video" },
    { AV_CODEC_ID_GEM, "gem" },
    { AV_CODEC_ID_VBN, "vbn" },
    { AV_CODEC_ID_JPEGXL, "jpegxl" },
    { AV_CODEC_ID_QOI, "qoi" },
    { AV_CODEC_ID_PHM, "phm" },
    { AV_CODEC_ID_RADIANCE_HDR, "radiance_hdr" },
    { AV_CODEC_ID_WBMP, "wbmp" },
    { AV_CODEC_ID_MEDIA100, "media100" },
    { AV_CODEC_ID_VQC, "vqc" },
    { AV_CODEC_ID_FIRST_AUDIO, "first_audio" },
    { AV_CODEC_ID_PCM_S16LE, "pcm_s16le" },
    { AV_CODEC_ID_PCM_S16BE, "pcm_s16be" },
    { AV_CODEC_ID_PCM_U16LE, "pcm_u16le" },
    { AV_CODEC_ID_PCM_U16BE, "pcm_u16be" },
    { AV_CODEC_ID_PCM_S8, "pcm_s8" },
    { AV_CODEC_ID_PCM_U8, "pcm_u8" },
    { AV_CODEC_ID_PCM_MULAW, "pcm_mulaw" },
    { AV_CODEC_ID_PCM_ALAW, "pcm_alaw" },
    { AV_CODEC_ID_PCM_S32LE, "pcm_s32le" },
    { AV_CODEC_ID_PCM_S32BE, "pcm_s32be" },
    { AV_CODEC_ID_PCM_U32LE, "pcm_u32le" },
    { AV_CODEC_ID_PCM_U32BE, "pcm_u32be" },
    { AV_CODEC_ID_PCM_S24LE, "pcm_s24le" },
    { AV_CODEC_ID_PCM_S24BE, "pcm_s24be" },
    { AV_CODEC_ID_PCM_U24LE, "pcm_u24le" },
    { AV_CODEC_ID_PCM_U24BE, "pcm_u24be" },
    { AV_CODEC_ID_PCM_S24DAUD, "pcm_s24daud" },
    { AV_CODEC_ID_PCM_ZORK, "pcm_zork" },
    { AV_CODEC_ID_PCM_S16LE_PLANAR, "pcm_s16le_planar" },
    { AV_CODEC_ID_PCM_DVD, "pcm_dvd" },
    { AV_CODEC_ID_PCM_F32BE, "pcm_f32be" },
    { AV_CODEC_ID_PCM_F32LE, "pcm_f32le" },
    { AV_CODEC_ID_PCM_F64BE, "pcm_f64be" },
    { AV_CODEC_ID_PCM_F64LE, "pcm_f64le" },
    { AV_CODEC_ID_PCM_BLURAY, "pcm_bluray" },
    { AV_CODEC_ID_PCM_LXF, "pcm_lxf" },
    { AV_CODEC_ID_S302M, "s302m" },
    { AV_CODEC_ID_PCM_S8_PLANAR, "pcm_s8_planar" },
    { AV_CODEC_ID_PCM_S24LE_PLANAR, "pcm_s24le_planar" },
    { AV_CODEC_ID_PCM_S32LE_PLANAR, "pcm_s32le_planar" },
    { AV_CODEC_ID_PCM_S16BE_PLANAR, "pcm_s16be_planar" },
    { AV_CODEC_ID_PCM_S64LE, "pcm_s64le" },
    { AV_CODEC_ID_PCM_S64BE, "pcm_s64be" },
    { AV_CODEC_ID_PCM_F16LE, "pcm_f16le" },
    { AV_CODEC_ID_PCM_F24LE, "pcm_f24le" },
    { AV_CODEC_ID_PCM_VIDC, "pcm_vidc" },
    { AV_CODEC_ID_PCM_SGA, "pcm_sga" },
    { AV_CODEC_ID_ADPCM_IMA_QT, "adpcm_ima_qt" },
    { AV_CODEC_ID_ADPCM_IMA_WAV, "adpcm_ima_wav" },
    { AV_CODEC_ID_ADPCM_IMA_DK3, "adpcm_ima_dk3" },
    { AV_CODEC_ID_ADPCM_IMA_DK4, "adpcm_ima_dk4" },
    { AV_CODEC_ID_ADPCM_IMA_WS, "adpcm_ima_ws" },
    { AV_CODEC_ID_ADPCM_IMA_SMJPEG, "adpcm_ima_smjpeg" },
    { AV_CODEC_ID_ADPCM_MS, "adpcm_ms" },
    { AV_CODEC_ID_ADPCM_4XM, "adpcm_4xm" },
    { AV_CODEC_ID_ADPCM_XA, "adpcm_xa" },
    { AV_CODEC_ID_ADPCM_ADX, "adpcm_adx" },
    { AV_CODEC_ID_ADPCM_EA, "adpcm_ea" },
    { AV_CODEC_ID_ADPCM_G726, "adpcm_g726" },
    { AV_CODEC_ID_ADPCM_CT, "adpcm_ct" },
    { AV_CODEC_ID_ADPCM_SWF, "adpcm_swf" },
    { AV_CODEC_ID_ADPCM_YAMAHA, "adpcm_yamaha" },
    { AV_CODEC_ID_ADPCM_SBPRO_4, "adpcm_sbpro_4" },
    { AV_CODEC_ID_ADPCM_SBPRO_3, "adpcm_sbpro_3" },
    { AV_CODEC_ID_ADPCM_SBPRO_2, "adpcm_sbpro_2" },
    { AV_CODEC_ID_ADPCM_THP, "adpcm_thp" },
    { AV_CODEC_ID_ADPCM_IMA_AMV, "adpcm_ima_amv" },
    { AV_CODEC_ID_ADPCM_EA_R1, "adpcm_ea_r1" },
    { AV_CODEC_ID_ADPCM_EA_R3, "adpcm_ea_r3" },
    { AV_CODEC_ID_ADPCM_EA_R2, "adpcm_ea_r2" },
    { AV_CODEC_ID_ADPCM_IMA_EA_SEAD, "adpcm_ima_ea_sead" },
    { AV_CODEC_ID_ADPCM_IMA_EA_EACS, "adpcm_ima_ea_eacs" },
    { AV_CODEC_ID_ADPCM_EA_XAS, "adpcm_ea_xas" },
    { AV_CODEC_ID_ADPCM_EA_MAXIS_XA, "adpcm_ea_maxis_xa" },
    { AV_CODEC_ID_ADPCM_IMA_ISS, "adpcm_ima_iss" },
    { AV_CODEC_ID_ADPCM_G722, "adpcm_g722" },
    { AV_CODEC_ID_ADPCM_IMA_APC, "adpcm_ima_apc" },
    { AV_CODEC_ID_ADPCM_VIMA, "adpcm_vima" },
    { AV_CODEC_ID_ADPCM_AFC, "adpcm_afc" },
    { AV_CODEC_ID_ADPCM_IMA_OKI, "adpcm_ima_oki" },
    { AV_CODEC_ID_ADPCM_DTK, "adpcm_dtk" },
    { AV_CODEC_ID_ADPCM_IMA_RAD, "adpcm_ima_rad" },
    { AV_CODEC_ID_ADPCM_G726LE, "adpcm_g726le" },
    { AV_CODEC_ID_ADPCM_THP_LE, "adpcm_thp_le" },
    { AV_CODEC_ID_ADPCM_PSX, "adpcm_psx" },
    { AV_CODEC_ID_ADPCM_AICA, "adpcm_aica" },
    { AV_CODEC_ID_ADPCM_IMA_DAT4, "adpcm_ima_dat4" },
    { AV_CODEC_ID_ADPCM_MTAF, "adpcm_mtaf" },
    { AV_CODEC_ID_ADPCM_AGM, "adpcm_agm" },
    { AV_CODEC_ID_ADPCM_ARGO, "adpcm_argo" },
    { AV_CODEC_ID_ADPCM_IMA_SSI, "adpcm_ima_ssi" },
    { AV_CODEC_ID_ADPCM_ZORK, "adpcm_zork" },
    { AV_CODEC_ID_ADPCM_IMA_APM, "adpcm_ima_apm" },
    { AV_CODEC_ID_ADPCM_IMA_ALP, "adpcm_ima_alp" },
    { AV_CODEC_ID_ADPCM_IMA_MTF, "adpcm_ima_mtf" },
    { AV_CODEC_ID_ADPCM_IMA_CUNNING, "adpcm_ima_cunning" },
    { AV_CODEC_ID_ADPCM_IMA_MOFLEX, "adpcm_ima_moflex" },
    { AV_CODEC_ID_ADPCM_IMA_ACORN, "adpcm_ima_acorn" },
    { AV_CODEC_ID_ADPCM_XMD, "adpcm_xmd" },
    { AV_CODEC_ID_AMR_NB, "amr_nb" },
    { AV_CODEC_ID_AMR_WB, "amr_wb" },
    { AV_CODEC_ID_RA_144, "ra_144" },
    { AV_CODEC_ID_RA_288, "ra_288" },
    { AV_CODEC_ID_ROQ_DPCM, "roq_dpcm" },
    { AV_CODEC_ID_INTERPLAY_DPCM, "interplay_dpcm" },
    { AV_CODEC_ID_XAN_DPCM, "xan_dpcm" },
    { AV_CODEC_ID_SOL_DPCM, "sol_dpcm" },
    { AV_CODEC_ID_SDX2_DPCM, "sdx2_dpcm" },
    { AV_CODEC_ID_GREMLIN_DPCM, "gremlin_dpcm" },
    { AV_CODEC_ID_DERF_DPCM, "derf_dpcm" },
    { AV_CODEC_ID_WADY_DPCM, "wady_dpcm" },
    { AV_CODEC_ID_CBD2_DPCM, "cbd2_dpcm" },
    { AV_CODEC_ID_MP2, "mp2" },
    { AV_CODEC_ID_MP3, "mp3" },
    { AV_CODEC_ID_AAC, "aac" },
    { AV_CODEC_ID_AC3, "ac3" },
    { AV_CODEC_ID_DTS, "dts" },
    { AV_CODEC_ID_VORBIS, "vorbis" },
    { AV_CODEC_ID_DVAUDIO, "dvaudio" },
    { AV_CODEC_ID_WMAV1, "wmav1" },
    { AV_CODEC_ID_WMAV2, "wmav2" },
    { AV_CODEC_ID_MACE3, "mace3" },
    { AV_CODEC_ID_MACE6, "mace6" },
    { AV_CODEC_ID_VMDAUDIO, "vmdaudio" },
    { AV_CODEC_ID_FLAC, "flac" },
    { AV_CODEC_ID_MP3ADU, "mp3adu" },
    { AV_CODEC_ID_MP3ON4, "mp3on4" },
    { AV_CODEC_ID_SHORTEN, "shorten" },
    { AV_CODEC_ID_ALAC, "alac" },
    { AV_CODEC_ID_WESTWOOD_SND1, "westwood_snd1" },
    { AV_CODEC_ID_GSM, "gsm" },
    { AV_CODEC_ID_QDM2, "qdm2" },
    { AV_CODEC_ID_COOK, "cook" },
    { AV_CODEC_ID_TRUESPEECH, "truespeech" },
    { AV_CODEC_ID_TTA, "tta" },
    { AV_CODEC_ID_SMACKAUDIO, "smackaudio" },
    { AV_CODEC_ID_QCELP, "qcelp" },
    { AV_CODEC_ID_WAVPACK, "wavpack" },
    { AV_CODEC_ID_DSICINAUDIO, "dsicinaudio" },
    { AV_CODEC_ID_IMC, "imc" },
    { AV_CODEC_ID_MUSEPACK7, "musepack7" },
    { AV_CODEC_ID_MLP, "mlp" },
    { AV_CODEC_ID_GSM_MS, "gsm_ms" },
    { AV_CODEC_ID_ATRAC3, "atrac3" },
    { AV_CODEC_ID_APE, "ape" },
    { AV_CODEC_ID_NELLYMOSER, "nellymoser" },
    { AV_CODEC_ID_MUSEPACK8, "musepack8" },
    { AV_CODEC_ID_SPEEX, "speex" },
    { AV_CODEC_ID_WMAVOICE, "wmavoice" },
    { AV_CODEC_ID_WMAPRO, "wmapro" },
    { AV_CODEC_ID_WMALOSSLESS, "wmalossless" },
    { AV_CODEC_ID_ATRAC3P, "atrac3p" },
    { AV_CODEC_ID_EAC3, "eac3" },
    { AV_CODEC_ID_SIPR, "sipr" },
    { AV_CODEC_ID_MP1, "mp1" },
    { AV_CODEC_ID_TWINVQ, "twinvq" },
    { AV_CODEC_ID_TRUEHD, "truehd" },
    { AV_CODEC_ID_MP4ALS, "mp4als" },
    { AV_CODEC_ID_ATRAC1, "atrac1" },
    { AV_CODEC_ID_BINKAUDIO_RDFT, "binkaudio_rdft" },
    { AV_CODEC_ID_BINKAUDIO_DCT, "binkaudio_dct" },
    { AV_CODEC_ID_AAC_LATM, "aac_latm" },
    { AV_CODEC_ID_QDMC, "qdmc" },
    { AV_CODEC_ID_CELT, "celt" },
    { AV_CODEC_ID_G723_1, "g723_1" },
    { AV_CODEC_ID_G729, "g729" },
    { AV_CODEC_ID_8SVX_EXP, "8svx_exp" },
    { AV_CODEC_ID_8SVX_FIB, "8svx_fib" },
    { AV_CODEC_ID_BMV_AUDIO, "bmv_audio" },
    { AV_CODEC_ID_RALF, "ralf" },
    { AV_CODEC_ID_IAC, "iac" },
    { AV_CODEC_ID_ILBC, "ilbc" },
    { AV_CODEC_ID_OPUS, "opus" },
    { AV_CODEC_ID_COMFORT_NOISE, "comfort_noise" },
    { AV_CODEC_ID_TAK, "tak" },
    { AV_CODEC_ID_METASOUND, "metasound" },
    { AV_CODEC_ID_PAF_AUDIO, "paf_audio" },
    { AV_CODEC_ID_ON2AVC, "on2avc" },
    { AV_CODEC_ID_DSS_SP, "dss_sp" },
    { AV_CODEC_ID_CODEC2, "codec2" },
    { AV_CODEC_ID_FFWAVESYNTH, "ffwavesynth" },
    { AV_CODEC_ID_SONIC, "sonic" },
    { AV_CODEC_ID_SONIC_LS, "sonic_ls" },
    { AV_CODEC_ID_EVRC, "evrc" },
    { AV_CODEC_ID_SMV, "smv" },
    { AV_CODEC_ID_DSD_LSBF, "dsd_lsbf" },
    { AV_CODEC_ID_DSD_MSBF, "dsd_msbf" },
    { AV_CODEC_ID_DSD_LSBF_PLANAR, "dsd_lsbf_planar" },
    { AV_CODEC_ID_DSD_MSBF_PLANAR, "dsd_msbf_planar" },
    { AV_CODEC_ID_4GV, "4gv" },
    { AV_CODEC_ID_INTERPLAY_ACM, "interplay_acm" },
    { AV_CODEC_ID_XMA1, "xma1" },
    { AV_CODEC_ID_XMA2, "xma2" },
    { AV_CODEC_ID_DST, "dst" },
    { AV_CODEC_ID_ATRAC3AL, "atrac3al" },
    { AV_CODEC_ID_ATRAC3PAL, "atrac3pal" },
    { AV_CODEC_ID_DOLBY_E, "dolby_e" },
    { AV_CODEC_ID_APTX, "aptx" },
    { AV_CODEC_ID_APTX_HD, "aptx_hd" },
    { AV_CODEC_ID_SBC, "sbc" },
    { AV_CODEC_ID_ATRAC9, "atrac9" },
    { AV_CODEC_ID_HCOM, "hcom" },
    { AV_CODEC_ID_ACELP_KELVIN, "acelp_kelvin" },
    { AV_CODEC_ID_MPEGH_3D_AUDIO, "mpegh_3d_audio" },
    { AV_CODEC_ID_SIREN, "siren" },
    { AV_CODEC_ID_HCA, "hca" },
    { AV_CODEC_ID_FASTAUDIO, "fastaudio" },
    { AV_CODEC_ID_MSNSIREN, "msnsiren" },
    { AV_CODEC_ID_DFPWM, "dfpwm" },
    { AV_CODEC_ID_BONK, "bonk" },
    { AV_CODEC_ID_MISC4, "misc4" },
    { AV_CODEC_ID_APAC, "apac" },
    { AV_CODEC_ID_FTR, "ftr" },
    { AV_CODEC_ID_WAVARC, "wavarc" },
    { AV_CODEC_ID_RKA, "rka" },
    { AV_CODEC_ID_FIRST_SUBTITLE, "first_subtitle" },
    { AV_CODEC_ID_DVD_SUBTITLE, "dvd_subtitle" },
    { AV_CODEC_ID_DVB_SUBTITLE, "dvb_subtitle" },
    { AV_CODEC_ID_TEXT, "text" },
    { AV_CODEC_ID_XSUB, "xsub" },
    { AV_CODEC_ID_SSA, "ssa" },
    { AV_CODEC_ID_MOV_TEXT, "mov_text" },
    { AV_CODEC_ID_HDMV_PGS_SUBTITLE, "hdmv_pgs_subtitle" },
    { AV_CODEC_ID_DVB_TELETEXT, "dvb_teletext" },
    { AV_CODEC_ID_SRT, "srt" },
    { AV_CODEC_ID_MICRODVD, "microdvd" },
    { AV_CODEC_ID_EIA_608, "eia_608" },
    { AV_CODEC_ID_JACOSUB, "jacosub" },
    { AV_CODEC_ID_SAMI, "sami" },
    { AV_CODEC_ID_REALTEXT, "realtext" },
    { AV_CODEC_ID_STL, "stl" },
    { AV_CODEC_ID_SUBVIEWER1, "subviewer1" },
    { AV_CODEC_ID_SUBVIEWER, "subviewer" },
    { AV_CODEC_ID_SUBRIP, "subrip" },
    { AV_CODEC_ID_WEBVTT, "webvtt" },
    { AV_CODEC_ID_MPL2, "mpl2" },
    { AV_CODEC_ID_VPLAYER, "vplayer" },
    { AV_CODEC_ID_PJS, "pjs" },
    { AV_CODEC_ID_ASS, "ass" },
    { AV_CODEC_ID_HDMV_TEXT_SUBTITLE, "hdmv_text_subtitle" },
    { AV_CODEC_ID_TTML, "ttml" },
    { AV_CODEC_ID_ARIB_CAPTION, "arib_caption" },
    { AV_CODEC_ID_FIRST_UNKNOWN, "first_unknown" },
    { AV_CODEC_ID_TTF, "ttf" },
    { AV_CODEC_ID_SCTE_35, "scte_35" },
    { AV_CODEC_ID_EPG, "epg" },
    { AV_CODEC_ID_BINTEXT, "bintext" },
    { AV_CODEC_ID_XBIN, "xbin" },
    { AV_CODEC_ID_IDF, "idf" },
    { AV_CODEC_ID_OTF, "otf" },
    { AV_CODEC_ID_SMPTE_KLV, "smpte_klv" },
    { AV_CODEC_ID_DVD_NAV, "dvd_nav" },
    { AV_CODEC_ID_TIMED_ID3, "timed_id3" },
    { AV_CODEC_ID_BIN_DATA, "bin_data" },
    { AV_CODEC_ID_PROBE, "probe" },
    { AV_CODEC_ID_MPEG2TS, "mpeg2ts" },
    { AV_CODEC_ID_MPEG4SYSTEMS, "mpeg4systems" },
    { AV_CODEC_ID_FFMETADATA, "ffmetadata" },
    { AV_CODEC_ID_WRAPPED_AVFRAME, "wrapped_avframe" },
    { AV_CODEC_ID_VNULL, "vnull" },
    { AV_CODEC_ID_ANULL, "anull" }
};

} // namespace anonymous

ffi::RetLocal<v8::Value> CodecParameters::SearchCodecID(const std::string& name)
{
    // The number of results that the fuzzy search algorithm should collect
    constexpr int kFuzzySearchN = 10;

    // Use `std::map` to sort results by their priority automatically
    std::multimap<int32_t, const NamedCodec*> results;

    for (const NamedCodec& codec : g_codec_names)
    {
        std::string_view codec_name(codec.name);
        size_t substr_pos = codec_name.find(name);
        if (substr_pos == std::string::npos)
        {
            // `name` is not a substring of this codec name.
            // We should evaluate its priority by levenshtein distance |d|,
            // which is always positive.
            int d = utils::SolveLevenshteinDistance(name, codec.name);
            results.insert({d, &codec});
            continue;
        }

        // The earlier the substring appears, the higher priority it has.
        results.insert({std::numeric_limits<int32_t>::min() + substr_pos, &codec});
    }

    // Collect and convert results
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> jsctx = isolate->GetCurrentContext();
    v8::Local<v8::Map> map = v8::Map::New(isolate);
    for (const auto& item : results)
    {
        auto key = v8::String::NewFromUtf8(isolate, item.second->name).ToLocalChecked();
        map->Set(jsctx, key, v8::Uint32::New(isolate, item.second->id)).ToLocalChecked();
        if (map->Size() >= kFuzzySearchN)
            break;
    }
    return map;
}

ffi::Ret<int32_t> CodecParameters::GetCodecType(uint32_t id)
{
    return avcodec_get_type(static_cast<AVCodecID>(id));
}

ffi::Ret<int32_t> CodecParameters::GetCodecBitsPerSample(uint32_t id)
{
    return av_get_exact_bits_per_sample(static_cast<AVCodecID>(id));
}

CodecParameters::CodecParameters()
{
    params_ = avcodec_parameters_alloc();
    CHECK(params_ && "allocation failed");
}

CodecParameters::~CodecParameters()
{
    avcodec_parameters_free(&params_);
}

ffi::RetLocal<v8::Value> CodecParameters::clone()
{
    AVCodecParameters *dst = avcodec_parameters_alloc();
    if (int res = avcodec_parameters_copy(dst, params_); res < 0)
    {
        avcodec_parameters_free(&dst);
        return ffi::Fail(ffi::kErr, fmt::format("failed to clone: {}", av_err2str(res)));
    }
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<CodecParameters>(isolate, dst);
}

ffi::RetLocal<v8::Value> CodecParameters::copyExtraData(const ffi::Mem<uint8_t>& data)
{
    using SizeT = decltype(AVCodecParameters::extradata_size);
    // ffmpeg requires an extra padding area, with its bytes zeroed.
    size_t min_size = data.ByteSize() + AV_INPUT_BUFFER_PADDING_SIZE;
    if (min_size > std::numeric_limits<SizeT>::max())
        return ffi::Fail(ffi::kRangeErr, "size of data exceeds the maximum value");

    if (params_->extradata_size < data.ByteSize())
    {
        if (params_->extradata)
            av_free(params_->extradata);
        params_->extradata = nullptr;
        params_->extradata_size = 0;
    }

    if (!params_->extradata)
        params_->extradata = static_cast<uint8_t*>(av_malloc(min_size));

    params_->extradata_size = static_cast<SizeT>(data.ByteSize());
    std::memcpy(params_->extradata, data.Address(), data.ByteSize());
    std::memset(params_->extradata + data.ByteSize(), 0, AV_INPUT_BUFFER_PADDING_SIZE);
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value>
CodecParameters::setChannelLayout(const ffi::Class<AChannelLayout>& layout)
{
    layout->CopyLayoutTo(params_->ch_layout);
    return GetThisHandle(v8::Isolate::GetCurrent());
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
