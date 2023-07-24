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

export type int = number
export type float = number
export type double = number

interface EmClassHandle {
    clone(): EmClassHandle
    delete(): void
    deleteLater(): unknown
    isAliasOf(other: unknown): boolean
    isDeleted(): boolean
}

interface EmVector<T> extends EmClassHandle {
    delete(): void
    get(pos: number): T
    push_back(value: T): void
    resize(n: number, val: T): void
    set(pos: number, value: T): boolean
    size(): number
}

interface EmbindEnum {
    readonly values: number[];
}

interface EmbindEnumEntity<T> {
    readonly value: number;
}

export interface OpenCVLib {

    /* Emscripten exported */

    _malloc(size: number): number
    _free(ptr: number): void

    readonly HEAP8: Int8Array
    readonly HEAP16: Int16Array
    readonly HEAP32: Int32Array
    readonly HEAPU8: Uint8Array
    readonly HEAPU16: Uint16Array
    readonly HEAPU32: Uint32Array
    readonly HEAPF32: Float32Array
    readonly HEAPF64: Float64Array
    
    /* OpenCV constants */
    readonly ACCESS_FAST: number
    readonly ACCESS_MASK: number
    readonly ACCESS_READ: number
    readonly ACCESS_RW: number
    readonly ACCESS_WRITE: number
    readonly ADAPTIVE_THRESH_GAUSSIAN_C: number
    readonly ADAPTIVE_THRESH_MEAN_C: number
    readonly AKAZE_DESCRIPTOR_KAZE: number
    readonly AKAZE_DESCRIPTOR_KAZE_UPRIGHT: number
    readonly AKAZE_DESCRIPTOR_MLDB: number
    readonly AKAZE_DESCRIPTOR_MLDB_UPRIGHT: number
    readonly AgastFeatureDetector_AGAST_5_8: number
    readonly AgastFeatureDetector_AGAST_7_12d: number
    readonly AgastFeatureDetector_AGAST_7_12s: number
    readonly AgastFeatureDetector_NONMAX_SUPPRESSION: number
    readonly AgastFeatureDetector_OAST_9_16: number
    readonly AgastFeatureDetector_THRESHOLD: number
    readonly BORDER_CONSTANT: number
    readonly BORDER_DEFAULT: number
    readonly BORDER_ISOLATED: number
    readonly BORDER_REFLECT: number
    readonly BORDER_REFLECT101: number
    readonly BORDER_REFLECT_101: number
    readonly BORDER_REPLICATE: number
    readonly BORDER_TRANSPARENT: number
    readonly BORDER_WRAP: number
    readonly CALIB_CB_ACCURACY: number
    readonly CALIB_CB_ADAPTIVE_THRESH: number
    readonly CALIB_CB_ASYMMETRIC_GRID: number
    readonly CALIB_CB_CLUSTERING: number
    readonly CALIB_CB_EXHAUSTIVE: number
    readonly CALIB_CB_FAST_CHECK: number
    readonly CALIB_CB_FILTER_QUADS: number
    readonly CALIB_CB_LARGER: number
    readonly CALIB_CB_MARKER: number
    readonly CALIB_CB_NORMALIZE_IMAGE: number
    readonly CALIB_CB_SYMMETRIC_GRID: number
    readonly CALIB_FIX_ASPECT_RATIO: number
    readonly CALIB_FIX_FOCAL_LENGTH: number
    readonly CALIB_FIX_INTRINSIC: number
    readonly CALIB_FIX_K1: number
    readonly CALIB_FIX_K2: number
    readonly CALIB_FIX_K3: number
    readonly CALIB_FIX_K4: number
    readonly CALIB_FIX_K5: number
    readonly CALIB_FIX_K6: number
    readonly CALIB_FIX_PRINCIPAL_POINT: number
    readonly CALIB_FIX_S1_S2_S3_S4: number
    readonly CALIB_FIX_TANGENT_DIST: number
    readonly CALIB_FIX_TAUX_TAUY: number
    readonly CALIB_HAND_EYE_ANDREFF: number
    readonly CALIB_HAND_EYE_DANIILIDIS: number
    readonly CALIB_HAND_EYE_HORAUD: number
    readonly CALIB_HAND_EYE_PARK: number
    readonly CALIB_HAND_EYE_TSAI: number
    readonly CALIB_NINTRINSIC: number
    readonly CALIB_RATIONAL_MODEL: number
    readonly CALIB_ROBOT_WORLD_HAND_EYE_LI: number
    readonly CALIB_ROBOT_WORLD_HAND_EYE_SHAH: number
    readonly CALIB_SAME_FOCAL_LENGTH: number
    readonly CALIB_THIN_PRISM_MODEL: number
    readonly CALIB_TILTED_MODEL: number
    readonly CALIB_USE_EXTRINSIC_GUESS: number
    readonly CALIB_USE_INTRINSIC_GUESS: number
    readonly CALIB_USE_LU: number
    readonly CALIB_USE_QR: number
    readonly CALIB_ZERO_DISPARITY: number
    readonly CALIB_ZERO_TANGENT_DIST: number
    readonly CASCADE_DO_CANNY_PRUNING: number
    readonly CASCADE_DO_ROUGH_SEARCH: number
    readonly CASCADE_FIND_BIGGEST_OBJECT: number
    readonly CASCADE_SCALE_IMAGE: number
    readonly CCL_DEFAULT: number
    readonly CCL_GRANA: number
    readonly CCL_WU: number
    readonly CC_STAT_AREA: number
    readonly CC_STAT_HEIGHT: number
    readonly CC_STAT_LEFT: number
    readonly CC_STAT_MAX: number
    readonly CC_STAT_TOP: number
    readonly CC_STAT_WIDTH: number
    readonly CHAIN_APPROX_NONE: number
    readonly CHAIN_APPROX_SIMPLE: number
    readonly CHAIN_APPROX_TC89_KCOS: number
    readonly CHAIN_APPROX_TC89_L1: number
    readonly CMP_EQ: number
    readonly CMP_GE: number
    readonly CMP_GT: number
    readonly CMP_LE: number
    readonly CMP_LT: number
    readonly CMP_NE: number
    readonly COLORMAP_AUTUMN: number
    readonly COLORMAP_BONE: number
    readonly COLORMAP_CIVIDIS: number
    readonly COLORMAP_COOL: number
    readonly COLORMAP_DEEPGREEN: number
    readonly COLORMAP_HOT: number
    readonly COLORMAP_HSV: number
    readonly COLORMAP_INFERNO: number
    readonly COLORMAP_JET: number
    readonly COLORMAP_MAGMA: number
    readonly COLORMAP_OCEAN: number
    readonly COLORMAP_PARULA: number
    readonly COLORMAP_PINK: number
    readonly COLORMAP_PLASMA: number
    readonly COLORMAP_RAINBOW: number
    readonly COLORMAP_SPRING: number
    readonly COLORMAP_SUMMER: number
    readonly COLORMAP_TURBO: number
    readonly COLORMAP_TWILIGHT: number
    readonly COLORMAP_TWILIGHT_SHIFTED: number
    readonly COLORMAP_VIRIDIS: number
    readonly COLORMAP_WINTER: number
    readonly COLOR_BGR2BGR555: number
    readonly COLOR_BGR2BGR565: number
    readonly COLOR_BGR2BGRA: number
    readonly COLOR_BGR2GRAY: number
    readonly COLOR_BGR2HLS: number
    readonly COLOR_BGR2HLS_FULL: number
    readonly COLOR_BGR2HSV: number
    readonly COLOR_BGR2HSV_FULL: number
    readonly COLOR_BGR2Lab: number
    readonly COLOR_BGR2Luv: number
    readonly COLOR_BGR2RGB: number
    readonly COLOR_BGR2RGBA: number
    readonly COLOR_BGR2XYZ: number
    readonly COLOR_BGR2YCrCb: number
    readonly COLOR_BGR2YUV: number
    readonly COLOR_BGR2YUV_I420: number
    readonly COLOR_BGR2YUV_IYUV: number
    readonly COLOR_BGR2YUV_YV12: number
    readonly COLOR_BGR5552BGR: number
    readonly COLOR_BGR5552BGRA: number
    readonly COLOR_BGR5552GRAY: number
    readonly COLOR_BGR5552RGB: number
    readonly COLOR_BGR5552RGBA: number
    readonly COLOR_BGR5652BGR: number
    readonly COLOR_BGR5652BGRA: number
    readonly COLOR_BGR5652GRAY: number
    readonly COLOR_BGR5652RGB: number
    readonly COLOR_BGR5652RGBA: number
    readonly COLOR_BGRA2BGR: number
    readonly COLOR_BGRA2BGR555: number
    readonly COLOR_BGRA2BGR565: number
    readonly COLOR_BGRA2GRAY: number
    readonly COLOR_BGRA2RGB: number
    readonly COLOR_BGRA2RGBA: number
    readonly COLOR_BGRA2YUV_I420: number
    readonly COLOR_BGRA2YUV_IYUV: number
    readonly COLOR_BGRA2YUV_YV12: number
    readonly COLOR_BayerBG2BGR: number
    readonly COLOR_BayerBG2BGRA: number
    readonly COLOR_BayerBG2BGR_EA: number
    readonly COLOR_BayerBG2BGR_VNG: number
    readonly COLOR_BayerBG2GRAY: number
    readonly COLOR_BayerBG2RGB: number
    readonly COLOR_BayerBG2RGBA: number
    readonly COLOR_BayerBG2RGB_EA: number
    readonly COLOR_BayerBG2RGB_VNG: number
    readonly COLOR_BayerGB2BGR: number
    readonly COLOR_BayerGB2BGRA: number
    readonly COLOR_BayerGB2BGR_EA: number
    readonly COLOR_BayerGB2BGR_VNG: number
    readonly COLOR_BayerGB2GRAY: number
    readonly COLOR_BayerGB2RGB: number
    readonly COLOR_BayerGB2RGBA: number
    readonly COLOR_BayerGB2RGB_EA: number
    readonly COLOR_BayerGB2RGB_VNG: number
    readonly COLOR_BayerGR2BGR: number
    readonly COLOR_BayerGR2BGRA: number
    readonly COLOR_BayerGR2BGR_EA: number
    readonly COLOR_BayerGR2BGR_VNG: number
    readonly COLOR_BayerGR2GRAY: number
    readonly COLOR_BayerGR2RGB: number
    readonly COLOR_BayerGR2RGBA: number
    readonly COLOR_BayerGR2RGB_EA: number
    readonly COLOR_BayerGR2RGB_VNG: number
    readonly COLOR_BayerRG2BGR: number
    readonly COLOR_BayerRG2BGRA: number
    readonly COLOR_BayerRG2BGR_EA: number
    readonly COLOR_BayerRG2BGR_VNG: number
    readonly COLOR_BayerRG2GRAY: number
    readonly COLOR_BayerRG2RGB: number
    readonly COLOR_BayerRG2RGBA: number
    readonly COLOR_BayerRG2RGB_EA: number
    readonly COLOR_BayerRG2RGB_VNG: number
    readonly COLOR_COLORCVT_MAX: number
    readonly COLOR_GRAY2BGR: number
    readonly COLOR_GRAY2BGR555: number
    readonly COLOR_GRAY2BGR565: number
    readonly COLOR_GRAY2BGRA: number
    readonly COLOR_GRAY2RGB: number
    readonly COLOR_GRAY2RGBA: number
    readonly COLOR_HLS2BGR: number
    readonly COLOR_HLS2BGR_FULL: number
    readonly COLOR_HLS2RGB: number
    readonly COLOR_HLS2RGB_FULL: number
    readonly COLOR_HSV2BGR: number
    readonly COLOR_HSV2BGR_FULL: number
    readonly COLOR_HSV2RGB: number
    readonly COLOR_HSV2RGB_FULL: number
    readonly COLOR_LBGR2Lab: number
    readonly COLOR_LBGR2Luv: number
    readonly COLOR_LRGB2Lab: number
    readonly COLOR_LRGB2Luv: number
    readonly COLOR_Lab2BGR: number
    readonly COLOR_Lab2LBGR: number
    readonly COLOR_Lab2LRGB: number
    readonly COLOR_Lab2RGB: number
    readonly COLOR_Luv2BGR: number
    readonly COLOR_Luv2LBGR: number
    readonly COLOR_Luv2LRGB: number
    readonly COLOR_Luv2RGB: number
    readonly COLOR_RGB2BGR: number
    readonly COLOR_RGB2BGR555: number
    readonly COLOR_RGB2BGR565: number
    readonly COLOR_RGB2BGRA: number
    readonly COLOR_RGB2GRAY: number
    readonly COLOR_RGB2HLS: number
    readonly COLOR_RGB2HLS_FULL: number
    readonly COLOR_RGB2HSV: number
    readonly COLOR_RGB2HSV_FULL: number
    readonly COLOR_RGB2Lab: number
    readonly COLOR_RGB2Luv: number
    readonly COLOR_RGB2RGBA: number
    readonly COLOR_RGB2XYZ: number
    readonly COLOR_RGB2YCrCb: number
    readonly COLOR_RGB2YUV: number
    readonly COLOR_RGB2YUV_I420: number
    readonly COLOR_RGB2YUV_IYUV: number
    readonly COLOR_RGB2YUV_YV12: number
    readonly COLOR_RGBA2BGR: number
    readonly COLOR_RGBA2BGR555: number
    readonly COLOR_RGBA2BGR565: number
    readonly COLOR_RGBA2BGRA: number
    readonly COLOR_RGBA2GRAY: number
    readonly COLOR_RGBA2RGB: number
    readonly COLOR_RGBA2YUV_I420: number
    readonly COLOR_RGBA2YUV_IYUV: number
    readonly COLOR_RGBA2YUV_YV12: number
    readonly COLOR_RGBA2mRGBA: number
    readonly COLOR_XYZ2BGR: number
    readonly COLOR_XYZ2RGB: number
    readonly COLOR_YCrCb2BGR: number
    readonly COLOR_YCrCb2RGB: number
    readonly COLOR_YUV2BGR: number
    readonly COLOR_YUV2BGRA_I420: number
    readonly COLOR_YUV2BGRA_IYUV: number
    readonly COLOR_YUV2BGRA_NV12: number
    readonly COLOR_YUV2BGRA_NV21: number
    readonly COLOR_YUV2BGRA_UYNV: number
    readonly COLOR_YUV2BGRA_UYVY: number
    readonly COLOR_YUV2BGRA_Y422: number
    readonly COLOR_YUV2BGRA_YUNV: number
    readonly COLOR_YUV2BGRA_YUY2: number
    readonly COLOR_YUV2BGRA_YUYV: number
    readonly COLOR_YUV2BGRA_YV12: number
    readonly COLOR_YUV2BGRA_YVYU: number
    readonly COLOR_YUV2BGR_I420: number
    readonly COLOR_YUV2BGR_IYUV: number
    readonly COLOR_YUV2BGR_NV12: number
    readonly COLOR_YUV2BGR_NV21: number
    readonly COLOR_YUV2BGR_UYNV: number
    readonly COLOR_YUV2BGR_UYVY: number
    readonly COLOR_YUV2BGR_Y422: number
    readonly COLOR_YUV2BGR_YUNV: number
    readonly COLOR_YUV2BGR_YUY2: number
    readonly COLOR_YUV2BGR_YUYV: number
    readonly COLOR_YUV2BGR_YV12: number
    readonly COLOR_YUV2BGR_YVYU: number
    readonly COLOR_YUV2GRAY_420: number
    readonly COLOR_YUV2GRAY_I420: number
    readonly COLOR_YUV2GRAY_IYUV: number
    readonly COLOR_YUV2GRAY_NV12: number
    readonly COLOR_YUV2GRAY_NV21: number
    readonly COLOR_YUV2GRAY_UYNV: number
    readonly COLOR_YUV2GRAY_UYVY: number
    readonly COLOR_YUV2GRAY_Y422: number
    readonly COLOR_YUV2GRAY_YUNV: number
    readonly COLOR_YUV2GRAY_YUY2: number
    readonly COLOR_YUV2GRAY_YUYV: number
    readonly COLOR_YUV2GRAY_YV12: number
    readonly COLOR_YUV2GRAY_YVYU: number
    readonly COLOR_YUV2RGB: number
    readonly COLOR_YUV2RGBA_I420: number
    readonly COLOR_YUV2RGBA_IYUV: number
    readonly COLOR_YUV2RGBA_NV12: number
    readonly COLOR_YUV2RGBA_NV21: number
    readonly COLOR_YUV2RGBA_UYNV: number
    readonly COLOR_YUV2RGBA_UYVY: number
    readonly COLOR_YUV2RGBA_Y422: number
    readonly COLOR_YUV2RGBA_YUNV: number
    readonly COLOR_YUV2RGBA_YUY2: number
    readonly COLOR_YUV2RGBA_YUYV: number
    readonly COLOR_YUV2RGBA_YV12: number
    readonly COLOR_YUV2RGBA_YVYU: number
    readonly COLOR_YUV2RGB_I420: number
    readonly COLOR_YUV2RGB_IYUV: number
    readonly COLOR_YUV2RGB_NV12: number
    readonly COLOR_YUV2RGB_NV21: number
    readonly COLOR_YUV2RGB_UYNV: number
    readonly COLOR_YUV2RGB_UYVY: number
    readonly COLOR_YUV2RGB_Y422: number
    readonly COLOR_YUV2RGB_YUNV: number
    readonly COLOR_YUV2RGB_YUY2: number
    readonly COLOR_YUV2RGB_YUYV: number
    readonly COLOR_YUV2RGB_YV12: number
    readonly COLOR_YUV2RGB_YVYU: number
    readonly COLOR_YUV420p2BGR: number
    readonly COLOR_YUV420p2BGRA: number
    readonly COLOR_YUV420p2GRAY: number
    readonly COLOR_YUV420p2RGB: number
    readonly COLOR_YUV420p2RGBA: number
    readonly COLOR_YUV420sp2BGR: number
    readonly COLOR_YUV420sp2BGRA: number
    readonly COLOR_YUV420sp2GRAY: number
    readonly COLOR_YUV420sp2RGB: number
    readonly COLOR_YUV420sp2RGBA: number
    readonly COLOR_mRGBA2RGBA: number
    readonly CONTOURS_MATCH_I1: number
    readonly CONTOURS_MATCH_I2: number
    readonly CONTOURS_MATCH_I3: number
    readonly COVAR_COLS: number
    readonly COVAR_NORMAL: number
    readonly COVAR_ROWS: number
    readonly COVAR_SCALE: number
    readonly COVAR_SCRAMBLED: number
    readonly COVAR_USE_AVG: number
    readonly CirclesGridFinderParameters_ASYMMETRIC_GRID: number
    readonly CirclesGridFinderParameters_SYMMETRIC_GRID: number
    readonly DCT_INVERSE: number
    readonly DCT_ROWS: number
    readonly DECOMP_CHOLESKY: number
    readonly DECOMP_EIG: number
    readonly DECOMP_LU: number
    readonly DECOMP_NORMAL: number
    readonly DECOMP_QR: number
    readonly DECOMP_SVD: number
    readonly DFT_COMPLEX_INPUT: number
    readonly DFT_COMPLEX_OUTPUT: number
    readonly DFT_INVERSE: number
    readonly DFT_REAL_OUTPUT: number
    readonly DFT_ROWS: number
    readonly DFT_SCALE: number
    readonly DISOpticalFlow_PRESET_FAST: number
    readonly DISOpticalFlow_PRESET_MEDIUM: number
    readonly DISOpticalFlow_PRESET_ULTRAFAST: number
    readonly DIST_C: number
    readonly DIST_FAIR: number
    readonly DIST_HUBER: number
    readonly DIST_L1: number
    readonly DIST_L12: number
    readonly DIST_L2: number
    readonly DIST_LABEL_CCOMP: number
    readonly DIST_LABEL_PIXEL: number
    readonly DIST_MASK_3: number
    readonly DIST_MASK_5: number
    readonly DIST_MASK_PRECISE: number
    readonly DIST_USER: number
    readonly DIST_WELSCH: number
    readonly DescriptorMatcher_BRUTEFORCE: number
    readonly DescriptorMatcher_BRUTEFORCE_HAMMING: number
    readonly DescriptorMatcher_BRUTEFORCE_HAMMINGLUT: number
    readonly DescriptorMatcher_BRUTEFORCE_L1: number
    readonly DescriptorMatcher_BRUTEFORCE_SL2: number
    readonly DescriptorMatcher_FLANNBASED: number
    readonly DrawMatchesFlags_DEFAULT: number
    readonly DrawMatchesFlags_DRAW_OVER_OUTIMG: number
    readonly DrawMatchesFlags_DRAW_RICH_KEYPOINTS: number
    readonly DrawMatchesFlags_NOT_DRAW_SINGLE_POINTS: number
    readonly FILLED: number
    readonly FILTER_SCHARR: number
    readonly FLOODFILL_FIXED_RANGE: number
    readonly FLOODFILL_MASK_ONLY: number
    readonly FM_7POINT: number
    readonly FM_8POINT: number
    readonly FM_LMEDS: number
    readonly FM_RANSAC: number
    readonly FONT_HERSHEY_COMPLEX: number
    readonly FONT_HERSHEY_COMPLEX_SMALL: number
    readonly FONT_HERSHEY_DUPLEX: number
    readonly FONT_HERSHEY_PLAIN: number
    readonly FONT_HERSHEY_SCRIPT_COMPLEX: number
    readonly FONT_HERSHEY_SCRIPT_SIMPLEX: number
    readonly FONT_HERSHEY_SIMPLEX: number
    readonly FONT_HERSHEY_TRIPLEX: number
    readonly FONT_ITALIC: number
    readonly FastFeatureDetector_FAST_N: number
    readonly FastFeatureDetector_NONMAX_SUPPRESSION: number
    readonly FastFeatureDetector_THRESHOLD: number
    readonly FastFeatureDetector_TYPE_5_8: number
    readonly FastFeatureDetector_TYPE_7_12: number
    readonly FastFeatureDetector_TYPE_9_16: number
    readonly FileNode_EMPTY: number
    readonly FileNode_FLOAT: number
    readonly FileNode_FLOW: number
    readonly FileNode_INT: number
    readonly FileNode_MAP: number
    readonly FileNode_NAMED: number
    readonly FileNode_NONE: number
    readonly FileNode_REAL: number
    readonly FileNode_SEQ: number
    readonly FileNode_STR: number
    readonly FileNode_STRING: number
    readonly FileNode_TYPE_MASK: number
    readonly FileNode_UNIFORM: number
    readonly FileStorage_APPEND: number
    readonly FileStorage_BASE64: number
    readonly FileStorage_FORMAT_AUTO: number
    readonly FileStorage_FORMAT_JSON: number
    readonly FileStorage_FORMAT_MASK: number
    readonly FileStorage_FORMAT_XML: number
    readonly FileStorage_FORMAT_YAML: number
    readonly FileStorage_INSIDE_MAP: number
    readonly FileStorage_MEMORY: number
    readonly FileStorage_NAME_EXPECTED: number
    readonly FileStorage_READ: number
    readonly FileStorage_UNDEFINED: number
    readonly FileStorage_VALUE_EXPECTED: number
    readonly FileStorage_WRITE: number
    readonly FileStorage_WRITE_BASE64: number
    readonly Formatter_FMT_C: number
    readonly Formatter_FMT_CSV: number
    readonly Formatter_FMT_DEFAULT: number
    readonly Formatter_FMT_MATLAB: number
    readonly Formatter_FMT_NUMPY: number
    readonly Formatter_FMT_PYTHON: number
    readonly GC_BGD: number
    readonly GC_EVAL: number
    readonly GC_EVAL_FREEZE_MODEL: number
    readonly GC_FGD: number
    readonly GC_INIT_WITH_MASK: number
    readonly GC_INIT_WITH_RECT: number
    readonly GC_PR_BGD: number
    readonly GC_PR_FGD: number
    readonly GEMM_1_T: number
    readonly GEMM_2_T: number
    readonly GEMM_3_T: number
    readonly HISTCMP_BHATTACHARYYA: number
    readonly HISTCMP_CHISQR: number
    readonly HISTCMP_CHISQR_ALT: number
    readonly HISTCMP_CORREL: number
    readonly HISTCMP_HELLINGER: number
    readonly HISTCMP_INTERSECT: number
    readonly HISTCMP_KL_DIV: number
    readonly HOGDescriptor_DEFAULT_NLEVELS: number
    readonly HOGDescriptor_DESCR_FORMAT_COL_BY_COL: number
    readonly HOGDescriptor_DESCR_FORMAT_ROW_BY_ROW: number
    readonly HOGDescriptor_L2Hys: number
    readonly HOUGH_GRADIENT: number
    readonly HOUGH_GRADIENT_ALT: number
    readonly HOUGH_MULTI_SCALE: number
    readonly HOUGH_PROBABILISTIC: number
    readonly HOUGH_STANDARD: number
    readonly INPAINT_NS: number
    readonly INPAINT_TELEA: number
    readonly INTERSECT_FULL: number
    readonly INTERSECT_NONE: number
    readonly INTERSECT_PARTIAL: number
    readonly INTER_AREA: number
    readonly INTER_BITS: number
    readonly INTER_BITS2: number
    readonly INTER_CUBIC: number
    readonly INTER_LANCZOS4: number
    readonly INTER_LINEAR: number
    readonly INTER_LINEAR_EXACT: number
    readonly INTER_MAX: number
    readonly INTER_NEAREST: number
    readonly INTER_NEAREST_EXACT: number
    readonly INTER_TAB_SIZE: number
    readonly INTER_TAB_SIZE2: number
    readonly KAZE_DIFF_CHARBONNIER: number
    readonly KAZE_DIFF_PM_G1: number
    readonly KAZE_DIFF_PM_G2: number
    readonly KAZE_DIFF_WEICKERT: number
    readonly KMEANS_PP_CENTERS: number
    readonly KMEANS_RANDOM_CENTERS: number
    readonly KMEANS_USE_INITIAL_LABELS: number
    readonly LDR_SIZE: number
    readonly LINE_4: number
    readonly LINE_8: number
    readonly LINE_AA: number
    readonly LMEDS: number
    readonly LOCAL_OPTIM_GC: number
    readonly LOCAL_OPTIM_INNER_AND_ITER_LO: number
    readonly LOCAL_OPTIM_INNER_LO: number
    readonly LOCAL_OPTIM_NULL: number
    readonly LOCAL_OPTIM_SIGMA: number
    readonly LSD_REFINE_ADV: number
    readonly LSD_REFINE_NONE: number
    readonly LSD_REFINE_STD: number
    readonly MARKER_CROSS: number
    readonly MARKER_DIAMOND: number
    readonly MARKER_SQUARE: number
    readonly MARKER_STAR: number
    readonly MARKER_TILTED_CROSS: number
    readonly MARKER_TRIANGLE_DOWN: number
    readonly MARKER_TRIANGLE_UP: number
    readonly MIXED_CLONE: number
    readonly MONOCHROME_TRANSFER: number
    readonly MORPH_BLACKHAT: number
    readonly MORPH_CLOSE: number
    readonly MORPH_CROSS: number
    readonly MORPH_DILATE: number
    readonly MORPH_ELLIPSE: number
    readonly MORPH_ERODE: number
    readonly MORPH_GRADIENT: number
    readonly MORPH_HITMISS: number
    readonly MORPH_OPEN: number
    readonly MORPH_RECT: number
    readonly MORPH_TOPHAT: number
    readonly MOTION_AFFINE: number
    readonly MOTION_EUCLIDEAN: number
    readonly MOTION_HOMOGRAPHY: number
    readonly MOTION_TRANSLATION: number
    readonly Mat_AUTO_STEP: number
    readonly Mat_CONTINUOUS_FLAG: number
    readonly Mat_DEPTH_MASK: number
    readonly Mat_MAGIC_MASK: number
    readonly Mat_MAGIC_VAL: number
    readonly Mat_SUBMATRIX_FLAG: number
    readonly Mat_TYPE_MASK: number
    readonly NEIGH_FLANN_KNN: number
    readonly NEIGH_FLANN_RADIUS: number
    readonly NEIGH_GRID: number
    readonly NORMAL_CLONE: number
    readonly NORMCONV_FILTER: number
    readonly NORM_HAMMING: number
    readonly NORM_HAMMING2: number
    readonly NORM_INF: number
    readonly NORM_L1: number
    readonly NORM_L2: number
    readonly NORM_L2SQR: number
    readonly NORM_MINMAX: number
    readonly NORM_RELATIVE: number
    readonly NORM_TYPE_MASK: number
    readonly OPTFLOW_FARNEBACK_GAUSSIAN: number
    readonly OPTFLOW_LK_GET_MIN_EIGENVALS: number
    readonly OPTFLOW_USE_INITIAL_FLOW: number
    readonly ORB_FAST_SCORE: number
    readonly ORB_HARRIS_SCORE: number
    readonly PCA_DATA_AS_COL: number
    readonly PCA_DATA_AS_ROW: number
    readonly PCA_USE_AVG: number
    readonly PROJ_SPHERICAL_EQRECT: number
    readonly PROJ_SPHERICAL_ORTHO: number
    readonly Param_ALGORITHM: number
    readonly Param_BOOLEAN: number
    readonly Param_FLOAT: number
    readonly Param_INT: number
    readonly Param_MAT: number
    readonly Param_MAT_VECTOR: number
    readonly Param_REAL: number
    readonly Param_SCALAR: number
    readonly Param_STRING: number
    readonly Param_UCHAR: number
    readonly Param_UINT64: number
    readonly Param_UNSIGNED_INT: number
    readonly QUAT_ASSUME_NOT_UNIT: number
    readonly QUAT_ASSUME_UNIT: number
    readonly QuatEnum_EULER_ANGLES_MAX_VALUE: number
    readonly QuatEnum_EXT_XYX: number
    readonly QuatEnum_EXT_XYZ: number
    readonly QuatEnum_EXT_XZX: number
    readonly QuatEnum_EXT_XZY: number
    readonly QuatEnum_EXT_YXY: number
    readonly QuatEnum_EXT_YXZ: number
    readonly QuatEnum_EXT_YZX: number
    readonly QuatEnum_EXT_YZY: number
    readonly QuatEnum_EXT_ZXY: number
    readonly QuatEnum_EXT_ZXZ: number
    readonly QuatEnum_EXT_ZYX: number
    readonly QuatEnum_EXT_ZYZ: number
    readonly QuatEnum_INT_XYX: number
    readonly QuatEnum_INT_XYZ: number
    readonly QuatEnum_INT_XZX: number
    readonly QuatEnum_INT_XZY: number
    readonly QuatEnum_INT_YXY: number
    readonly QuatEnum_INT_YXZ: number
    readonly QuatEnum_INT_YZX: number
    readonly QuatEnum_INT_YZY: number
    readonly QuatEnum_INT_ZXY: number
    readonly QuatEnum_INT_ZXZ: number
    readonly QuatEnum_INT_ZYX: number
    readonly QuatEnum_INT_ZYZ: number
    readonly RANSAC: number
    readonly RECURS_FILTER: number
    readonly REDUCE_AVG: number
    readonly REDUCE_MAX: number
    readonly REDUCE_MIN: number
    readonly REDUCE_SUM: number
    readonly RETR_CCOMP: number
    readonly RETR_EXTERNAL: number
    readonly RETR_FLOODFILL: number
    readonly RETR_LIST: number
    readonly RETR_TREE: number
    readonly RHO: number
    readonly RNG_NORMAL: number
    readonly RNG_UNIFORM: number
    readonly ROTATE_180: number
    readonly ROTATE_90_CLOCKWISE: number
    readonly ROTATE_90_COUNTERCLOCKWISE: number
    readonly SAMPLING_NAPSAC: number
    readonly SAMPLING_PROGRESSIVE_NAPSAC: number
    readonly SAMPLING_PROSAC: number
    readonly SAMPLING_UNIFORM: number
    readonly SCORE_METHOD_LMEDS: number
    readonly SCORE_METHOD_MAGSAC: number
    readonly SCORE_METHOD_MSAC: number
    readonly SCORE_METHOD_RANSAC: number
    readonly SOLVELP_MULTI: number
    readonly SOLVELP_SINGLE: number
    readonly SOLVELP_UNBOUNDED: number
    readonly SOLVELP_UNFEASIBLE: number
    readonly SOLVEPNP_AP3P: number
    readonly SOLVEPNP_DLS: number
    readonly SOLVEPNP_EPNP: number
    readonly SOLVEPNP_IPPE: number
    readonly SOLVEPNP_IPPE_SQUARE: number
    readonly SOLVEPNP_ITERATIVE: number
    readonly SOLVEPNP_MAX_COUNT: number
    readonly SOLVEPNP_P3P: number
    readonly SOLVEPNP_SQPNP: number
    readonly SOLVEPNP_UPNP: number
    readonly SORT_ASCENDING: number
    readonly SORT_DESCENDING: number
    readonly SORT_EVERY_COLUMN: number
    readonly SORT_EVERY_ROW: number
    readonly SVD_FULL_UV: number
    readonly SVD_MODIFY_A: number
    readonly SVD_NO_UV: number
    readonly SparseMat_HASH_BIT: number
    readonly SparseMat_HASH_SCALE: number
    readonly SparseMat_MAGIC_VAL: number
    readonly SparseMat_MAX_DIM: number
    readonly StereoBM_PREFILTER_NORMALIZED_RESPONSE: number
    readonly StereoBM_PREFILTER_XSOBEL: number
    readonly StereoMatcher_DISP_SCALE: number
    readonly StereoMatcher_DISP_SHIFT: number
    readonly StereoSGBM_MODE_HH: number
    readonly StereoSGBM_MODE_HH4: number
    readonly StereoSGBM_MODE_SGBM: number
    readonly StereoSGBM_MODE_SGBM_3WAY: number
    readonly Subdiv2D_NEXT_AROUND_DST: number
    readonly Subdiv2D_NEXT_AROUND_LEFT: number
    readonly Subdiv2D_NEXT_AROUND_ORG: number
    readonly Subdiv2D_NEXT_AROUND_RIGHT: number
    readonly Subdiv2D_PREV_AROUND_DST: number
    readonly Subdiv2D_PREV_AROUND_LEFT: number
    readonly Subdiv2D_PREV_AROUND_ORG: number
    readonly Subdiv2D_PREV_AROUND_RIGHT: number
    readonly Subdiv2D_PTLOC_ERROR: number
    readonly Subdiv2D_PTLOC_INSIDE: number
    readonly Subdiv2D_PTLOC_ON_EDGE: number
    readonly Subdiv2D_PTLOC_OUTSIDE_RECT: number
    readonly Subdiv2D_PTLOC_VERTEX: number
    readonly THRESH_BINARY: number
    readonly THRESH_BINARY_INV: number
    readonly THRESH_MASK: number
    readonly THRESH_OTSU: number
    readonly THRESH_TOZERO: number
    readonly THRESH_TOZERO_INV: number
    readonly THRESH_TRIANGLE: number
    readonly THRESH_TRUNC: number
    readonly TM_CCOEFF: number
    readonly TM_CCOEFF_NORMED: number
    readonly TM_CCORR: number
    readonly TM_CCORR_NORMED: number
    readonly TM_SQDIFF: number
    readonly TM_SQDIFF_NORMED: number
    readonly TermCriteria_COUNT: number
    readonly TermCriteria_EPS: number
    readonly TermCriteria_MAX_ITER: number
    readonly UMatData_ASYNC_CLEANUP: number
    readonly UMatData_COPY_ON_MAP: number
    readonly UMatData_DEVICE_COPY_OBSOLETE: number
    readonly UMatData_DEVICE_MEM_MAPPED: number
    readonly UMatData_HOST_COPY_OBSOLETE: number
    readonly UMatData_TEMP_COPIED_UMAT: number
    readonly UMatData_TEMP_UMAT: number
    readonly UMatData_USER_ALLOCATED: number
    readonly UMat_AUTO_STEP: number
    readonly UMat_CONTINUOUS_FLAG: number
    readonly UMat_DEPTH_MASK: number
    readonly UMat_MAGIC_MASK: number
    readonly UMat_MAGIC_VAL: number
    readonly UMat_SUBMATRIX_FLAG: number
    readonly UMat_TYPE_MASK: number
    readonly USAC_ACCURATE: number
    readonly USAC_DEFAULT: number
    readonly USAC_FAST: number
    readonly USAC_FM_8PTS: number
    readonly USAC_MAGSAC: number
    readonly USAC_PARALLEL: number
    readonly USAC_PROSAC: number
    readonly USAGE_ALLOCATE_DEVICE_MEMORY: number
    readonly USAGE_ALLOCATE_HOST_MEMORY: number
    readonly USAGE_ALLOCATE_SHARED_MEMORY: number
    readonly USAGE_DEFAULT: number
    readonly WARP_FILL_OUTLIERS: number
    readonly WARP_INVERSE_MAP: number
    readonly WARP_POLAR_LINEAR: number
    readonly WARP_POLAR_LOG: number
    readonly _InputArray_CUDA_GPU_MAT: number
    readonly _InputArray_CUDA_HOST_MEM: number
    readonly _InputArray_EXPR: number
    readonly _InputArray_FIXED_SIZE: number
    readonly _InputArray_FIXED_TYPE: number
    readonly _InputArray_KIND_MASK: number
    readonly _InputArray_KIND_SHIFT: number
    readonly _InputArray_MAT: number
    readonly _InputArray_MATX: number
    readonly _InputArray_NONE: number
    readonly _InputArray_OPENGL_BUFFER: number
    readonly _InputArray_STD_ARRAY: number
    readonly _InputArray_STD_ARRAY_MAT: number
    readonly _InputArray_STD_BOOL_VECTOR: number
    readonly _InputArray_STD_VECTOR: number
    readonly _InputArray_STD_VECTOR_CUDA_GPU_MAT: number
    readonly _InputArray_STD_VECTOR_MAT: number
    readonly _InputArray_STD_VECTOR_UMAT: number
    readonly _InputArray_STD_VECTOR_VECTOR: number
    readonly _InputArray_UMAT: number
    readonly _OutputArray_DEPTH_MASK_16F: number
    readonly _OutputArray_DEPTH_MASK_16S: number
    readonly _OutputArray_DEPTH_MASK_16U: number
    readonly _OutputArray_DEPTH_MASK_32F: number
    readonly _OutputArray_DEPTH_MASK_32S: number
    readonly _OutputArray_DEPTH_MASK_64F: number
    readonly _OutputArray_DEPTH_MASK_8S: number
    readonly _OutputArray_DEPTH_MASK_8U: number
    readonly _OutputArray_DEPTH_MASK_ALL: number
    readonly _OutputArray_DEPTH_MASK_ALL_16F: number
    readonly _OutputArray_DEPTH_MASK_ALL_BUT_8S: number
    readonly _OutputArray_DEPTH_MASK_FLT: number
    readonly __UMAT_USAGE_FLAGS_32BIT: number
    readonly BadAlign: number
    readonly BadAlphaChannel: number
    readonly BadCOI: number
    readonly BadCallBack: number
    readonly BadDataPtr: number
    readonly BadDepth: number
    readonly BadImageSize: number
    readonly BadModelOrChSeq: number
    readonly BadNumChannel1U: number
    readonly BadNumChannels: number
    readonly BadOffset: number
    readonly BadOrder: number
    readonly BadOrigin: number
    readonly BadROISize: number
    readonly BadStep: number
    readonly BadTileSize: number
    readonly GpuApiCallError: number
    readonly GpuNotSupported: number
    readonly HeaderIsNull: number
    readonly MaskIsTiled: number
    readonly OpenCLApiCallError: number
    readonly OpenCLDoubleNotSupported: number
    readonly OpenCLInitError: number
    readonly OpenCLNoAMDBlasFft: number
    readonly OpenGlApiCallError: number
    readonly OpenGlNotSupported: number
    readonly StsAssert: number
    readonly StsAutoTrace: number
    readonly StsBackTrace: number
    readonly StsBadArg: number
    readonly StsBadFlag: number
    readonly StsBadFunc: number
    readonly StsBadMask: number
    readonly StsBadMemBlock: number
    readonly StsBadPoint: number
    readonly StsBadSize: number
    readonly StsDivByZero: number
    readonly StsError: number
    readonly StsFilterOffsetErr: number
    readonly StsFilterStructContentErr: number
    readonly StsInplaceNotSupported: number
    readonly StsInternal: number
    readonly StsKernelStructContentErr: number
    readonly StsNoConv: number
    readonly StsNoMem: number
    readonly StsNotImplemented: number
    readonly StsNullPtr: number
    readonly StsObjectNotFound: number
    readonly StsOk: number
    readonly StsOutOfRange: number
    readonly StsParseError: number
    readonly StsUnmatchedFormats: number
    readonly StsUnmatchedSizes: number
    readonly StsUnsupportedFormat: number
    readonly StsVecLengthErr: number
    readonly CvFeatureParams_HAAR: number
    readonly CvFeatureParams_HOG: number
    readonly CvFeatureParams_LBP: number
    readonly TEST_CUSTOM: number
    readonly TEST_EQ: number
    readonly TEST_GE: number
    readonly TEST_GT: number
    readonly TEST_LE: number
    readonly TEST_LT: number
    readonly TEST_NE: number
    readonly TrackerSamplerCSC_MODE_DETECT: number
    readonly TrackerSamplerCSC_MODE_INIT_NEG: number
    readonly TrackerSamplerCSC_MODE_INIT_POS: number
    readonly TrackerSamplerCSC_MODE_TRACK_NEG: number
    readonly TrackerSamplerCSC_MODE_TRACK_POS: number
    readonly DNN_BACKEND_CUDA: number
    readonly DNN_BACKEND_DEFAULT: number
    readonly DNN_BACKEND_HALIDE: number
    readonly DNN_BACKEND_INFERENCE_ENGINE: number
    readonly DNN_BACKEND_OPENCV: number
    readonly DNN_BACKEND_VKCOM: number
    readonly DNN_TARGET_CPU: number
    readonly DNN_TARGET_CUDA: number
    readonly DNN_TARGET_CUDA_FP16: number
    readonly DNN_TARGET_FPGA: number
    readonly DNN_TARGET_HDDL: number
    readonly DNN_TARGET_MYRIAD: number
    readonly DNN_TARGET_OPENCL: number
    readonly DNN_TARGET_OPENCL_FP16: number
    readonly DNN_TARGET_VULKAN: number
    readonly CALIB_CHECK_COND: number
    readonly CALIB_FIX_SKEW: number
    readonly CALIB_RECOMPUTE_EXTRINSIC: number
    readonly CV_8UC1: number
    readonly CV_8UC2: number
    readonly CV_8UC3: number
    readonly CV_8UC4: number
    readonly CV_8SC1: number
    readonly CV_8SC2: number
    readonly CV_8SC3: number
    readonly CV_8SC4: number
    readonly CV_16UC1: number
    readonly CV_16UC2: number
    readonly CV_16UC3: number
    readonly CV_16UC4: number
    readonly CV_16SC1: number
    readonly CV_16SC2: number
    readonly CV_16SC3: number
    readonly CV_16SC4: number
    readonly CV_32SC1: number
    readonly CV_32SC2: number
    readonly CV_32SC3: number
    readonly CV_32SC4: number
    readonly CV_32FC1: number
    readonly CV_32FC2: number
    readonly CV_32FC3: number
    readonly CV_32FC4: number
    readonly CV_64FC1: number
    readonly CV_64FC2: number
    readonly CV_64FC3: number
    readonly CV_64FC4: number
    readonly CV_8U: number
    readonly CV_8S: number
    readonly CV_16U: number
    readonly CV_16S: number
    readonly CV_32S: number
    readonly CV_32F: number
    readonly CV_64F: number
    readonly INT_MIN: number
    readonly INT_MAX: number

    Range: new (start?: int, end?: int) => RangeLike
    TermCriteria: new (type?: int, maxCount?: int, epsilon?: double) => TermCriteriaLike
    Size: new (width?: number, height?: number) => SizeLike
    Point: new (x?: number, y?: number) => PointLike
    Rect: new (x?: number, y?: number, width?: number, height?: number) => RectLike
    RotatedRect: new (center?: Point2fLike, size?: Size2fLike, angle?: float) => RotatedRectLike
    Scalar: (v0?: double, v1?: double, v2?: double, v3?: double) => Scalar
    MinMaxLoc: (minVal?: double, maxVal?: double, minLoc?: PointLike, maxLoc?: PointLike) => MinMaxLocLike
    Circle: new (center?: Point2fLike, radius?: float) => CircleLike

    rotatedRectPoints(r: RotatedRectLike): [Point2fLike, Point2fLike, Point2fLike, Point2fLike]
    rotatedRectBoundingRect(r: RotatedRectLike): RectLike
    rotatedRectBoundingRect2f(r: RotatedRectLike): Rect2fLike
    exceptionFromPtr(ptr: number): ExceptionLike

    Mat: {
        new(): Mat
        new(mat: Mat): Mat
        new(size: SizeLike, type: int): Mat
        new(rows: int, cols: int, type: int): Mat
        new(rows: int, cols: int, type: int, scalar: ScalarLike): Mat
        new(rows: int, cols: int, type: int, data: unknown, step: number): Mat
        
        eye(size: SizeLike, type: int): Mat
        eye(rows: int, cols: int, type: int): Mat
        ones(size: SizeLike, type: int): Mat
        ones(rows: int, cols: int, type: int): Mat
        zeros(size: SizeLike, type: int): Mat
        zeros(rows: int, cols: int, type: int): Mat
    };
    matFromArray<T>(rows: int, cols: int, type: int, array: ArrayLike<T>): Mat
    matFromImageData(imageData: ImageData): Mat

    /* Vectors */

    IntVector: new() => IntVector;
    FloatVector: new() => FloatVector;
    DoubleVector: new() => DoubleVector;
    PointVector: new() => PointVector;
    MatVector: new() => MatVector;
    RectVector: new() => RectVector;
    KeyPointVector: new() => KeyPointVector;
    DMatchVector: new() => DMatchVector;
    DMatchVectorVector: new() => DMatchVectorVector;

    /* Functions */

    minEnclosingCircle(mat: Mat): CircleLike
    floodFill(image: Mat, mask: Mat, seedPoint: PointLike, newVal: ScalarLike, rect: RectLike, loDiff?: ScalarLike, upDiff?: ScalarLike, flags?: int): int
    minMaxLoc(src: Mat, mask?: Mat): MinMaxLocLike
    CV_MAT_DEPTH(flags: int): int
    getBuildInformation(): string

    Canny(image: Mat,
          edges: Mat,
          threshold1: double,
          threshold2: double,
          apertureSize?: int,
          L2gradient?: boolean): void

    Canny(dx: Mat,
          dy: Mat,
          edges: Mat,
          threshold1: double,
          threshold2: double,
          L2gradient?: boolean): void

    GaussianBlur(src: Mat,
                 dst: Mat,
                 ksize: SizeLike,
                 sigmaX: double,
                 sigmaY?: double,
                 borderType?: int): void

    HoughCircles(image: Mat,
                 circles: Mat,
                 method: int,
                 dp: double,
                 minDist: double,
                 param1?: double,
                 param2?: double,
                 minRadius?: int,
                 maxRadius?: int): void

    HoughLines(image: Mat,
               lines: Mat,
               rho: double,
               theta: double,
               threshold: int,
               srn?: double,
               stn?: double,
               min_theta?: double,
               max_theta?: double): void

    HoughLinesP(image: Mat,
                lines: Mat,
                rho: double,
                theta: double,
                threshold: int,
                minLineLength?: double,
                maxLineGap?: double): void

    Laplacian(src: Mat,
              dst: Mat,
              ddepth: int,
              ksize?: int,
              scale?: double,
              delta?: double,
              borderType?: int): void

    Rodrigues(src: Mat,
              dst: Mat,
              jacobian?: Mat): void

    Scharr(src: Mat,
           dst: Mat,
           ddepth: int,
           dx: int,
           dy: int,
           scale?: double,
           delta?: double,
           borderType?: int): void

    Sobel(src: Mat,
          dst: Mat,
          ddepth: int,
          dx: int,
          dy: int,
          ksize?: int,
          scale?: double,
          delta?: double,
          borderType?: int): void

    absdiff(src1: Mat,
            src2: Mat,
            dst: Mat): void

    adaptiveThreshold(src: Mat,
                      dst: Mat,
                      maxValue: double,
                      adaptiveMethod: int,
                      thresholdType: int,
                      blockSize: int,
                      C: double): void

    add(src1: Mat,
        src2: Mat,
        dst: Mat,
        mask?: Mat,
        dtype?: int): void

    addWeighted(src1: Mat,
                alpha: double,
                src2: Mat,
                beta: double,
                gamma: double,
                dst: Mat,
                dtype?: int): void

    approxPolyDP(curve: Mat,
                 approxCurve: Mat,
                 epsilon: double,
                 closed: boolean): void

    arcLength(curve: Mat,
              closed: boolean): double

    bilateralFilter(src: Mat,
                    dst: Mat,
                    d: int,
                    sigmaColor: double,
                    sigmaSpace: double,
                    borderType?: int): void

    bitwise_and(src1: Mat,
                src2: Mat,
                dst: Mat,
                mask?: Mat): void

    bitwise_not(src: Mat,
                dst: Mat,
                mask?: Mat): void

    bitwise_or(src1: Mat,
               src2: Mat,
               dst: Mat,
               mask?: Mat): void

    bitwise_xor(src1: Mat,
                src2: Mat,
                dst: Mat,
                mask?: Mat): void

    blur(src: Mat,
         dst: Mat,
         ksize: SizeLike,
         anchor?: PointLike,
         borderType?: int): void

    boundingRect(array: Mat): RectLike

    boxFilter(src: Mat,
              dst: Mat,
              ddepth: int,
              ksize: SizeLike,
              anchor?: PointLike,
              normalize?: boolean,
              borderType?: int): void

    calcBackProject(images: MatVector,
                    channels: IntVector|int[],
                    hist: Mat,
                    dst: Mat,
                    ranges: FloatVector|float[],
                    scale: double): void

    calcHist(images: MatVector,
             channels: IntVector|int[],
             mask: Mat,
             hist: Mat,
             histSize: IntVector|int[],
             ranges: FloatVector|float[],
             accumulate?: boolean): void

    calcOpticalFlowFarneback(prev: Mat,
                             next: Mat,
                             flow: Mat,
                             pyr_scale: double,
                             levels: int,
                             winsize: int,
                             iterations: int,
                             poly_n: int,
                             poly_sigma: double,
                             flags: int): void

    calcOpticalFlowPyrLK(prevImg: Mat,
                         nextImg: Mat,
                         prevPts: Mat,
                         nextPts: Mat,
                         status: Mat,
                         err: Mat,
                         winSize?: SizeLike,
                         maxLevel?: int,
                         criteria?: TermCriteriaLike,
                         flags?: int,
                         minEigThreshold?: double): void

    calibrateCameraExtended(objectPoints: MatVector,
                            imagePoints: MatVector,
                            imageSize: SizeLike,
                            cameraMatrix: Mat,
                            distCoeffs: Mat,
                            rvecs: MatVector,
                            tvecs: MatVector,
                            stdDeviationsIntrinsics: Mat,
                            stdDeviationsExtrinsics: Mat,
                            perViewErrors: Mat,
                            flags?: int,
                            criteria?: TermCriteriaLike): double

    cartToPolar(x: Mat,
                y: Mat,
                magnitude: Mat,
                angle: Mat,
                angleInDegrees?: boolean): void

    circle(img: Mat,
           center: PointLike,
           radius: int,
           color: ScalarLike,
           thickness?: int,
           lineType?: int,
           shift?: int): void

    compare(src1: Mat,
            src2: Mat,
            dst: Mat,
            cmpop: int): void

    compareHist(H1: Mat,
                H2: Mat,
                method: int): double

    connectedComponents(image: Mat,
                        labels: Mat,
                        connectivity?: int,
                        ltype?: int): int

    connectedComponentsWithStats(image: Mat,
                                 labels: Mat,
                                 stats: Mat,
                                 centroids: Mat,
                                 connectivity?: int,
                                 ltype?: int): int

    contourArea(contour: Mat,
                oriented?: boolean): double

    convertScaleAbs(src: Mat,
                    dst: Mat,
                    alpha?: double,
                    beta?: double): void

    convexHull(points: Mat,
               hull: Mat,
               clockwise?: boolean,
               returnPoints?: boolean): void

    convexityDefects(contour: Mat,
                     convexhull: Mat,
                     convexityDefects: Mat): void

    copyMakeBorder(src: Mat,
                   dst: Mat,
                   top: int,
                   bottom: int,
                   left: int,
                   right: int,
                   borderType: int,
                   value?: ScalarLike): void

    cornerHarris(src: Mat,
                 dst: Mat,
                 blockSize: int,
                 ksize: int,
                 k: double,
                 borderType?: int): void

    cornerMinEigenVal(src: Mat,
                      dst: Mat,
                      blockSize: int,
                      ksize?: int,
                      borderType?: int): void

    countNonZero(src: Mat): int

    cvtColor(src: Mat,
             dst: Mat,
             code: int,
             dstCn?: int): void

    demosaicing(src: Mat,
                dst: Mat,
                code: int,
                dstCn?: int): void

    determinant(mtx: Mat): double

    dft(src: Mat,
        dst: Mat,
        flags?: int,
        nonzeroRows?: int): void

    dilate(src: Mat,
           dst: Mat,
           kernel: Mat,
           anchor?: PointLike,
           iterations?: int,
           borderType?: int,
           borderValue?: ScalarLike): void

    distanceTransform(src: Mat,
                      dst: Mat,
                      distanceType: int,
                      maskSize: int,
                      dstType?: int): void

    distanceTransformWithLabels(src: Mat,
                                dst: Mat,
                                labels: Mat,
                                distanceType: int,
                                maskSize: int,
                                labelType?: int): void

    divide(src1: Mat,
           src2: Mat,
           dst: Mat,
           scale?: double,
           dtype?: int): void

    divide(scale: double,
           src2: Mat,
           dst: Mat,
           dtype?: int): void

    drawContours(image: Mat,
                 contours: MatVector,
                 contourIdx: int,
                 color: ScalarLike,
                 thickness?: int,
                 lineType?: int,
                 hierarchy?: Mat,
                 maxLevel?: int,
                 offset?: PointLike): void

    drawFrameAxes(image: Mat,
                  cameraMatrix: Mat,
                  distCoeffs: Mat,
                  rvec: Mat,
                  tvec: Mat,
                  length: float,
                  thickness?: int): void

    drawKeypoints(image: Mat,
                  keypoints: KeyPointVector,
                  outImage: Mat,
                  color?: ScalarLike,
                  flags?: DrawMatchesFlags): void

    drawMatches(img1: Mat,
                keypoints1: KeyPointVector,
                img2: Mat,
                keypoints2: KeyPointVector,
                matches1to2: DMatchVector,
                outImg: Mat,
                matchColor?: ScalarLike,
                singlePointColor?: ScalarLike,
                matchesMask?: unknown,
                flags?: DrawMatchesFlags): void

    drawMatchesKnn(img1: Mat,
                   keypoints1: KeyPointVector,
                   img2: Mat,
                   keypoints2: KeyPointVector,
                   matches1to2: DMatchVectorVector,
                   outImg: Mat,
                   matchColor?: ScalarLike,
                   singlePointColor?: ScalarLike,
                   matchesMask?: unknown,
                   flags?: DrawMatchesFlags): void

    eigen(src: Mat,
          eigenvalues: Mat,
          eigenvectors?: Mat): boolean

    ellipse(img: Mat,
            center: PointLike,
            axes: SizeLike,
            angle: double,
            startAngle: double,
            endAngle: double,
            color: ScalarLike,
            thickness?: int,
            lineType?: int,
            shift?: int): void

    ellipse(img: Mat,
            box: RotatedRectLike,
            color: ScalarLike,
            thickness?: int,
            lineType?: int): void

    ellipse2Poly(center: PointLike,
                 axes: SizeLike,
                 angle: int,
                 arcStart: int,
                 arcEnd: int,
                 delta: int,
                 pts: PointVector): void

    equalizeHist(src: Mat,
                 dst: Mat): void

    erode(src: Mat,
          dst: Mat,
          kernel: Mat,
          anchor?: PointLike,
          iterations?: int,
          borderType?: int,
          borderValue?: ScalarLike): void

    estimateAffine2D(from: Mat,
                     to: Mat,
                     inliers?: Mat,
                     method?: int,
                     ransacReprojThreshold?: double,
                     maxIters?: number,
                     confidence?: double,
                     refineIters?: number): Mat

    estimateAffine2D(pts1: Mat,
                     pts2: Mat,
                     inliers: Mat,
                     params: unknown): Mat

    exp(src: Mat,
        dst: Mat): void

    fillConvexPoly(img: Mat,
                   points: Mat,
                   color: ScalarLike,
                   lineType?: int,
                   shift?: int): void

    fillPoly(img: Mat,
             pts: MatVector,
             color: ScalarLike,
             lineType?: int,
             shift?: int,
             offset?: PointLike): void

    filter2D(src: Mat,
             dst: Mat,
             ddepth: int,
             kernel: Mat,
             anchor?: PointLike,
             delta?: double,
             borderType?: int): void

    findContours(image: Mat,
                 contours: MatVector,
                 hierarchy: Mat,
                 mode: int,
                 method: int,
                 offset?: PointLike): void

    findHomography(srcPoints: Mat,
                   dstPoints: Mat,
                   method?: int,
                   ransacReprojThreshold?: double,
                   mask?: Mat,
                   maxIters?: int,
                   confidence?: double): Mat

    findHomography(srcPoints: Mat,
                   dstPoints: Mat,
                   mask: Mat,
                   params: unknown): Mat

    findTransformECC(templateImage: Mat,
                     inputImage: Mat,
                     warpMatrix: Mat,
                     motionType: int,
                     criteria: TermCriteriaLike,
                     inputMask: Mat,
                     gaussFiltSize: int): double

    fitEllipse(points: Mat): RotatedRectLike

    fitLine(points: Mat,
            line: Mat,
            distType: int,
            param: double,
            reps: double,
            aeps: double): void

    flip(src: Mat,
         dst: Mat,
         flipCode: int): void

    gemm(src1: Mat,
         src2: Mat,
         alpha: double,
         src3: Mat,
         beta: double,
         dst: Mat,
         flags?: int): void

    getAffineTransform(src: Mat,
                       dst: Mat): Mat

    getDefaultNewCameraMatrix(cameraMatrix: Mat,
                              imgsize?: SizeLike,
                              centerPrincipalPoint?: boolean): Mat

    getOptimalDFTSize(vecsize: int): int

    getPerspectiveTransform(src: Mat,
                            dst: Mat,
                            solveMethod?: int): Mat

    getRotationMatrix2D(center: Point2fLike,
                        angle: double,
                        scale: double): Mat

    getStructuringElement(shape: int,
                          ksize: SizeLike,
                          anchor?: PointLike): Mat

    goodFeaturesToTrack(image: Mat,
                        corners: Mat,
                        maxCorners: int,
                        qualityLevel: double,
                        minDistance: double,
                        mask?: Mat,
                        blockSize?: int,
                        useHarrisDetector?: boolean,
                        k?: double): void

    goodFeaturesToTrack(image: Mat,
                        corners: Mat,
                        maxCorners: int,
                        qualityLevel: double,
                        minDistance: double,
                        mask: Mat,
                        blockSize: int,
                        gradientSize: int,
                        useHarrisDetector?: boolean,
                        k?: double): void

    grabCut(img: Mat,
            mask: Mat,
            rect: RectLike,
            bgdModel: Mat,
            fgdModel: Mat,
            iterCount: int,
            mode?: int): void

    groupRectangles(rectList: RectVector,
                    weights: IntVector|int[],
                    groupThreshold: int,
                    eps?: double): void

    hconcat(src: MatVector,
            dst: Mat): void

    inRange(src: Mat,
            lowerb: Mat | ScalarLike | number[],
            upperb: Mat | ScalarLike | number[],
            dst: Mat): void

    initUndistortRectifyMap(cameraMatrix: Mat,
                            distCoeffs: Mat,
                            R: Mat,
                            newCameraMatrix: Mat,
                            size: SizeLike,
                            m1type: int,
                            map1: Mat,
                            map2: Mat): void

    inpaint(src: Mat,
            inpaintMask: Mat,
            dst: Mat,
            inpaintRadius: double,
            flags: int): void

    integral(src: Mat,
             sum: Mat,
             sdepth?: int): void

    integral2(src: Mat,
              sum: Mat,
              sqsum: Mat,
              sdepth?: int,
              sqdepth?: int): void

    invert(src: Mat,
           dst: Mat,
           flags?: int): double

    isContourConvex(contour: Mat): boolean

    kmeans(data: Mat,
           K: int,
           bestLabels: Mat,
           criteria: TermCriteriaLike,
           attempts: int,
           flags: int,
           centers?: Mat): double

    line(img: Mat,
         pt1: PointLike,
         pt2: PointLike,
         color: ScalarLike,
         thickness?: int,
         lineType?: int,
         shift?: int): void

    log(src: Mat,
        dst: Mat): void

    magnitude(x: Mat,
              y: Mat,
              magnitude: Mat): void

    matchShapes(contour1: Mat,
                contour2: Mat,
                method: int,
                parameter: double): double

    matchTemplate(image: Mat,
                  templ: Mat,
                  result: Mat,
                  method: int,
                  mask?: Mat): void

    max(src1: Mat,
        src2: Mat,
        dst: Mat): void

    mean(src: Mat,
         mask?: Mat): ScalarLike

    meanStdDev(src: Mat,
               mean: Mat,
               stddev: Mat,
               mask?: Mat): void

    medianBlur(src: Mat,
               dst: Mat,
               ksize: int): void

    merge(mv: MatVector,
          dst: Mat): void

    min(src1: Mat,
        src2: Mat,
        dst: Mat): void

    minAreaRect(points: Mat): RotatedRectLike

    mixChannels(src: MatVector,
                dst: MatVector,
                fromTo: IntVector|int[]): void

    moments(array: Mat,
            binaryImage?: boolean): MomentsLike

    morphologyEx(src: Mat,
                 dst: Mat,
                 op: int,
                 kernel: Mat,
                 anchor?: PointLike,
                 iterations?: int,
                 borderType?: int,
                 borderValue?: ScalarLike): void

    multiply(src1: Mat,
             src2: Mat,
             dst: Mat,
             scale?: double,
             dtype?: int): void

    norm(src1: Mat,
         normType?: int,
         mask?: Mat): double

    norm(src1: Mat,
         src2: Mat,
         normType?: int,
         mask?: Mat): double

    normalize(src: Mat,
              dst: Mat,
              alpha?: double,
              beta?: double,
              norm_type?: int,
              dtype?: int,
              mask?: Mat): void

    perspectiveTransform(src: Mat,
                         dst: Mat,
                         m: Mat): void

    pointPolygonTest(contour: Mat,
                     pt: Point2fLike,
                     measureDist: boolean): double

    polarToCart(magnitude: Mat,
                angle: Mat,
                x: Mat,
                y: Mat,
                angleInDegrees?: boolean): void

    polylines(img: Mat,
              pts: MatVector,
              isClosed: boolean,
              color: ScalarLike,
              thickness?: int,
              lineType?: int,
              shift?: int): void

    pow(src: Mat,
        power: double,
        dst: Mat): void

    putText(img: Mat,
            text: string,
            org: PointLike,
            fontFace: int,
            fontScale: double,
            color: ScalarLike,
            thickness?: int,
            lineType?: int,
            bottomLeftOrigin?: boolean): void

    pyrDown(src: Mat,
            dst: Mat,
            dstsize?: SizeLike,
            borderType?: int): void

    pyrUp(src: Mat,
          dst: Mat,
          dstsize?: SizeLike,
          borderType?: int): void

    randn(dst: Mat,
          mean: Mat,
          stddev: Mat): void

    randu(dst: Mat,
          low: Mat,
          high: Mat): void

    rectangle(img: Mat,
              pt1: PointLike,
              pt2: PointLike,
              color: ScalarLike,
              thickness?: int,
              lineType?: int,
              shift?: int): void

    rectangle1(img: Mat,
               rec: RectLike,
               color: ScalarLike,
               thickness?: int,
               lineType?: int,
               shift?: int): void

    reduce(src: Mat,
           dst: Mat,
           dim: int,
           rtype: int,
           dtype?: int): void

    remap(src: Mat,
          dst: Mat,
          map1: Mat,
          map2: Mat,
          interpolation: int,
          borderMode?: int,
          borderValue?: ScalarLike): void

    repeat(src: Mat,
           ny: int,
           nx: int,
           dst: Mat): void

    resize(src: Mat,
           dst: Mat,
           dsize: SizeLike,
           fx?: double,
           fy?: double,
           interpolation?: int): void

    rotate(src: Mat,
           dst: Mat,
           rotateCode: int): void

    sepFilter2D(src: Mat,
                dst: Mat,
                ddepth: int,
                kernelX: Mat,
                kernelY: Mat,
                anchor?: PointLike,
                delta?: double,
                borderType?: int): void

    setIdentity(mtx: Mat,
                s?: ScalarLike): void

    setRNGSeed(seed: int): void

    solve(src1: Mat,
          src2: Mat,
          dst: Mat,
          flags?: int): boolean

    solvePnP(objectPoints: Mat,
             imagePoints: Mat,
             cameraMatrix: Mat,
             distCoeffs: Mat,
             rvec: Mat,
             tvec: Mat,
             useExtrinsicGuess?: boolean,
             flags?: int): boolean

    solvePnPRansac(objectPoints: Mat,
                   imagePoints: Mat,
                   cameraMatrix: Mat,
                   distCoeffs: Mat,
                   rvec: Mat,
                   tvec: Mat,
                   useExtrinsicGuess?: boolean,
                   iterationsCount?: int,
                   reprojectionError?: float,
                   confidence?: double,
                   inliers?: Mat,
                   flags?: int): boolean

    solvePnPRansac(objectPoints: Mat,
                   imagePoints: Mat,
                   cameraMatrix: Mat,
                   distCoeffs: Mat,
                   rvec: Mat,
                   tvec: Mat,
                   inliers: Mat,
                   params?: unknown): boolean

    solvePnPRefineLM(objectPoints: Mat,
                     imagePoints: Mat,
                     cameraMatrix: Mat,
                     distCoeffs: Mat,
                     rvec: Mat,
                     tvec: Mat,
                     criteria?: TermCriteriaLike): void

    solvePoly(coeffs: Mat,
              roots: Mat,
              maxIters?: int): double

    split(m: Mat,
          mv: MatVector): void

    sqrt(src: Mat,
         dst: Mat): void

    subtract(src1: Mat,
             src2: Mat,
             dst: Mat,
             mask?: Mat,
             dtype?: int): void

    threshold(src: Mat,
              dst: Mat,
              thresh: double,
              maxval: double,
              type: int): double

    trace(mtx: Mat): ScalarLike

    transform(src: Mat,
              dst: Mat,
              m: Mat): void

    transpose(src: Mat,
              dst: Mat): void

    undistort(src: Mat,
              dst: Mat,
              cameraMatrix: Mat,
              distCoeffs: Mat,
              newCameraMatrix?: Mat): void

    vconcat(src: MatVector,
            dst: Mat): void

    warpAffine(src: Mat,
               dst: Mat,
               M: Mat,
               dsize: SizeLike,
               flags?: int,
               borderMode?: int,
               borderValue?: ScalarLike): void

    warpPerspective(src: Mat,
                    dst: Mat,
                    M: Mat,
                    dsize: SizeLike,
                    flags?: int,
                    borderMode?: int,
                    borderValue?: ScalarLike): void

    warpPolar(src: Mat,
              dst: Mat,
              dsize: SizeLike,
              center: Point2fLike,
              maxRadius: double,
              flags: int): void

    watershed(image: Mat,
              markers: Mat): void

    blobFromImage(image: Mat,
                  scalefactor?: double,
                  size?: SizeLike,
                  mean?: ScalarLike,
                  swapRB?: boolean,
                  crop?: boolean,
                  ddepth?: int): Mat

    readNet(model: string,
            config?: string,
            framework?: string): dnn_Net

    readNet(framework: string,
            bufferModel: unknown,
            bufferConfig?: unknown): dnn_Net

    readNetFromCaffe(prototxt: string,
                     caffeModel?: string): dnn_Net

    readNetFromCaffe(bufferProto: unknown,
                     bufferModel?: unknown): dnn_Net

    readNetFromDarknet(cfgFile: string,
                       darknetModel?: string): dnn_Net

    readNetFromDarknet(bufferCfg: unknown,
                       bufferModel?: unknown): dnn_Net

    readNetFromONNX(onnxFile: string): dnn_Net

    readNetFromONNX(buffer: unknown): dnn_Net

    readNetFromTensorflow(model: string,
                          config?: string): dnn_Net

    readNetFromTensorflow(bufferModel: unknown,
                          bufferConfig?: unknown): dnn_Net

    readNetFromTorch(model: string,
                     isBinary?: boolean,
                     evaluate?: boolean): dnn_Net

    initUndistortRectifyMap(K: Mat,
                            D: Mat,
                            R: Mat,
                            P: Mat,
                            size: SizeLike,
                            m1type: int,
                            map1: Mat,
                            map2: Mat): void

    /* Enumerations */

    AKAZE_DescriptorType: AKAZE_DescriptorTypeEnumValues
    AccessFlag: AccessFlagEnumValues
    AdaptiveThresholdTypes: AdaptiveThresholdTypesEnumValues
    AgastFeatureDetector_DetectorType: AgastFeatureDetector_DetectorTypeEnumValues
    BorderTypes: BorderTypesEnumValues
    CirclesGridFinderParameters_GridType: CirclesGridFinderParameters_GridTypeEnumValues
    CmpTypes: CmpTypesEnumValues
    ColorConversionCodes: ColorConversionCodesEnumValues
    ColormapTypes: ColormapTypesEnumValues
    ConnectedComponentsAlgorithmsTypes: ConnectedComponentsAlgorithmsTypesEnumValues
    ConnectedComponentsTypes: ConnectedComponentsTypesEnumValues
    ContourApproximationModes: ContourApproximationModesEnumValues
    CovarFlags: CovarFlagsEnumValues
    DecompTypes: DecompTypesEnumValues
    DescriptorMatcher_MatcherType: DescriptorMatcher_MatcherTypeEnumValues
    DftFlags: DftFlagsEnumValues
    DistanceTransformLabelTypes: DistanceTransformLabelTypesEnumValues
    DistanceTransformMasks: DistanceTransformMasksEnumValues
    DistanceTypes: DistanceTypesEnumValues
    DrawMatchesFlags: DrawMatchesFlagsEnumValues
    FastFeatureDetector_DetectorType: FastFeatureDetector_DetectorTypeEnumValues
    FileStorage_Mode: FileStorage_ModeEnumValues
    FileStorage_State: FileStorage_StateEnumValues
    FloodFillFlags: FloodFillFlagsEnumValues
    Formatter_FormatType: Formatter_FormatTypeEnumValues
    GemmFlags: GemmFlagsEnumValues
    GrabCutClasses: GrabCutClassesEnumValues
    GrabCutModes: GrabCutModesEnumValues
    HOGDescriptor_DescriptorStorageFormat: HOGDescriptor_DescriptorStorageFormatEnumValues
    HOGDescriptor_HistogramNormType: HOGDescriptor_HistogramNormTypeEnumValues
    HandEyeCalibrationMethod: HandEyeCalibrationMethodEnumValues
    HersheyFonts: HersheyFontsEnumValues
    HistCompMethods: HistCompMethodsEnumValues
    HoughModes: HoughModesEnumValues
    InterpolationFlags: InterpolationFlagsEnumValues
    InterpolationMasks: InterpolationMasksEnumValues
    KAZE_DiffusivityType: KAZE_DiffusivityTypeEnumValues
    KmeansFlags: KmeansFlagsEnumValues
    LineSegmentDetectorModes: LineSegmentDetectorModesEnumValues
    LineTypes: LineTypesEnumValues
    LocalOptimMethod: LocalOptimMethodEnumValues
    MarkerTypes: MarkerTypesEnumValues
    MorphShapes: MorphShapesEnumValues
    MorphTypes: MorphTypesEnumValues
    NeighborSearchMethod: NeighborSearchMethodEnumValues
    NormTypes: NormTypesEnumValues
    ORB_ScoreType: ORB_ScoreTypeEnumValues
    PCA_Flags: PCA_FlagsEnumValues
    Param: ParamEnumValues
    QuatAssumeType: QuatAssumeTypeEnumValues
    QuatEnum_EulerAnglesType: QuatEnum_EulerAnglesTypeEnumValues
    RectanglesIntersectTypes: RectanglesIntersectTypesEnumValues
    ReduceTypes: ReduceTypesEnumValues
    RetrievalModes: RetrievalModesEnumValues
    RobotWorldHandEyeCalibrationMethod: RobotWorldHandEyeCalibrationMethodEnumValues
    RotateFlags: RotateFlagsEnumValues
    SVD_Flags: SVD_FlagsEnumValues
    SamplingMethod: SamplingMethodEnumValues
    ScoreMethod: ScoreMethodEnumValues
    ShapeMatchModes: ShapeMatchModesEnumValues
    SolveLPResult: SolveLPResultEnumValues
    SolvePnPMethod: SolvePnPMethodEnumValues
    SortFlags: SortFlagsEnumValues
    SpecialFilter: SpecialFilterEnumValues
    TemplateMatchModes: TemplateMatchModesEnumValues
    TermCriteria_Type: TermCriteria_TypeEnumValues
    ThresholdTypes: ThresholdTypesEnumValues
    UMatData_MemoryFlag: UMatData_MemoryFlagEnumValues
    UMatUsageFlags: UMatUsageFlagsEnumValues
    UndistortTypes: UndistortTypesEnumValues
    WarpPolarMode: WarpPolarModeEnumValues
    _InputArray_KindFlag: _InputArray_KindFlagEnumValues
    Error_Code: Error_CodeEnumValues
    detail_CvFeatureParams_FeatureType: detail_CvFeatureParams_FeatureTypeEnumValues
    detail_TestOp: detail_TestOpEnumValues
    detail_TrackerSamplerCSC_MODE: detail_TrackerSamplerCSC_MODEEnumValues
    dnn_Backend: dnn_BackendEnumValues
    dnn_Target: dnn_TargetEnumValues

    /* Classes and constructors */

    AKAZE: {
        new(descriptor_type?: AKAZE_DescriptorType, descriptor_size?: int,
            descriptor_channels?: int, threshold?: float, nOctaves?: int,
            nOctaveLayers?: int, diffusivity?: KAZE_DiffusivityType): AKAZE
    }

    AgastFeatureDetector: {
        new(threshold?: int, nonmaxSuppression?: boolean,
            type?: AgastFeatureDetector_DetectorType): AgastFeatureDetector
    }

    Algorithm: {
    }

    AlignMTB: {
        new(max_bits?: int, exclude_range?: int, cut?: boolean): AlignMTB
    }

    BFMatcher: {
        new(normType?: int, crossCheck?: boolean): BFMatcher
    }

    BRISK: {
        new(thresh?: int, octaves?: int, patternScale?: float): BRISK
        new(radiusList: FloatVector|float[], numberList: IntVector|int[],
            dMax?: float, dMin?: float, indexChange?: IntVector|int[]): BRISK
        new(thresh: int, octaves: int, radiusList: FloatVector|float[],
            numberList: IntVector|int[], dMax?: float, dMin?: float,
            indexChange?: IntVector|int[]): BRISK
    }

    BackgroundSubtractor: {
    }

    BackgroundSubtractorMOG2: {
        new(history?: int, varThreshold?: double, detectShadows?: boolean): BackgroundSubtractorMOG2
    }

    CLAHE: {
        new(clipLimit?: double, tileGridSize?: SizeLike): CLAHE
    }

    CalibrateCRF: {
    }

    CalibrateDebevec: {
        new(samples?: int, lambda?: float, random?: boolean): CalibrateDebevec
    }

    CalibrateRobertson: {
        new(max_iter?: int, threshold?: float): CalibrateRobertson
    }

    CascadeClassifier: {
        new(): CascadeClassifier
        new(filename: string): CascadeClassifier
    }

    DescriptorMatcher: {
        new(emptyTrainData?: boolean): DescriptorMatcher
        new(descriptorMatcherType: string): DescriptorMatcher
        new(matcherType: DescriptorMatcher_MatcherType): DescriptorMatcher
    }

    FastFeatureDetector: {
        new(threshold?: int, nonmaxSuppression?: boolean,
            type?: FastFeatureDetector_DetectorType): FastFeatureDetector
    }

    Feature2D: {
    }

    GFTTDetector: {
        new(maxCorners?: int, qualityLevel?: double, minDistance?: double,
            blockSize?: int, useHarrisDetector?: boolean, k?: double): GFTTDetector
        new(maxCorners: int, qualityLevel: double, minDistance: double,
            blockSize: int, gradiantSize: int, useHarrisDetector?: boolean, k?: double): GFTTDetector
    }

    HOGDescriptor: {
        new(): HOGDescriptor
        new(_winSize: SizeLike, _blockSize: SizeLike, _blockStride: SizeLike,
            _cellSize: SizeLike, _nbins: int, _derivAperture?: int, _winSigma?: double,
            _histogramNormType?: HOGDescriptor_HistogramNormType, _L2HysThreshold?: double,
            _gammaCorrection?: boolean, _nlevels?: int, _signedGradient?: boolean): HOGDescriptor
        new(filename: string): HOGDescriptor
    }

    KAZE: {
        new(extended?: boolean, upright?: boolean, threshold?: float, nOctaves?: int,
            nOctaveLayers?: int, diffusivity?: KAZE_DiffusivityType): KAZE
    }

    MSER: {
        new(_delta?: int, _min_area?: int, _max_area?: int, _max_variation?: double,
            _min_diversity?: double, _max_evolution?: int, _area_threshold?: double,
            _min_margin?: double, _edge_blur_size?: int): MSER
    }

    MergeDebevec: {
        new(): MergeDebevec
    }

    MergeExposures: {
    }

    MergeMertens: {
        new(contrast_weight?: float, saturation_weight?: float, exposure_weight?: float): MergeMertens
    }

    MergeRobertson: {
        new(): MergeRobertson
    }

    ORB: {
        new(nfeatures?: int, scaleFactor?: float, nlevels?: int, edgeThreshold?: int,
            firstLevel?: int, WTA_K?: int, scoreType?: ORB_ScoreType, patchSize?: int,
            fastThreshold?: int): ORB
    }

    QRCodeDetector: {
        new(): QRCodeDetector
    }

    Tonemap: {
    }

    TonemapDrago: {
        new(gamma?: float, saturation?: float, bias?: float): TonemapDrago
    }

    TonemapMantiuk: {
        new(gamma?: float, scale?: float, saturation?: float): TonemapMantiuk
    }

    TonemapReinhard: {
        new(gamma?: float, intensity?: float, light_adapt?: float, color_adapt?: float): TonemapReinhard
    }

    dnn_Net: {
    }

    IntelligentScissorsMB: {
        new(): IntelligentScissorsMB
    }
}

export interface RangeLike {
    start: number
    end: number
}

export interface TermCriteriaLike {
    type: number
    maxCount: number
    epsilon: double
}

export interface SizeLike {
    width: number
    height: number
}

export type Size2fLike = SizeLike

export interface PointLike {
    readonly x: number
    readonly y: number
}

export type Point2fLike = PointLike

export interface RectLike {
    readonly x: number
    readonly y: number
    readonly width: number
    readonly height: number
}

export type Rect2fLike = RectLike

export interface RotatedRectLike {
    center: Point2fLike
    size: Size2fLike
    angle: float
}

export interface KeyPointLike {
    angle: float
    class_id: number
    octave: number
    pt: Point2fLike
    response: float
    size: number
}

export interface DMatchLike {
    queryIdx: number
    trainIdx: number
    imgIdx: number
    distance: float
}

export interface Scalar {
    [index: number]: double
}

export type ScalarLike = [double, double, double, double] | double[] | Scalar

export interface MinMaxLocLike {
    minVal: double
    maxVal: double
    minLoc: PointLike
    maxLoc: PointLike
}

export interface CircleLike {
    center: Point2fLike
    radius: float
}

export interface MomentsLike {
    m00: double
    m10: double
    m01: double
    m20: double
    m11: double
    m02: double
    m30: double
    m21: double
    m12: double
    m03: double
    mu20: double
    mu11: double
    mu02: double
    mu30: double
    mu21: double
    mu12: double
    mu03: double
    nu20: double
    nu11: double
    nu02: double
    nu30: double
    nu21: double
    nu12: double
    nu03: double
}

export interface ExceptionLike {
    code: number
    msg: string;
}

export interface Mat extends EmClassHandle {
    readonly rows: int
    readonly cols: int
    readonly matSize: int[]
    readonly step: int[]
    readonly data: Uint8Array
    readonly data8S: Int8Array
    readonly data16U: Uint16Array
    readonly data16S: Int16Array
    readonly data32S: Int32Array
    readonly data32F: Float32Array
    readonly data64F: Float64Array

    elemSize(): int
    elemSize1(): int
    channels(): int
    convertTo(m: Mat, rtype: int, alpha: double, beta: double): void
    convertTo(m: Mat, rtype: int): void
    convertTo(m: Mat, rtype: int, alpha: double): void
    total(): int
    row(y: int): Mat
    create(rows: int, cols: int, type: int): Mat
    create(size: SizeLike, type: int): Mat
    rowRange(startrow: int, endrow: int): Mat
    rowRange(r: RangeLike): Mat
    copyTo(mat: Mat): void
    copyTo(mat: Mat, mask: Mat): void
    type(): int
    empty(): boolean
    colRange(startcol: int, endcol: int): Mat
    colRange(r: RangeLike): Mat
    step1(i: int): int
    clone(): Mat
    depth(): int
    col(x: int): Mat
    dot(mat: Mat): double
    mul(mat: Mat, scale: double): Mat
    inv(type: int): Mat
    t(): Mat
    roi(rect: RectLike): Mat
    diag(d: int): Mat
    diag(): Mat
    isContinuous(): boolean
    setTo(scalar: ScalarLike): void
    setTo(scalar: ScalarLike, mask: Mat): void
    size(): SizeLike

    ptr(row: int): Uint8Array
    ptr(row: int, col: int): Uint8Array
    ucharPtr(row: int): Uint8Array
    ucharPtr(row: int, col: int): Uint8Array
    charPtr(row: int): Int8Array
    charPtr(row: int, col: int): Int8Array
    shortPtr(row: int): Int16Array
    shortPtr(row: int, col: int): Int16Array
    ushortPtr(row: int): Uint16Array
    ushortPtr(row: int, col: int): Uint16Array
    intPtr(row: int): Int32Array
    intPtr(row: int, col: int): Int32Array
    floatPtr(row: int): Float32Array
    floatPtr(row: int, col: int): Float32Array
    doublePtr(row: int): Float64Array
    doublePtr(row: int, col: int): Float64Array

    charAt(row: int): number
    charAt(row: int, col: int): number
    charAt(i0: int, i1: int, i2: int): number
    ucharAt(row: int): number
    ucharAt(row: int, col: int): number
    ucharAt(i0: int, i1: int, i2: int): number
    shortAt(row: int): number
    shortAt(row: int, col: int): number
    shortAt(i0: int, i1: int, i2: int): number
    ushortAt(row: int): number
    ushortAt(row: int, col: int): number
    ushortAt(i0: int, i1: int, i2: int): number
    intAt(row: int): number
    intAt(row: int, col: int): number
    intAt(i0: int, i1: int, i2: int): number
    floatAt(row: int): number
    floatAt(row: int, col: int): number
    floatAt(i0: int, i1: int, i2: int): number
    doubleAt(row: int): number
    doubleAt(row: int, col: int): number
    doubleAt(i0: int, i1: int, i2: int): number
}

export interface IntVector extends EmVector<int> { }
export interface FloatVector extends EmVector<float> { }
export interface DoubleVector extends EmVector<double> { }
export interface PointVector extends EmVector<PointLike> { }
export interface MatVector extends EmVector<Mat> { }
export interface RectVector extends EmVector<RectLike> { }
export interface KeyPointVector extends EmVector<KeyPointLike> { }
export interface DMatchVector extends EmVector<DMatchLike> { }
export interface DMatchVectorVector extends EmVector<DMatchVector> { }

export type AKAZE_DescriptorType = EmbindEnumEntity<AKAZE_DescriptorTypeEnumValues>;
interface AKAZE_DescriptorTypeEnumValues extends EmbindEnum {
    DESCRIPTOR_KAZE_UPRIGHT: AKAZE_DescriptorType
    DESCRIPTOR_KAZE: AKAZE_DescriptorType
    DESCRIPTOR_MLDB_UPRIGHT: AKAZE_DescriptorType
    DESCRIPTOR_MLDB: AKAZE_DescriptorType
}

export type AccessFlag = EmbindEnumEntity<AccessFlagEnumValues>;
interface AccessFlagEnumValues extends EmbindEnum {
    ACCESS_READ: AccessFlag
    ACCESS_WRITE: AccessFlag
    ACCESS_RW: AccessFlag
    ACCESS_MASK: AccessFlag
    ACCESS_FAST: AccessFlag
}

export type AdaptiveThresholdTypes = EmbindEnumEntity<AdaptiveThresholdTypesEnumValues>;
interface AdaptiveThresholdTypesEnumValues extends EmbindEnum {
    ADAPTIVE_THRESH_MEAN_C: AdaptiveThresholdTypes
    ADAPTIVE_THRESH_GAUSSIAN_C: AdaptiveThresholdTypes
}

export type AgastFeatureDetector_DetectorType = EmbindEnumEntity<AgastFeatureDetector_DetectorTypeEnumValues>;
interface AgastFeatureDetector_DetectorTypeEnumValues extends EmbindEnum {
    AGAST_5_8: AgastFeatureDetector_DetectorType
    AGAST_7_12d: AgastFeatureDetector_DetectorType
    AGAST_7_12s: AgastFeatureDetector_DetectorType
    OAST_9_16: AgastFeatureDetector_DetectorType
}

export type BorderTypes = EmbindEnumEntity<BorderTypesEnumValues>;
interface BorderTypesEnumValues extends EmbindEnum {
    BORDER_CONSTANT: BorderTypes
    BORDER_REPLICATE: BorderTypes
    BORDER_REFLECT: BorderTypes
    BORDER_WRAP: BorderTypes
    BORDER_REFLECT_101: BorderTypes
    BORDER_TRANSPARENT: BorderTypes
    BORDER_REFLECT101: BorderTypes
    BORDER_DEFAULT: BorderTypes
    BORDER_ISOLATED: BorderTypes
}

export type CirclesGridFinderParameters_GridType = EmbindEnumEntity<CirclesGridFinderParameters_GridTypeEnumValues>;
interface CirclesGridFinderParameters_GridTypeEnumValues extends EmbindEnum {
    SYMMETRIC_GRID: CirclesGridFinderParameters_GridType
    ASYMMETRIC_GRID: CirclesGridFinderParameters_GridType
}

export type CmpTypes = EmbindEnumEntity<CmpTypesEnumValues>;
interface CmpTypesEnumValues extends EmbindEnum {
    CMP_EQ: CmpTypes
    CMP_GT: CmpTypes
    CMP_GE: CmpTypes
    CMP_LT: CmpTypes
    CMP_LE: CmpTypes
    CMP_NE: CmpTypes
}

export type ColorConversionCodes = EmbindEnumEntity<ColorConversionCodesEnumValues>;
interface ColorConversionCodesEnumValues extends EmbindEnum {
    COLOR_BGR2BGRA: ColorConversionCodes
    COLOR_RGB2RGBA: ColorConversionCodes
    COLOR_BGRA2BGR: ColorConversionCodes
    COLOR_RGBA2RGB: ColorConversionCodes
    COLOR_BGR2RGBA: ColorConversionCodes
    COLOR_RGB2BGRA: ColorConversionCodes
    COLOR_RGBA2BGR: ColorConversionCodes
    COLOR_BGRA2RGB: ColorConversionCodes
    COLOR_BGR2RGB: ColorConversionCodes
    COLOR_RGB2BGR: ColorConversionCodes
    COLOR_BGRA2RGBA: ColorConversionCodes
    COLOR_RGBA2BGRA: ColorConversionCodes
    COLOR_BGR2GRAY: ColorConversionCodes
    COLOR_RGB2GRAY: ColorConversionCodes
    COLOR_GRAY2BGR: ColorConversionCodes
    COLOR_GRAY2RGB: ColorConversionCodes
    COLOR_GRAY2BGRA: ColorConversionCodes
    COLOR_GRAY2RGBA: ColorConversionCodes
    COLOR_BGRA2GRAY: ColorConversionCodes
    COLOR_RGBA2GRAY: ColorConversionCodes
    COLOR_BGR2BGR565: ColorConversionCodes
    COLOR_RGB2BGR565: ColorConversionCodes
    COLOR_BGR5652BGR: ColorConversionCodes
    COLOR_BGR5652RGB: ColorConversionCodes
    COLOR_BGRA2BGR565: ColorConversionCodes
    COLOR_RGBA2BGR565: ColorConversionCodes
    COLOR_BGR5652BGRA: ColorConversionCodes
    COLOR_BGR5652RGBA: ColorConversionCodes
    COLOR_GRAY2BGR565: ColorConversionCodes
    COLOR_BGR5652GRAY: ColorConversionCodes
    COLOR_BGR2BGR555: ColorConversionCodes
    COLOR_RGB2BGR555: ColorConversionCodes
    COLOR_BGR5552BGR: ColorConversionCodes
    COLOR_BGR5552RGB: ColorConversionCodes
    COLOR_BGRA2BGR555: ColorConversionCodes
    COLOR_RGBA2BGR555: ColorConversionCodes
    COLOR_BGR5552BGRA: ColorConversionCodes
    COLOR_BGR5552RGBA: ColorConversionCodes
    COLOR_GRAY2BGR555: ColorConversionCodes
    COLOR_BGR5552GRAY: ColorConversionCodes
    COLOR_BGR2XYZ: ColorConversionCodes
    COLOR_RGB2XYZ: ColorConversionCodes
    COLOR_XYZ2BGR: ColorConversionCodes
    COLOR_XYZ2RGB: ColorConversionCodes
    COLOR_BGR2YCrCb: ColorConversionCodes
    COLOR_RGB2YCrCb: ColorConversionCodes
    COLOR_YCrCb2BGR: ColorConversionCodes
    COLOR_YCrCb2RGB: ColorConversionCodes
    COLOR_BGR2HSV: ColorConversionCodes
    COLOR_RGB2HSV: ColorConversionCodes
    COLOR_BGR2Lab: ColorConversionCodes
    COLOR_RGB2Lab: ColorConversionCodes
    COLOR_BGR2Luv: ColorConversionCodes
    COLOR_RGB2Luv: ColorConversionCodes
    COLOR_BGR2HLS: ColorConversionCodes
    COLOR_RGB2HLS: ColorConversionCodes
    COLOR_HSV2BGR: ColorConversionCodes
    COLOR_HSV2RGB: ColorConversionCodes
    COLOR_Lab2BGR: ColorConversionCodes
    COLOR_Lab2RGB: ColorConversionCodes
    COLOR_Luv2BGR: ColorConversionCodes
    COLOR_Luv2RGB: ColorConversionCodes
    COLOR_HLS2BGR: ColorConversionCodes
    COLOR_HLS2RGB: ColorConversionCodes
    COLOR_BGR2HSV_FULL: ColorConversionCodes
    COLOR_RGB2HSV_FULL: ColorConversionCodes
    COLOR_BGR2HLS_FULL: ColorConversionCodes
    COLOR_RGB2HLS_FULL: ColorConversionCodes
    COLOR_HSV2BGR_FULL: ColorConversionCodes
    COLOR_HSV2RGB_FULL: ColorConversionCodes
    COLOR_HLS2BGR_FULL: ColorConversionCodes
    COLOR_HLS2RGB_FULL: ColorConversionCodes
    COLOR_LBGR2Lab: ColorConversionCodes
    COLOR_LRGB2Lab: ColorConversionCodes
    COLOR_LBGR2Luv: ColorConversionCodes
    COLOR_LRGB2Luv: ColorConversionCodes
    COLOR_Lab2LBGR: ColorConversionCodes
    COLOR_Lab2LRGB: ColorConversionCodes
    COLOR_Luv2LBGR: ColorConversionCodes
    COLOR_Luv2LRGB: ColorConversionCodes
    COLOR_BGR2YUV: ColorConversionCodes
    COLOR_RGB2YUV: ColorConversionCodes
    COLOR_YUV2BGR: ColorConversionCodes
    COLOR_YUV2RGB: ColorConversionCodes
    COLOR_YUV2RGB_NV12: ColorConversionCodes
    COLOR_YUV2BGR_NV12: ColorConversionCodes
    COLOR_YUV2RGB_NV21: ColorConversionCodes
    COLOR_YUV2BGR_NV21: ColorConversionCodes
    COLOR_YUV420sp2RGB: ColorConversionCodes
    COLOR_YUV420sp2BGR: ColorConversionCodes
    COLOR_YUV2RGBA_NV12: ColorConversionCodes
    COLOR_YUV2BGRA_NV12: ColorConversionCodes
    COLOR_YUV2RGBA_NV21: ColorConversionCodes
    COLOR_YUV2BGRA_NV21: ColorConversionCodes
    COLOR_YUV420sp2RGBA: ColorConversionCodes
    COLOR_YUV420sp2BGRA: ColorConversionCodes
    COLOR_YUV2RGB_YV12: ColorConversionCodes
    COLOR_YUV2BGR_YV12: ColorConversionCodes
    COLOR_YUV2RGB_IYUV: ColorConversionCodes
    COLOR_YUV2BGR_IYUV: ColorConversionCodes
    COLOR_YUV2RGB_I420: ColorConversionCodes
    COLOR_YUV2BGR_I420: ColorConversionCodes
    COLOR_YUV420p2RGB: ColorConversionCodes
    COLOR_YUV420p2BGR: ColorConversionCodes
    COLOR_YUV2RGBA_YV12: ColorConversionCodes
    COLOR_YUV2BGRA_YV12: ColorConversionCodes
    COLOR_YUV2RGBA_IYUV: ColorConversionCodes
    COLOR_YUV2BGRA_IYUV: ColorConversionCodes
    COLOR_YUV2RGBA_I420: ColorConversionCodes
    COLOR_YUV2BGRA_I420: ColorConversionCodes
    COLOR_YUV420p2RGBA: ColorConversionCodes
    COLOR_YUV420p2BGRA: ColorConversionCodes
    COLOR_YUV2GRAY_420: ColorConversionCodes
    COLOR_YUV2GRAY_NV21: ColorConversionCodes
    COLOR_YUV2GRAY_NV12: ColorConversionCodes
    COLOR_YUV2GRAY_YV12: ColorConversionCodes
    COLOR_YUV2GRAY_IYUV: ColorConversionCodes
    COLOR_YUV2GRAY_I420: ColorConversionCodes
    COLOR_YUV420sp2GRAY: ColorConversionCodes
    COLOR_YUV420p2GRAY: ColorConversionCodes
    COLOR_YUV2RGB_UYVY: ColorConversionCodes
    COLOR_YUV2BGR_UYVY: ColorConversionCodes
    COLOR_YUV2RGB_Y422: ColorConversionCodes
    COLOR_YUV2BGR_Y422: ColorConversionCodes
    COLOR_YUV2RGB_UYNV: ColorConversionCodes
    COLOR_YUV2BGR_UYNV: ColorConversionCodes
    COLOR_YUV2RGBA_UYVY: ColorConversionCodes
    COLOR_YUV2BGRA_UYVY: ColorConversionCodes
    COLOR_YUV2RGBA_Y422: ColorConversionCodes
    COLOR_YUV2BGRA_Y422: ColorConversionCodes
    COLOR_YUV2RGBA_UYNV: ColorConversionCodes
    COLOR_YUV2BGRA_UYNV: ColorConversionCodes
    COLOR_YUV2RGB_YUY2: ColorConversionCodes
    COLOR_YUV2BGR_YUY2: ColorConversionCodes
    COLOR_YUV2RGB_YVYU: ColorConversionCodes
    COLOR_YUV2BGR_YVYU: ColorConversionCodes
    COLOR_YUV2RGB_YUYV: ColorConversionCodes
    COLOR_YUV2BGR_YUYV: ColorConversionCodes
    COLOR_YUV2RGB_YUNV: ColorConversionCodes
    COLOR_YUV2BGR_YUNV: ColorConversionCodes
    COLOR_YUV2RGBA_YUY2: ColorConversionCodes
    COLOR_YUV2BGRA_YUY2: ColorConversionCodes
    COLOR_YUV2RGBA_YVYU: ColorConversionCodes
    COLOR_YUV2BGRA_YVYU: ColorConversionCodes
    COLOR_YUV2RGBA_YUYV: ColorConversionCodes
    COLOR_YUV2BGRA_YUYV: ColorConversionCodes
    COLOR_YUV2RGBA_YUNV: ColorConversionCodes
    COLOR_YUV2BGRA_YUNV: ColorConversionCodes
    COLOR_YUV2GRAY_UYVY: ColorConversionCodes
    COLOR_YUV2GRAY_YUY2: ColorConversionCodes
    COLOR_YUV2GRAY_Y422: ColorConversionCodes
    COLOR_YUV2GRAY_UYNV: ColorConversionCodes
    COLOR_YUV2GRAY_YVYU: ColorConversionCodes
    COLOR_YUV2GRAY_YUYV: ColorConversionCodes
    COLOR_YUV2GRAY_YUNV: ColorConversionCodes
    COLOR_RGBA2mRGBA: ColorConversionCodes
    COLOR_mRGBA2RGBA: ColorConversionCodes
    COLOR_RGB2YUV_I420: ColorConversionCodes
    COLOR_BGR2YUV_I420: ColorConversionCodes
    COLOR_RGB2YUV_IYUV: ColorConversionCodes
    COLOR_BGR2YUV_IYUV: ColorConversionCodes
    COLOR_RGBA2YUV_I420: ColorConversionCodes
    COLOR_BGRA2YUV_I420: ColorConversionCodes
    COLOR_RGBA2YUV_IYUV: ColorConversionCodes
    COLOR_BGRA2YUV_IYUV: ColorConversionCodes
    COLOR_RGB2YUV_YV12: ColorConversionCodes
    COLOR_BGR2YUV_YV12: ColorConversionCodes
    COLOR_RGBA2YUV_YV12: ColorConversionCodes
    COLOR_BGRA2YUV_YV12: ColorConversionCodes
    COLOR_BayerBG2BGR: ColorConversionCodes
    COLOR_BayerGB2BGR: ColorConversionCodes
    COLOR_BayerRG2BGR: ColorConversionCodes
    COLOR_BayerGR2BGR: ColorConversionCodes
    COLOR_BayerBG2RGB: ColorConversionCodes
    COLOR_BayerGB2RGB: ColorConversionCodes
    COLOR_BayerRG2RGB: ColorConversionCodes
    COLOR_BayerGR2RGB: ColorConversionCodes
    COLOR_BayerBG2GRAY: ColorConversionCodes
    COLOR_BayerGB2GRAY: ColorConversionCodes
    COLOR_BayerRG2GRAY: ColorConversionCodes
    COLOR_BayerGR2GRAY: ColorConversionCodes
    COLOR_BayerBG2BGR_VNG: ColorConversionCodes
    COLOR_BayerGB2BGR_VNG: ColorConversionCodes
    COLOR_BayerRG2BGR_VNG: ColorConversionCodes
    COLOR_BayerGR2BGR_VNG: ColorConversionCodes
    COLOR_BayerBG2RGB_VNG: ColorConversionCodes
    COLOR_BayerGB2RGB_VNG: ColorConversionCodes
    COLOR_BayerRG2RGB_VNG: ColorConversionCodes
    COLOR_BayerGR2RGB_VNG: ColorConversionCodes
    COLOR_BayerBG2BGR_EA: ColorConversionCodes
    COLOR_BayerGB2BGR_EA: ColorConversionCodes
    COLOR_BayerRG2BGR_EA: ColorConversionCodes
    COLOR_BayerGR2BGR_EA: ColorConversionCodes
    COLOR_BayerBG2RGB_EA: ColorConversionCodes
    COLOR_BayerGB2RGB_EA: ColorConversionCodes
    COLOR_BayerRG2RGB_EA: ColorConversionCodes
    COLOR_BayerGR2RGB_EA: ColorConversionCodes
    COLOR_BayerBG2BGRA: ColorConversionCodes
    COLOR_BayerGB2BGRA: ColorConversionCodes
    COLOR_BayerRG2BGRA: ColorConversionCodes
    COLOR_BayerGR2BGRA: ColorConversionCodes
    COLOR_BayerBG2RGBA: ColorConversionCodes
    COLOR_BayerGB2RGBA: ColorConversionCodes
    COLOR_BayerRG2RGBA: ColorConversionCodes
    COLOR_BayerGR2RGBA: ColorConversionCodes
    COLOR_COLORCVT_MAX: ColorConversionCodes
}

export type ColormapTypes = EmbindEnumEntity<ColormapTypesEnumValues>;
interface ColormapTypesEnumValues extends EmbindEnum {
    COLORMAP_AUTUMN: ColormapTypes
    COLORMAP_BONE: ColormapTypes
    COLORMAP_JET: ColormapTypes
    COLORMAP_WINTER: ColormapTypes
    COLORMAP_RAINBOW: ColormapTypes
    COLORMAP_OCEAN: ColormapTypes
    COLORMAP_SUMMER: ColormapTypes
    COLORMAP_SPRING: ColormapTypes
    COLORMAP_COOL: ColormapTypes
    COLORMAP_HSV: ColormapTypes
    COLORMAP_PINK: ColormapTypes
    COLORMAP_HOT: ColormapTypes
    COLORMAP_PARULA: ColormapTypes
    COLORMAP_MAGMA: ColormapTypes
    COLORMAP_INFERNO: ColormapTypes
    COLORMAP_PLASMA: ColormapTypes
    COLORMAP_VIRIDIS: ColormapTypes
    COLORMAP_CIVIDIS: ColormapTypes
    COLORMAP_TWILIGHT: ColormapTypes
    COLORMAP_TWILIGHT_SHIFTED: ColormapTypes
    COLORMAP_TURBO: ColormapTypes
    COLORMAP_DEEPGREEN: ColormapTypes
}

export type ConnectedComponentsAlgorithmsTypes = EmbindEnumEntity<ConnectedComponentsAlgorithmsTypesEnumValues>;
interface ConnectedComponentsAlgorithmsTypesEnumValues extends EmbindEnum {
    CCL_WU: ConnectedComponentsAlgorithmsTypes
    CCL_DEFAULT: ConnectedComponentsAlgorithmsTypes
    CCL_GRANA: ConnectedComponentsAlgorithmsTypes
}

export type ConnectedComponentsTypes = EmbindEnumEntity<ConnectedComponentsTypesEnumValues>;
interface ConnectedComponentsTypesEnumValues extends EmbindEnum {
    CC_STAT_LEFT: ConnectedComponentsTypes
    CC_STAT_TOP: ConnectedComponentsTypes
    CC_STAT_WIDTH: ConnectedComponentsTypes
    CC_STAT_HEIGHT: ConnectedComponentsTypes
    CC_STAT_AREA: ConnectedComponentsTypes
    CC_STAT_MAX: ConnectedComponentsTypes
}

export type ContourApproximationModes = EmbindEnumEntity<ContourApproximationModesEnumValues>;
interface ContourApproximationModesEnumValues extends EmbindEnum {
    CHAIN_APPROX_NONE: ContourApproximationModes
    CHAIN_APPROX_SIMPLE: ContourApproximationModes
    CHAIN_APPROX_TC89_L1: ContourApproximationModes
    CHAIN_APPROX_TC89_KCOS: ContourApproximationModes
}

export type CovarFlags = EmbindEnumEntity<CovarFlagsEnumValues>;
interface CovarFlagsEnumValues extends EmbindEnum {
    COVAR_SCRAMBLED: CovarFlags
    COVAR_NORMAL: CovarFlags
    COVAR_USE_AVG: CovarFlags
    COVAR_SCALE: CovarFlags
    COVAR_ROWS: CovarFlags
    COVAR_COLS: CovarFlags
}

export type DecompTypes = EmbindEnumEntity<DecompTypesEnumValues>;
interface DecompTypesEnumValues extends EmbindEnum {
    DECOMP_LU: DecompTypes
    DECOMP_SVD: DecompTypes
    DECOMP_EIG: DecompTypes
    DECOMP_CHOLESKY: DecompTypes
    DECOMP_QR: DecompTypes
    DECOMP_NORMAL: DecompTypes
}

export type DescriptorMatcher_MatcherType = EmbindEnumEntity<DescriptorMatcher_MatcherTypeEnumValues>;
interface DescriptorMatcher_MatcherTypeEnumValues extends EmbindEnum {
    FLANNBASED: DescriptorMatcher_MatcherType
    BRUTEFORCE: DescriptorMatcher_MatcherType
    BRUTEFORCE_L1: DescriptorMatcher_MatcherType
    BRUTEFORCE_HAMMING: DescriptorMatcher_MatcherType
    BRUTEFORCE_HAMMINGLUT: DescriptorMatcher_MatcherType
    BRUTEFORCE_SL2: DescriptorMatcher_MatcherType
}

export type DftFlags = EmbindEnumEntity<DftFlagsEnumValues>;
interface DftFlagsEnumValues extends EmbindEnum {
    DFT_INVERSE: DftFlags
    DFT_SCALE: DftFlags
    DFT_ROWS: DftFlags
    DFT_COMPLEX_OUTPUT: DftFlags
    DFT_REAL_OUTPUT: DftFlags
    DFT_COMPLEX_INPUT: DftFlags
    DCT_INVERSE: DftFlags
    DCT_ROWS: DftFlags
}

export type DistanceTransformLabelTypes = EmbindEnumEntity<DistanceTransformLabelTypesEnumValues>;
interface DistanceTransformLabelTypesEnumValues extends EmbindEnum {
    DIST_LABEL_CCOMP: DistanceTransformLabelTypes
    DIST_LABEL_PIXEL: DistanceTransformLabelTypes
}

export type DistanceTransformMasks = EmbindEnumEntity<DistanceTransformMasksEnumValues>;
interface DistanceTransformMasksEnumValues extends EmbindEnum {
    DIST_MASK_3: DistanceTransformMasks
    DIST_MASK_5: DistanceTransformMasks
    DIST_MASK_PRECISE: DistanceTransformMasks
}

export type DistanceTypes = EmbindEnumEntity<DistanceTypesEnumValues>;
interface DistanceTypesEnumValues extends EmbindEnum {
    DIST_USER: DistanceTypes
    DIST_L1: DistanceTypes
    DIST_L2: DistanceTypes
    DIST_C: DistanceTypes
    DIST_L12: DistanceTypes
    DIST_FAIR: DistanceTypes
    DIST_WELSCH: DistanceTypes
    DIST_HUBER: DistanceTypes
}

export type DrawMatchesFlags = EmbindEnumEntity<DrawMatchesFlagsEnumValues>;
interface DrawMatchesFlagsEnumValues extends EmbindEnum {
    DEFAULT: DrawMatchesFlags
    DRAW_OVER_OUTIMG: DrawMatchesFlags
    NOT_DRAW_SINGLE_POINTS: DrawMatchesFlags
    DRAW_RICH_KEYPOINTS: DrawMatchesFlags
}

export type FastFeatureDetector_DetectorType = EmbindEnumEntity<FastFeatureDetector_DetectorTypeEnumValues>;
interface FastFeatureDetector_DetectorTypeEnumValues extends EmbindEnum {
    TYPE_5_8: FastFeatureDetector_DetectorType
    TYPE_7_12: FastFeatureDetector_DetectorType
    TYPE_9_16: FastFeatureDetector_DetectorType
}

export type FileStorage_Mode = EmbindEnumEntity<FileStorage_ModeEnumValues>;
interface FileStorage_ModeEnumValues extends EmbindEnum {
    READ: FileStorage_Mode
    WRITE: FileStorage_Mode
    APPEND: FileStorage_Mode
    MEMORY: FileStorage_Mode
    FORMAT_MASK: FileStorage_Mode
    FORMAT_AUTO: FileStorage_Mode
    FORMAT_XML: FileStorage_Mode
    FORMAT_YAML: FileStorage_Mode
    FORMAT_JSON: FileStorage_Mode
    BASE64: FileStorage_Mode
    WRITE_BASE64: FileStorage_Mode
}

export type FileStorage_State = EmbindEnumEntity<FileStorage_StateEnumValues>;
interface FileStorage_StateEnumValues extends EmbindEnum {
    UNDEFINED: FileStorage_State
    VALUE_EXPECTED: FileStorage_State
    NAME_EXPECTED: FileStorage_State
    INSIDE_MAP: FileStorage_State
}

export type FloodFillFlags = EmbindEnumEntity<FloodFillFlagsEnumValues>;
interface FloodFillFlagsEnumValues extends EmbindEnum {
    FLOODFILL_FIXED_RANGE: FloodFillFlags
    FLOODFILL_MASK_ONLY: FloodFillFlags
}

export type Formatter_FormatType = EmbindEnumEntity<Formatter_FormatTypeEnumValues>;
interface Formatter_FormatTypeEnumValues extends EmbindEnum {
    FMT_DEFAULT: Formatter_FormatType
    FMT_MATLAB: Formatter_FormatType
    FMT_CSV: Formatter_FormatType
    FMT_PYTHON: Formatter_FormatType
    FMT_NUMPY: Formatter_FormatType
    FMT_C: Formatter_FormatType
}

export type GemmFlags = EmbindEnumEntity<GemmFlagsEnumValues>;
interface GemmFlagsEnumValues extends EmbindEnum {
    GEMM_1_T: GemmFlags
    GEMM_2_T: GemmFlags
    GEMM_3_T: GemmFlags
}

export type GrabCutClasses = EmbindEnumEntity<GrabCutClassesEnumValues>;
interface GrabCutClassesEnumValues extends EmbindEnum {
    GC_BGD: GrabCutClasses
    GC_FGD: GrabCutClasses
    GC_PR_BGD: GrabCutClasses
    GC_PR_FGD: GrabCutClasses
}

export type GrabCutModes = EmbindEnumEntity<GrabCutModesEnumValues>;
interface GrabCutModesEnumValues extends EmbindEnum {
    GC_INIT_WITH_RECT: GrabCutModes
    GC_INIT_WITH_MASK: GrabCutModes
    GC_EVAL: GrabCutModes
    GC_EVAL_FREEZE_MODEL: GrabCutModes
}

export type HOGDescriptor_DescriptorStorageFormat = EmbindEnumEntity<HOGDescriptor_DescriptorStorageFormatEnumValues>;
interface HOGDescriptor_DescriptorStorageFormatEnumValues extends EmbindEnum {
    DESCR_FORMAT_COL_BY_COL: HOGDescriptor_DescriptorStorageFormat
    DESCR_FORMAT_ROW_BY_ROW: HOGDescriptor_DescriptorStorageFormat
}

export type HOGDescriptor_HistogramNormType = EmbindEnumEntity<HOGDescriptor_HistogramNormTypeEnumValues>;
interface HOGDescriptor_HistogramNormTypeEnumValues extends EmbindEnum {
    L2Hys: HOGDescriptor_HistogramNormType
}

export type HandEyeCalibrationMethod = EmbindEnumEntity<HandEyeCalibrationMethodEnumValues>;
interface HandEyeCalibrationMethodEnumValues extends EmbindEnum {
    CALIB_HAND_EYE_TSAI: HandEyeCalibrationMethod
    CALIB_HAND_EYE_PARK: HandEyeCalibrationMethod
    CALIB_HAND_EYE_HORAUD: HandEyeCalibrationMethod
    CALIB_HAND_EYE_ANDREFF: HandEyeCalibrationMethod
    CALIB_HAND_EYE_DANIILIDIS: HandEyeCalibrationMethod
}

export type HersheyFonts = EmbindEnumEntity<HersheyFontsEnumValues>;
interface HersheyFontsEnumValues extends EmbindEnum {
    FONT_HERSHEY_SIMPLEX: HersheyFonts
    FONT_HERSHEY_PLAIN: HersheyFonts
    FONT_HERSHEY_DUPLEX: HersheyFonts
    FONT_HERSHEY_COMPLEX: HersheyFonts
    FONT_HERSHEY_TRIPLEX: HersheyFonts
    FONT_HERSHEY_COMPLEX_SMALL: HersheyFonts
    FONT_HERSHEY_SCRIPT_SIMPLEX: HersheyFonts
    FONT_HERSHEY_SCRIPT_COMPLEX: HersheyFonts
    FONT_ITALIC: HersheyFonts
}

export type HistCompMethods = EmbindEnumEntity<HistCompMethodsEnumValues>;
interface HistCompMethodsEnumValues extends EmbindEnum {
    HISTCMP_CORREL: HistCompMethods
    HISTCMP_CHISQR: HistCompMethods
    HISTCMP_INTERSECT: HistCompMethods
    HISTCMP_BHATTACHARYYA: HistCompMethods
    HISTCMP_HELLINGER: HistCompMethods
    HISTCMP_CHISQR_ALT: HistCompMethods
    HISTCMP_KL_DIV: HistCompMethods
}

export type HoughModes = EmbindEnumEntity<HoughModesEnumValues>;
interface HoughModesEnumValues extends EmbindEnum {
    HOUGH_STANDARD: HoughModes
    HOUGH_PROBABILISTIC: HoughModes
    HOUGH_MULTI_SCALE: HoughModes
    HOUGH_GRADIENT: HoughModes
    HOUGH_GRADIENT_ALT: HoughModes
}

export type InterpolationFlags = EmbindEnumEntity<InterpolationFlagsEnumValues>;
interface InterpolationFlagsEnumValues extends EmbindEnum {
    INTER_NEAREST: InterpolationFlags
    INTER_LINEAR: InterpolationFlags
    INTER_CUBIC: InterpolationFlags
    INTER_AREA: InterpolationFlags
    INTER_LANCZOS4: InterpolationFlags
    INTER_LINEAR_EXACT: InterpolationFlags
    INTER_NEAREST_EXACT: InterpolationFlags
    INTER_MAX: InterpolationFlags
    WARP_FILL_OUTLIERS: InterpolationFlags
    WARP_INVERSE_MAP: InterpolationFlags
}

export type InterpolationMasks = EmbindEnumEntity<InterpolationMasksEnumValues>;
interface InterpolationMasksEnumValues extends EmbindEnum {
    INTER_BITS: InterpolationMasks
    INTER_BITS2: InterpolationMasks
    INTER_TAB_SIZE: InterpolationMasks
    INTER_TAB_SIZE2: InterpolationMasks
}

export type KAZE_DiffusivityType = EmbindEnumEntity<KAZE_DiffusivityTypeEnumValues>;
interface KAZE_DiffusivityTypeEnumValues extends EmbindEnum {
    DIFF_PM_G1: KAZE_DiffusivityType
    DIFF_PM_G2: KAZE_DiffusivityType
    DIFF_WEICKERT: KAZE_DiffusivityType
    DIFF_CHARBONNIER: KAZE_DiffusivityType
}

export type KmeansFlags = EmbindEnumEntity<KmeansFlagsEnumValues>;
interface KmeansFlagsEnumValues extends EmbindEnum {
    KMEANS_RANDOM_CENTERS: KmeansFlags
    KMEANS_PP_CENTERS: KmeansFlags
    KMEANS_USE_INITIAL_LABELS: KmeansFlags
}

export type LineSegmentDetectorModes = EmbindEnumEntity<LineSegmentDetectorModesEnumValues>;
interface LineSegmentDetectorModesEnumValues extends EmbindEnum {
    LSD_REFINE_NONE: LineSegmentDetectorModes
    LSD_REFINE_STD: LineSegmentDetectorModes
    LSD_REFINE_ADV: LineSegmentDetectorModes
}

export type LineTypes = EmbindEnumEntity<LineTypesEnumValues>;
interface LineTypesEnumValues extends EmbindEnum {
    FILLED: LineTypes
    LINE_4: LineTypes
    LINE_8: LineTypes
    LINE_AA: LineTypes
}

export type LocalOptimMethod = EmbindEnumEntity<LocalOptimMethodEnumValues>;
interface LocalOptimMethodEnumValues extends EmbindEnum {
    LOCAL_OPTIM_NULL: LocalOptimMethod
    LOCAL_OPTIM_INNER_LO: LocalOptimMethod
    LOCAL_OPTIM_INNER_AND_ITER_LO: LocalOptimMethod
    LOCAL_OPTIM_GC: LocalOptimMethod
    LOCAL_OPTIM_SIGMA: LocalOptimMethod
}

export type MarkerTypes = EmbindEnumEntity<MarkerTypesEnumValues>;
interface MarkerTypesEnumValues extends EmbindEnum {
    MARKER_CROSS: MarkerTypes
    MARKER_TILTED_CROSS: MarkerTypes
    MARKER_STAR: MarkerTypes
    MARKER_DIAMOND: MarkerTypes
    MARKER_SQUARE: MarkerTypes
    MARKER_TRIANGLE_UP: MarkerTypes
    MARKER_TRIANGLE_DOWN: MarkerTypes
}

export type MorphShapes = EmbindEnumEntity<MorphShapesEnumValues>;
interface MorphShapesEnumValues extends EmbindEnum {
    MORPH_RECT: MorphShapes
    MORPH_CROSS: MorphShapes
    MORPH_ELLIPSE: MorphShapes
}

export type MorphTypes = EmbindEnumEntity<MorphTypesEnumValues>;
interface MorphTypesEnumValues extends EmbindEnum {
    MORPH_ERODE: MorphTypes
    MORPH_DILATE: MorphTypes
    MORPH_OPEN: MorphTypes
    MORPH_CLOSE: MorphTypes
    MORPH_GRADIENT: MorphTypes
    MORPH_TOPHAT: MorphTypes
    MORPH_BLACKHAT: MorphTypes
    MORPH_HITMISS: MorphTypes
}

export type NeighborSearchMethod = EmbindEnumEntity<NeighborSearchMethodEnumValues>;
interface NeighborSearchMethodEnumValues extends EmbindEnum {
    NEIGH_FLANN_KNN: NeighborSearchMethod
    NEIGH_GRID: NeighborSearchMethod
    NEIGH_FLANN_RADIUS: NeighborSearchMethod
}

export type NormTypes = EmbindEnumEntity<NormTypesEnumValues>;
interface NormTypesEnumValues extends EmbindEnum {
    NORM_INF: NormTypes
    NORM_L1: NormTypes
    NORM_L2: NormTypes
    NORM_L2SQR: NormTypes
    NORM_HAMMING: NormTypes
    NORM_HAMMING2: NormTypes
    NORM_TYPE_MASK: NormTypes
    NORM_RELATIVE: NormTypes
    NORM_MINMAX: NormTypes
}

export type ORB_ScoreType = EmbindEnumEntity<ORB_ScoreTypeEnumValues>;
interface ORB_ScoreTypeEnumValues extends EmbindEnum {
    HARRIS_SCORE: ORB_ScoreType
    FAST_SCORE: ORB_ScoreType
}

export type PCA_Flags = EmbindEnumEntity<PCA_FlagsEnumValues>;
interface PCA_FlagsEnumValues extends EmbindEnum {
    DATA_AS_ROW: PCA_Flags
    DATA_AS_COL: PCA_Flags
    USE_AVG: PCA_Flags
}

export type Param = EmbindEnumEntity<ParamEnumValues>;
interface ParamEnumValues extends EmbindEnum {
    INT: Param
    BOOLEAN: Param
    REAL: Param
    STRING: Param
    MAT: Param
    MAT_VECTOR: Param
    ALGORITHM: Param
    FLOAT: Param
    UNSIGNED_INT: Param
    UINT64: Param
    UCHAR: Param
    SCALAR: Param
}

export type QuatAssumeType = EmbindEnumEntity<QuatAssumeTypeEnumValues>;
interface QuatAssumeTypeEnumValues extends EmbindEnum {
    QUAT_ASSUME_NOT_UNIT: QuatAssumeType
    QUAT_ASSUME_UNIT: QuatAssumeType
}

export type QuatEnum_EulerAnglesType = EmbindEnumEntity<QuatEnum_EulerAnglesTypeEnumValues>;
interface QuatEnum_EulerAnglesTypeEnumValues extends EmbindEnum {
    INT_XYZ: QuatEnum_EulerAnglesType
    INT_XZY: QuatEnum_EulerAnglesType
    INT_YXZ: QuatEnum_EulerAnglesType
    INT_YZX: QuatEnum_EulerAnglesType
    INT_ZXY: QuatEnum_EulerAnglesType
    INT_ZYX: QuatEnum_EulerAnglesType
    INT_XYX: QuatEnum_EulerAnglesType
    INT_XZX: QuatEnum_EulerAnglesType
    INT_YXY: QuatEnum_EulerAnglesType
    INT_YZY: QuatEnum_EulerAnglesType
    INT_ZXZ: QuatEnum_EulerAnglesType
    INT_ZYZ: QuatEnum_EulerAnglesType
    EXT_XYZ: QuatEnum_EulerAnglesType
    EXT_XZY: QuatEnum_EulerAnglesType
    EXT_YXZ: QuatEnum_EulerAnglesType
    EXT_YZX: QuatEnum_EulerAnglesType
    EXT_ZXY: QuatEnum_EulerAnglesType
    EXT_ZYX: QuatEnum_EulerAnglesType
    EXT_XYX: QuatEnum_EulerAnglesType
    EXT_XZX: QuatEnum_EulerAnglesType
    EXT_YXY: QuatEnum_EulerAnglesType
    EXT_YZY: QuatEnum_EulerAnglesType
    EXT_ZXZ: QuatEnum_EulerAnglesType
    EXT_ZYZ: QuatEnum_EulerAnglesType
    EULER_ANGLES_MAX_VALUE: QuatEnum_EulerAnglesType
}

export type RectanglesIntersectTypes = EmbindEnumEntity<RectanglesIntersectTypesEnumValues>;
interface RectanglesIntersectTypesEnumValues extends EmbindEnum {
    INTERSECT_NONE: RectanglesIntersectTypes
    INTERSECT_PARTIAL: RectanglesIntersectTypes
    INTERSECT_FULL: RectanglesIntersectTypes
}

export type ReduceTypes = EmbindEnumEntity<ReduceTypesEnumValues>;
interface ReduceTypesEnumValues extends EmbindEnum {
    REDUCE_SUM: ReduceTypes
    REDUCE_AVG: ReduceTypes
    REDUCE_MAX: ReduceTypes
    REDUCE_MIN: ReduceTypes
}

export type RetrievalModes = EmbindEnumEntity<RetrievalModesEnumValues>;
interface RetrievalModesEnumValues extends EmbindEnum {
    RETR_EXTERNAL: RetrievalModes
    RETR_LIST: RetrievalModes
    RETR_CCOMP: RetrievalModes
    RETR_TREE: RetrievalModes
    RETR_FLOODFILL: RetrievalModes
}

export type RobotWorldHandEyeCalibrationMethod = EmbindEnumEntity<RobotWorldHandEyeCalibrationMethodEnumValues>;
interface RobotWorldHandEyeCalibrationMethodEnumValues extends EmbindEnum {
    CALIB_ROBOT_WORLD_HAND_EYE_SHAH: RobotWorldHandEyeCalibrationMethod
    CALIB_ROBOT_WORLD_HAND_EYE_LI: RobotWorldHandEyeCalibrationMethod
}

export type RotateFlags = EmbindEnumEntity<RotateFlagsEnumValues>;
interface RotateFlagsEnumValues extends EmbindEnum {
    ROTATE_90_CLOCKWISE: RotateFlags
    ROTATE_180: RotateFlags
    ROTATE_90_COUNTERCLOCKWISE: RotateFlags
}

export type SVD_Flags = EmbindEnumEntity<SVD_FlagsEnumValues>;
interface SVD_FlagsEnumValues extends EmbindEnum {
    MODIFY_A: SVD_Flags
    NO_UV: SVD_Flags
    FULL_UV: SVD_Flags
}

export type SamplingMethod = EmbindEnumEntity<SamplingMethodEnumValues>;
interface SamplingMethodEnumValues extends EmbindEnum {
    SAMPLING_UNIFORM: SamplingMethod
    SAMPLING_PROGRESSIVE_NAPSAC: SamplingMethod
    SAMPLING_NAPSAC: SamplingMethod
    SAMPLING_PROSAC: SamplingMethod
}

export type ScoreMethod = EmbindEnumEntity<ScoreMethodEnumValues>;
interface ScoreMethodEnumValues extends EmbindEnum {
    SCORE_METHOD_RANSAC: ScoreMethod
    SCORE_METHOD_MSAC: ScoreMethod
    SCORE_METHOD_MAGSAC: ScoreMethod
    SCORE_METHOD_LMEDS: ScoreMethod
}

export type ShapeMatchModes = EmbindEnumEntity<ShapeMatchModesEnumValues>;
interface ShapeMatchModesEnumValues extends EmbindEnum {
    CONTOURS_MATCH_I1: ShapeMatchModes
    CONTOURS_MATCH_I2: ShapeMatchModes
    CONTOURS_MATCH_I3: ShapeMatchModes
}

export type SolveLPResult = EmbindEnumEntity<SolveLPResultEnumValues>;
interface SolveLPResultEnumValues extends EmbindEnum {
    SOLVELP_UNBOUNDED: SolveLPResult
    SOLVELP_UNFEASIBLE: SolveLPResult
    SOLVELP_SINGLE: SolveLPResult
    SOLVELP_MULTI: SolveLPResult
}

export type SolvePnPMethod = EmbindEnumEntity<SolvePnPMethodEnumValues>;
interface SolvePnPMethodEnumValues extends EmbindEnum {
    SOLVEPNP_ITERATIVE: SolvePnPMethod
    SOLVEPNP_EPNP: SolvePnPMethod
    SOLVEPNP_P3P: SolvePnPMethod
    SOLVEPNP_DLS: SolvePnPMethod
    SOLVEPNP_UPNP: SolvePnPMethod
    SOLVEPNP_AP3P: SolvePnPMethod
    SOLVEPNP_IPPE: SolvePnPMethod
    SOLVEPNP_IPPE_SQUARE: SolvePnPMethod
    SOLVEPNP_SQPNP: SolvePnPMethod
    SOLVEPNP_MAX_COUNT: SolvePnPMethod
}

export type SortFlags = EmbindEnumEntity<SortFlagsEnumValues>;
interface SortFlagsEnumValues extends EmbindEnum {
    SORT_EVERY_ROW: SortFlags
    SORT_EVERY_COLUMN: SortFlags
    SORT_ASCENDING: SortFlags
    SORT_DESCENDING: SortFlags
}

export type SpecialFilter = EmbindEnumEntity<SpecialFilterEnumValues>;
interface SpecialFilterEnumValues extends EmbindEnum {
    FILTER_SCHARR: SpecialFilter
}

export type TemplateMatchModes = EmbindEnumEntity<TemplateMatchModesEnumValues>;
interface TemplateMatchModesEnumValues extends EmbindEnum {
    TM_SQDIFF: SpecialFilter
    TM_SQDIFF_NORMED: SpecialFilter
    TM_CCORR: SpecialFilter
    TM_CCORR_NORMED: SpecialFilter
    TM_CCOEFF: SpecialFilter
    TM_CCOEFF_NORMED: SpecialFilter
}

export type TermCriteria_Type = EmbindEnumEntity<TermCriteria_TypeEnumValues>;
interface TermCriteria_TypeEnumValues extends EmbindEnum {
    COUNT: TermCriteria_Type
    MAX_ITER: TermCriteria_Type
    EPS: TermCriteria_Type
}

export type ThresholdTypes = EmbindEnumEntity<ThresholdTypesEnumValues>;
interface ThresholdTypesEnumValues extends EmbindEnum {
    THRESH_BINARY: ThresholdTypes
    THRESH_BINARY_INV: ThresholdTypes
    THRESH_TRUNC: ThresholdTypes
    THRESH_TOZERO: ThresholdTypes
    THRESH_TOZERO_INV: ThresholdTypes
    THRESH_MASK: ThresholdTypes
    THRESH_OTSU: ThresholdTypes
    THRESH_TRIANGLE: ThresholdTypes
}

export type UMatData_MemoryFlag = EmbindEnumEntity<UMatData_MemoryFlagEnumValues>;
interface UMatData_MemoryFlagEnumValues extends EmbindEnum {
    COPY_ON_MAP: UMatData_MemoryFlag
    HOST_COPY_OBSOLETE: UMatData_MemoryFlag
    DEVICE_COPY_OBSOLETE: UMatData_MemoryFlag
    TEMP_UMAT: UMatData_MemoryFlag
    TEMP_COPIED_UMAT: UMatData_MemoryFlag
    USER_ALLOCATED: UMatData_MemoryFlag
    DEVICE_MEM_MAPPED: UMatData_MemoryFlag
    ASYNC_CLEANUP: UMatData_MemoryFlag
}

export type UMatUsageFlags = EmbindEnumEntity<UMatUsageFlagsEnumValues>;
interface UMatUsageFlagsEnumValues extends EmbindEnum {
    USAGE_DEFAULT: UMatUsageFlags
    USAGE_ALLOCATE_HOST_MEMORY: UMatUsageFlags
    USAGE_ALLOCATE_DEVICE_MEMORY: UMatUsageFlags
    USAGE_ALLOCATE_SHARED_MEMORY: UMatUsageFlags
    __UMAT_USAGE_FLAGS_32BIT: UMatUsageFlags
}

export type UndistortTypes = EmbindEnumEntity<UndistortTypesEnumValues>;
interface UndistortTypesEnumValues extends EmbindEnum {
    PROJ_SPHERICAL_ORTHO: UndistortTypes
    PROJ_SPHERICAL_EQRECT: UndistortTypes
}

export type WarpPolarMode = EmbindEnumEntity<WarpPolarModeEnumValues>;
interface WarpPolarModeEnumValues extends EmbindEnum {
    WARP_POLAR_LINEAR: WarpPolarMode
    WARP_POLAR_LOG: WarpPolarMode
}

export type _InputArray_KindFlag = EmbindEnumEntity<_InputArray_KindFlagEnumValues>;
interface _InputArray_KindFlagEnumValues extends EmbindEnum {
    KIND_SHIFT: _InputArray_KindFlag
    FIXED_TYPE: _InputArray_KindFlag
    FIXED_SIZE: _InputArray_KindFlag
    KIND_MASK: _InputArray_KindFlag
    NONE: _InputArray_KindFlag
    MAT: _InputArray_KindFlag
    MATX: _InputArray_KindFlag
    STD_VECTOR: _InputArray_KindFlag
    STD_VECTOR_VECTOR: _InputArray_KindFlag
    STD_VECTOR_MAT: _InputArray_KindFlag
    EXPR: _InputArray_KindFlag
    OPENGL_BUFFER: _InputArray_KindFlag
    CUDA_HOST_MEM: _InputArray_KindFlag
    CUDA_GPU_MAT: _InputArray_KindFlag
    UMAT: _InputArray_KindFlag
    STD_VECTOR_UMAT: _InputArray_KindFlag
    STD_BOOL_VECTOR: _InputArray_KindFlag
    STD_VECTOR_CUDA_GPU_MAT: _InputArray_KindFlag
    STD_ARRAY: _InputArray_KindFlag
    STD_ARRAY_MAT: _InputArray_KindFlag
}

export type Error_Code = EmbindEnumEntity<Error_CodeEnumValues>;
interface Error_CodeEnumValues extends EmbindEnum {
    StsOk: Error_Code
    StsBackTrace: Error_Code
    StsError: Error_Code
    StsInternal: Error_Code
    StsNoMem: Error_Code
    StsBadArg: Error_Code
    StsBadFunc: Error_Code
    StsNoConv: Error_Code
    StsAutoTrace: Error_Code
    HeaderIsNull: Error_Code
    BadImageSize: Error_Code
    BadOffset: Error_Code
    BadDataPtr: Error_Code
    BadStep: Error_Code
    BadModelOrChSeq: Error_Code
    BadNumChannels: Error_Code
    BadNumChannel1U: Error_Code
    BadDepth: Error_Code
    BadAlphaChannel: Error_Code
    BadOrder: Error_Code
    BadOrigin: Error_Code
    BadAlign: Error_Code
    BadCallBack: Error_Code
    BadTileSize: Error_Code
    BadCOI: Error_Code
    BadROISize: Error_Code
    MaskIsTiled: Error_Code
    StsNullPtr: Error_Code
    StsVecLengthErr: Error_Code
    StsFilterStructContentErr: Error_Code
    StsKernelStructContentErr: Error_Code
    StsFilterOffsetErr: Error_Code
    StsBadSize: Error_Code
    StsDivByZero: Error_Code
    StsInplaceNotSupported: Error_Code
    StsObjectNotFound: Error_Code
    StsUnmatchedFormats: Error_Code
    StsBadFlag: Error_Code
    StsBadPoint: Error_Code
    StsBadMask: Error_Code
    StsUnmatchedSizes: Error_Code
    StsUnsupportedFormat: Error_Code
    StsOutOfRange: Error_Code
    StsParseError: Error_Code
    StsNotImplemented: Error_Code
    StsBadMemBlock: Error_Code
    StsAssert: Error_Code
    GpuNotSupported: Error_Code
    GpuApiCallError: Error_Code
    OpenGlNotSupported: Error_Code
    OpenGlApiCallError: Error_Code
    OpenCLApiCallError: Error_Code
    OpenCLDoubleNotSupported: Error_Code
    OpenCLInitError: Error_Code
    OpenCLNoAMDBlasFft: Error_Code
}

export type detail_CvFeatureParams_FeatureType = EmbindEnumEntity<detail_CvFeatureParams_FeatureTypeEnumValues>;
interface detail_CvFeatureParams_FeatureTypeEnumValues extends EmbindEnum {
    HAAR: detail_CvFeatureParams_FeatureType
    LBP: detail_CvFeatureParams_FeatureType
    HOG: detail_CvFeatureParams_FeatureType
}

export type detail_TestOp = EmbindEnumEntity<detail_TestOpEnumValues>;
interface detail_TestOpEnumValues extends EmbindEnum {
    TEST_CUSTOM: detail_TestOp
    TEST_EQ: detail_TestOp
    TEST_NE: detail_TestOp
    TEST_LE: detail_TestOp
    TEST_LT: detail_TestOp
    TEST_GE: detail_TestOp
    TEST_GT: detail_TestOp
}

export type detail_TrackerSamplerCSC_MODE = EmbindEnumEntity<detail_TrackerSamplerCSC_MODEEnumValues>;
interface detail_TrackerSamplerCSC_MODEEnumValues extends EmbindEnum {
    MODE_INIT_POS: detail_TrackerSamplerCSC_MODE
    MODE_INIT_NEG: detail_TrackerSamplerCSC_MODE
    MODE_TRACK_POS: detail_TrackerSamplerCSC_MODE
    MODE_TRACK_NEG: detail_TrackerSamplerCSC_MODE
    MODE_DETECT: detail_TrackerSamplerCSC_MODE
}

export type dnn_Backend = EmbindEnumEntity<dnn_BackendEnumValues>;
interface dnn_BackendEnumValues extends EmbindEnum {
    DNN_BACKEND_DEFAULT: dnn_Backend
    DNN_BACKEND_HALIDE: dnn_Backend
    DNN_BACKEND_INFERENCE_ENGINE: dnn_Backend
    DNN_BACKEND_OPENCV: dnn_Backend
    DNN_BACKEND_VKCOM: dnn_Backend
    DNN_BACKEND_CUDA: dnn_Backend
}

export type dnn_Target = EmbindEnumEntity<dnn_TargetEnumValues>;
interface dnn_TargetEnumValues extends EmbindEnum {
    DNN_TARGET_CPU: dnn_Target
    DNN_TARGET_OPENCL: dnn_Target
    DNN_TARGET_OPENCL_FP16: dnn_Target
    DNN_TARGET_MYRIAD: dnn_Target
    DNN_TARGET_VULKAN: dnn_Target
    DNN_TARGET_FPGA: dnn_Target
    DNN_TARGET_CUDA: dnn_Target
    DNN_TARGET_CUDA_FP16: dnn_Target
    DNN_TARGET_HDDL: dnn_Target
}


export interface AKAZE extends Feature2D {
    getDefaultName(): string
    getDescriptorChannels(): int
    getDescriptorSize(): int
    getDescriptorType(): AKAZE_DescriptorType
    getDiffusivity(): KAZE_DiffusivityType
    getNOctaveLayers(): int
    getNOctaves(): int
    getThreshold(): double
    setDescriptorChannels(dch: int): void
    setDescriptorSize(dsize: int): void
    setDescriptorType(dtype: AKAZE_DescriptorType): void
    setDiffusivity(diff: KAZE_DiffusivityType): void
    setNOctaveLayers(octaveLayers: int): void
    setNOctaves(octaves: int): void
    setThreshold(threshold: double): void
}
export interface AgastFeatureDetector extends Feature2D {
    getDefaultName(): string
    getNonmaxSuppression(): boolean
    getThreshold(): int
    getType(): AgastFeatureDetector_DetectorType
    setNonmaxSuppression(f: boolean): void
    setThreshold(threshold: int): void
    setType(type: AgastFeatureDetector_DetectorType): void
}

export interface Algorithm extends EmClassHandle {
}

export interface AlignMTB extends EmClassHandle {
    calculateShift(img0: Mat, img1: Mat): PointLike
    computeBitmaps(img: Mat, tb: Mat, eb: Mat): void
    getCut(): boolean
    getExcludeRange(): int
    getMaxBits(): int
    setCut(value: boolean): void
    setExcludeRange(exclude_range: int): void
    setMaxBits(max_bits: int): void
    shiftMat(src: Mat, dst: Mat, shift: PointLike): void
}

export interface BFMatcher extends DescriptorMatcher {
}

export interface BRISK extends Feature2D {
    getDefaultName(): string
}

export interface BackgroundSubtractor extends EmClassHandle {
    apply(image: Mat, fgmask: Mat, learningRate?: double): void
    getBackgroundImage(backgroundImage: Mat): void
}

export interface BackgroundSubtractorMOG2 extends EmClassHandle {
    apply(image: Mat, fgmask: Mat, learningRate?: double): void
}

export interface CLAHE extends EmClassHandle {
    apply(src: Mat, dst: Mat): void
    collectGarbage(): void
    getClipLimit(): double
    getTilesGridSize(): SizeLike
    setClipLimit(clipLimit: double): void
    setTilesGridSize(tileGridSize: SizeLike): void
}

export interface CalibrateCRF extends EmClassHandle {
    process(src: MatVector, dst: Mat, times: Mat): void
}

export interface CalibrateDebevec extends EmClassHandle {
    getLambda(): float
    getRandom(): boolean
    getSamples(): int
    setLambda(lambda: float): void
    setRandom(random: boolean): void
    setSamples(samples: int): void
}

export interface CalibrateRobertson extends EmClassHandle {
    getMaxIter(): int
    getRadiance(): Mat
    getThreshold(): float
    setMaxIter(max_iter: int): void
    setThreshold(threshold: float): void
}

export interface CascadeClassifier extends EmClassHandle {
    detectMultiScale(image: Mat, objects: RectVector, scaleFactor?: double,
                     minNeighbors?: int, flags?: int, minSize?: SizeLike, maxSize?: SizeLike): void
    detectMultiScale2(image: Mat, objects: RectVector, numDetections: IntVector|int[],
                      scaleFactor?: double, minNeighbors?: int, flags?: int,
                      minSize?: SizeLike, maxSize?: SizeLike): void
    detectMultiScale3(image: Mat, objects: RectVector, rejectLevels: IntVector|int[],
                      levelWeights: DoubleVector|double[], scaleFactor?: double,
                      minNeighbors?: int, flags?: int, minSize?: SizeLike, maxSize?: SizeLike,
                      outputRejectLevels?: boolean): void
    empty(): boolean
    load(filename: string): boolean
}

export interface DescriptorMatcher extends EmClassHandle {
    add(descriptors: MatVector): void
    clear(): void
    empty(): boolean
    isMaskSupported(): boolean
    knnMatch(queryDescriptors: Mat, trainDescriptors: Mat, matches: DMatchVectorVector,
             k: int, mask?: Mat, compactResult?: boolean): void
    knnMatch(queryDescriptors: Mat, matches: DMatchVectorVector, k: int,
             masks?: MatVector, compactResult?: boolean): void
    match(queryDescriptors: Mat, trainDescriptors: Mat, matches: DMatchVector, mask?: Mat): void
    match(queryDescriptors: Mat, matches: DMatchVector, masks?: MatVector): void
    radiusMatch(queryDescriptors: Mat, trainDescriptors: Mat, matches: DMatchVectorVector,
                maxDistance: float, mask?: Mat, compactResult?: boolean): void
    radiusMatch(queryDescriptors: Mat, matches: DMatchVectorVector, maxDistance: float,
                masks?: MatVector, compactResult?: boolean): void
    train(): void
}

export interface FastFeatureDetector extends Feature2D {
    getDefaultName(): string
    getNonmaxSuppression(): boolean
    getThreshold(): int
    getType(): FastFeatureDetector_DetectorType
    setNonmaxSuppression(f: boolean): void
    setThreshold(threshold: int): void
    setType(type: FastFeatureDetector_DetectorType): void
}

export interface Feature2D extends EmClassHandle {
    compute(image: Mat, keypoints: KeyPointVector, descriptors: Mat): void
    compute(images: MatVector, keypoints: unknown, descriptors: MatVector): void
    defaultNorm(): int
    descriptorSize(): int
    descriptorType(): int
    detect(image: Mat, keypoints: KeyPointVector, mask?: Mat): void
    detect(images: MatVector, keypoints: unknown, masks?: MatVector): void
    detectAndCompute(image: Mat, mask: Mat, keypoints: KeyPointVector,
                     descriptors: Mat, useProvidedKeypoints?: boolean): void
    empty(): boolean
    getDefaultName(): string
}

export interface GFTTDetector extends Feature2D {
    getBlockSize(): int
    getDefaultName(): string
    getHarrisDetector(): boolean
    getK(): double
    getMaxFeatures(): int
    getMinDistance(): double
    getQualityLevel(): double
    setBlockSize(blockSize: int): void
    setHarrisDetector(val: boolean): void
    setK(k: double): void
    setMaxFeatures(maxFeatures: int): void
    setMinDistance(minDistance: double): void
    setQualityLevel(qlevel: double): void
}

export interface HOGDescriptor extends EmClassHandle {
    winSize: SizeLike
    blockSize: SizeLike
    blockStride: SizeLike
    cellSize: SizeLike
    nbins: int
    derivAperture: int
    winSigma: double
    histogramNormType: HOGDescriptor_HistogramNormType
    L2HysThreshold: double
    gammaCorrection: boolean
    svmDetector: FloatVector|float[]
    nlevels: int
    signedGradient: boolean

    detectMultiScale(img: Mat, foundLocations: RectVector, foundWeights: DoubleVector|double[],
                     hitThreshold?: double, winStride?: SizeLike, padding?: SizeLike,
                     scale?: double, finalThreshold?: double, useMeanshiftGrouping?: boolean): void

    getDaimlerPeopleDetector(): FloatVector|float[]
    getDefaultPeopleDetector(): FloatVector|float[]
    load(filename: string, objname?: string): boolean
    setSVMDetector(svmdetector: Mat): void
}

export interface KAZE extends Feature2D {
    getDefaultName(): string
    getDiffusivity(): KAZE_DiffusivityType
    getExtended(): boolean
    getNOctaveLayers(): int
    getNOctaves(): int
    getThreshold(): double
    getUpright(): boolean
    setDiffusivity(diff: KAZE_DiffusivityType): void
    setExtended(extended: boolean): void
    setNOctaveLayers(octaveLayers: int): void
    setNOctaves(octaves: int): void
    setThreshold(threshold: double): void
    setUpright(upright: boolean): void
}

export interface MSER extends Feature2D {
    detectRegions(image: Mat, msers: unknown, bboxes: RectVector): void
    getDefaultName(): string
    getDelta(): int
    getMaxArea(): int
    getMinArea(): int
    getPass2Only(): boolean
    setDelta(delta: int): void
    setMaxArea(maxArea: int): void
    setMinArea(minArea: int): void
    setPass2Only(f: boolean): void
}

export interface MergeDebevec extends EmClassHandle {
    process(src: MatVector, dst: Mat, times: Mat, response: Mat): void
    process(src: MatVector, dst: Mat, times: Mat): void
}

export interface MergeExposures extends EmClassHandle {
    process(src: MatVector, dst: Mat, times: Mat, response: Mat): void
}

export interface MergeMertens extends EmClassHandle {
    getContrastWeight(): float
    getExposureWeight(): float
    getSaturationWeight(): float
    process(src: MatVector, dst: Mat, times: Mat, response: Mat): void
    process(src: MatVector, dst: Mat): void
    setContrastWeight(contrast_weiht: float): void
    setExposureWeight(exposure_weight: float): void
    setSaturationWeight(saturation_weight: float): void
}

export interface MergeRobertson extends EmClassHandle {
    process(src: MatVector, dst: Mat, times: Mat, response: Mat): void
    process(src: MatVector, dst: Mat, times: Mat): void
}

export interface ORB extends Feature2D {
    getDefaultName(): string
    getFastThreshold(): int
    setEdgeThreshold(edgeThreshold: int): void
    setFirstLevel(firstLevel: int): void
    setMaxFeatures(maxFeatures: int): void
    setNLevels(nlevels: int): void
    setPatchSize(patchSize: int): void
    setScaleFactor(scaleFactor: double): void
    setScoreType(scoreType: ORB_ScoreType): void
    setWTA_K(wta_k: int): void
}

export interface QRCodeDetector extends EmClassHandle {
    // TODO(sora): complete this
}

export interface Tonemap extends EmClassHandle {
    getGamma(): float
    process(src: Mat, dst: Mat): void
    setGamma(gamma: float): void
}

export interface TonemapDrago extends EmClassHandle {
    getBias(): float
    getSaturation(): float
    setBias(bias: float): void
    setSaturation(saturation: float): void
}

export interface TonemapMantiuk extends EmClassHandle {
    getSaturation(): float
    getScale(): float
    setSaturation(saturation: float): void
    setScale(scale: float): void
}

export interface TonemapReinhard extends EmClassHandle {
    getColorAdaptation(): float
    getIntensity(): float
    getLightAdaptation(): float
    setColorAdaptation(color_adapt: float): void
    setIntensity(intensity: float): void
    setLightAdaptation(light_adapt: float): void
}

export interface dnn_Net extends EmClassHandle {
    forward(outputName?: string): Mat
    forward(outputBlobs: MatVector, outputName?: string): void
    forward(outputBlobs: MatVector, outBlobNames: unknown): void
    setInput(blob: Mat, name?: string, scalefactor?: double, mean?: ScalarLike): void
}

export interface IntelligentScissorsMB extends EmClassHandle {
    applyImage(image: Mat): IntelligentScissorsMB
    applyImageFeatures(non_edge: Mat, gradient_direction: Mat,
                       gradient_magnitude: Mat, image?: Mat): IntelligentScissorsMB
    buildMap(sourcePt: PointLike): void
    getContour(targetPt: PointLike, contour: Mat, backward?: boolean): void
    setEdgeFeatureCannyParameters(threshold1: double, threshold2: double,
                                  apertureSize?: int, L2gradient?: boolean): IntelligentScissorsMB
    setEdgeFeatureZeroCrossingParameters(gradient_magnitude_min_value?: float): IntelligentScissorsMB
    setGradientMagnitudeMaxLimit(gradient_magnitude_threshold_max?: float): IntelligentScissorsMB
    setWeights(weight_non_edge: float, weight_gradient_direction: float,
               weight_gradient_magnitude: float): IntelligentScissorsMB
}
