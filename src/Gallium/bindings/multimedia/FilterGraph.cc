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

#include "Core/Exception.h"
#include "Gallium/bindings/multimedia/ffwrappers/libavfilter.h"
#include "Gallium/bindings/multimedia/Frame.h"
#include "Gallium/bindings/multimedia/AChannelLayout.h"
#include "Gallium/bindings/multimedia/HWFramesContext.h"
#include "Gallium/bindings/multimedia/FilterGraph.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

struct PipelineInOutPad
{
    ~PipelineInOutPad() {
        av_channel_layout_uninit(&ch_layout);
        if (hw_frames_ctx)
            av_buffer_unref(&hw_frames_ctx);

        // `filter_ctx` is managed by the `AVFilterGraph` object, and we should
        // not free it manually here.
    }

    std::string             pad_id;
    bool                    is_linked = false;
    AVMediaType             media_type = AVMEDIA_TYPE_UNKNOWN;

    // For input pad, points to an `abuffer` (audio) or `buffer` (video) filter;
    // for output pad, points to an `abuffersink` (audio) or `buffersink` filter.
    AVFilterContext        *filter_ctx = nullptr;

    // For input pads:
    AVRational              time_base = AV_TIME_BASE_Q;

    // For audio input pads:
    AVSampleFormat          sample_fmt = AV_SAMPLE_FMT_NONE;
    AVChannelLayout         ch_layout{};
    int32_t                 sample_rate = 0;

    // For video input pads:
    AVPixelFormat           pixel_fmt = AV_PIX_FMT_NONE;
    int32_t                 width = 0;
    int32_t                 height = 0;
    AVRational              sar{1, 1};
    AVColorSpace            color_space = AVCOL_SPC_UNSPECIFIED;
    AVColorRange            color_range = AVCOL_RANGE_UNSPECIFIED;
    AVBufferRef            *hw_frames_ctx = nullptr;
};

struct PipelineContext
{
    ~PipelineContext() {
        if (graph)
            avfilter_graph_free(&graph);
    }

    using NamePadMap = std::unordered_map<std::string, PipelineInOutPad>;

    std::string             graph_dsl;
    AVFilterGraph          *graph = nullptr;
    NamePadMap              in_pads;
    NamePadMap              out_pads;
};

FilterGraphBuilder::FilterGraphBuilder()
    : ctx_(std::make_unique<PipelineContext>())
{
    ctx_->graph = avfilter_graph_alloc();
    CHECK(ctx_->graph && "allocation failed");
}

ffi::RetLocal<v8::Value> FilterGraphBuilder::setThreads(int32_t num)
{
    ctx_->graph->nb_threads = num;
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value> FilterGraphBuilder::setGraph(const std::string& dsl)
{
    ctx_->graph_dsl = dsl;
    return GetThisHandle(v8::Isolate::GetCurrent());
}

#define CHECK_PAD_REDEFINE(in_or_out) \
    if (ctx_->in_or_out##_pads.contains(pad_id)) { \
        return ffi::Fail(ffi::kErr, fmt::format("pad '{}' has been added", pad_id)); \
    }


ffi::RetLocal<v8::Value>
FilterGraphBuilder::addAudioSrc(const std::string& pad_id,
                                ffi::Enum<SampleFormat> format,
                                const ffi::Opt<AVRationalAdapter>& timebase,
                                int32_t sample_rate,
                                const ffi::Class<AChannelLayout>& ch_layout)
{
    CHECK_PAD_REDEFINE(in)
    PipelineInOutPad& pad = ctx_->in_pads[pad_id];
    pad.pad_id = pad_id;
    pad.media_type = AVMEDIA_TYPE_AUDIO;
    pad.sample_fmt = static_cast<AVSampleFormat>(*format);
    if (timebase)
        pad.time_base = **timebase;
    pad.sample_rate = sample_rate;
    ch_layout->CopyLayoutTo(pad.ch_layout);

    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value>
FilterGraphBuilder::addVideoSrc(const std::string& pad_id,
                                ffi::Enum<PixelFormat> format,
                                const ffi::Opt<AVRationalAdapter>& timebase,
                                int32_t width,
                                int32_t height,
                                const AVRationalAdapter& sar,
                                ffi::Enum<ColorSpace> color_space,
                                ffi::Enum<ColorRange> color_range,
                                const ffi::Opt<ffi::Class<HWFramesContext>>& hwctx)
{
    CHECK_PAD_REDEFINE(in)

    if (hwctx && *format != PixelFormat::kNone)
        return ffi::Fail(ffi::kErr, "pixel format must be `kNone` when `hwctx` passed");

    PipelineInOutPad& pad = ctx_->in_pads[pad_id];
    pad.pad_id = pad_id;
    pad.media_type = AVMEDIA_TYPE_VIDEO;
    pad.pixel_fmt = static_cast<AVPixelFormat>(*format);
    if (timebase)
        pad.time_base = **timebase;
    pad.width = width;
    pad.height = height;
    pad.sar = *sar;
    pad.color_space = static_cast<AVColorSpace>(*color_space);
    pad.color_range = static_cast<AVColorRange>(*color_range);
    if (hwctx)
        pad.hw_frames_ctx = av_buffer_ref((*hwctx)->GetAVBufferRef());

    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value> FilterGraphBuilder::addAudioSink(const std::string& pad_id)
{
    CHECK_PAD_REDEFINE(out)
    PipelineInOutPad& pad = ctx_->out_pads[pad_id];
    pad.pad_id = pad_id;
    pad.media_type = AVMEDIA_TYPE_AUDIO;
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value> FilterGraphBuilder::addVideoSink(const std::string& pad_id)
{
    CHECK_PAD_REDEFINE(out)
    PipelineInOutPad& pad = ctx_->out_pads[pad_id];
    pad.pad_id = pad_id;
    pad.media_type = AVMEDIA_TYPE_VIDEO;
    return GetThisHandle(v8::Isolate::GetCurrent());
}

#define FAIL_ERR_FMT(fmtstr, ...) \
    ffi::Fail(ffi::kErr, fmt::format(fmtstr __VA_OPT__(,) __VA_ARGS__))

ffi::RetLocal<v8::Value> FilterGraphBuilder::build()
{
    NotifyDisposeState(DisposeState::kDisposed);
    ScopeExitAutoInvoker reset_internal_sp([this] {
        // Release all the resources
        ctx_.reset();
    });

    // `inputs` and `outputs` are linked lists containing all the unlinked
    // filters in the graph.
    AVFilterInOut *inputs = nullptr, *outputs = nullptr;
    int res = avfilter_graph_parse2(ctx_->graph, ctx_->graph_dsl.c_str(), &inputs, &outputs);
    ScopeExitAutoInvoker inout_free([&inputs, &outputs] {
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
    });
    if (res < 0)
        return FAIL_ERR_FMT("failed to parse the graph DSL: {}", av_err2str(res));

    // Match, configure, and link input pads. If `inputs` is nullptr, the filter
    // does not require any inputs.
    if (inputs)
    {
        const AVFilter *abuffersrc = avfilter_get_by_name("abuffer");
        const AVFilter *vbuffersrc = avfilter_get_by_name("buffer");
        CHECK(abuffersrc && vbuffersrc && "required filters not compiled in");

        for (AVFilterInOut *cur = inputs; cur; cur = cur->next)
        {
            if (!cur->name)
            {
                return FAIL_ERR_FMT("specify a name for the {}th input pad of filter '{}'",
                                    cur->pad_idx + 1, cur->filter_ctx->filter->name);
            }
            std::string cur_name = cur->name;

            if (!ctx_->in_pads.contains(cur_name))
                return FAIL_ERR_FMT("missing an input pad named '{}'", cur_name);
            PipelineInOutPad& pad = ctx_->in_pads[cur_name];

            if (avfilter_pad_get_type(cur->filter_ctx->input_pads, cur->pad_idx) != pad.media_type)
                return FAIL_ERR_FMT("the media type of input pad '{}' does not match", cur_name);

            AVBufferSrcParameters *params = av_buffersrc_parameters_alloc();
            CHECK(params && "allocation failed");

            const AVFilter *filter;

            params->time_base = pad.time_base;
            if (pad.media_type == AVMEDIA_TYPE_AUDIO)
            {
                filter = abuffersrc;
                params->format = static_cast<int>(pad.sample_fmt);
                params->sample_rate = pad.sample_rate;
                av_channel_layout_copy(&params->ch_layout, &pad.ch_layout);
            }
            else
            {
                filter = vbuffersrc;
                params->format = static_cast<int>(pad.pixel_fmt);
                params->width = pad.width;
                params->height = pad.height;
                params->sample_aspect_ratio = pad.sar;
                params->color_space = pad.color_space;
                params->color_range = pad.color_range;
                if (pad.hw_frames_ctx)
                {
                    // buffersrc will take internal references when necessary, so
                    // there is no need to use `av_buffer_ref()`
                    params->hw_frames_ctx = pad.hw_frames_ctx;
                    params->format = reinterpret_cast<AVHWFramesContext*>(
                            pad.hw_frames_ctx->data)->format;
                }
            }

            pad.filter_ctx = avfilter_graph_alloc_filter(ctx_->graph, filter, cur->name);
            CHECK(pad.filter_ctx && "allocation failed");

            CHECK(av_buffersrc_parameters_set(pad.filter_ctx, params) >= 0);
            av_channel_layout_uninit(&params->ch_layout);
            av_free(params);

            res = avfilter_init_str(pad.filter_ctx, nullptr);
            if (res < 0)
            {
                return FAIL_ERR_FMT("could not initialize buffersrc for input pad '{}': {}",
                                    cur_name, av_err2str(res));
            }

            res = avfilter_link(pad.filter_ctx, 0, cur->filter_ctx, cur->pad_idx);
            if (res < 0)
            {
                return FAIL_ERR_FMT("could not link input pad '{}': {}",
                                    cur_name, av_err2str(res));
            }

            pad.is_linked = true;
        }
    }
    // Detect unlinked input pads. They were added by the user, but not used
    // in the graph.
    for (const auto& pad_name_pair : ctx_->in_pads)
    {
        if (!pad_name_pair.second.is_linked)
            return FAIL_ERR_FMT("input pad '{}' is not linked into the graph", pad_name_pair.first);
    }

    // Match, configure, and link output pads. If `output` is nullptr, the filter
    // does not require any outputs.
    if (outputs)
    {
        const AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
        const AVFilter *vbuffersink = avfilter_get_by_name("buffersink");
        CHECK(abuffersink && vbuffersink && "required filters not compiled in");

        for (AVFilterInOut *cur = outputs; cur; cur = cur->next)
        {
            if (!cur->name)
            {
                return FAIL_ERR_FMT("specify a name for the {}th output pad of filter '{}'",
                                    cur->pad_idx + 1, cur->filter_ctx->filter->name);
            }
            std::string cur_name = cur->name;

            if (!ctx_->out_pads.contains(cur_name))
                return FAIL_ERR_FMT("missing an output pad named '{}'", cur_name);
            PipelineInOutPad& pad = ctx_->out_pads[cur_name];

            if (avfilter_pad_get_type(cur->filter_ctx->output_pads, cur->pad_idx) != pad.media_type)
                return FAIL_ERR_FMT("the media type of output pad '{}' does not match", cur_name);

            res = avfilter_graph_create_filter(
                &pad.filter_ctx,
                pad.media_type == AVMEDIA_TYPE_AUDIO ? abuffersink : vbuffersink,
                cur_name.c_str(),
                nullptr,
                nullptr,
                ctx_->graph
            );
            if (res < 0)
                return FAIL_ERR_FMT("failed to create buffersink for output pad '{}'", cur_name);

            res = avfilter_link(cur->filter_ctx, cur->pad_idx, pad.filter_ctx, 0);
            if (res < 0)
            {
                return FAIL_ERR_FMT("could not link output pad '{}': {}",
                                    cur_name, av_err2str(res));
            }

            pad.is_linked = true;
        }
    }
    // Detect unlinked output pads. They were added by the user, but not used
    // in the graph.
    for (const auto& pad_name_pair : ctx_->out_pads)
    {
        if (!pad_name_pair.second.is_linked)
            return FAIL_ERR_FMT("output pad '{}' is not linked into the graph", pad_name_pair.first);
    }

    res = avfilter_graph_config(ctx_->graph, nullptr);
    if (res < 0)
        return FAIL_ERR_FMT("failed to configure the graph: {}", av_err2str(res));

    return ffi::JSObject::New<FilterGraph>(v8::Isolate::GetCurrent(), std::move(ctx_));
}

ffi::Ret<void> FilterGraph::dispose()
{
    ctx_.reset();
    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

ffi::RetLocal<v8::Value> FilterGraph::getSinkProperties(const std::string& pad_id)
{
    if (!ctx_->out_pads.contains(pad_id))
        return FAIL_ERR_FMT("output pad '{}' not found", pad_id);
    PipelineInOutPad& pad = ctx_->out_pads[pad_id];
    AVFilterContext *fctx = pad.filter_ctx;

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto create_null_or_rational = [isolate](const AVRational& r) -> v8::Local<v8::Value> {
        if (r.den == 0)
            return v8::Null(isolate);
        return CreateJSRational(isolate, r);
    };

    ffi::ObjectLiteralMap result;
    result["type"] = v8::Int32::New(isolate, static_cast<int>(pad.media_type));
    result["timeBase"] = create_null_or_rational(av_buffersink_get_time_base(fctx));

    if (pad.media_type == AVMEDIA_TYPE_AUDIO)
    {
        result["sampleFormat"] = v8::Int32::New(isolate, av_buffersink_get_format(fctx));
        AVChannelLayout ch_layout{};
        CHECK(av_buffersink_get_ch_layout(fctx, &ch_layout) >= 0);
        // The constructor uninits the `ch_layout` structure
        result["channelLayout"] = ffi::JSObject::New<AChannelLayout>(isolate, ch_layout);
        result["sampleRate"] = v8::Int32::New(isolate, av_buffersink_get_sample_rate(fctx));
    }
    else
    {
        result["pixelFormat"] = v8::Int32::New(isolate, static_cast<int>(pad.media_type));
        result["frameRate"] = create_null_or_rational(av_buffersink_get_frame_rate(fctx));
        result["width"] = v8::Int32::New(isolate, av_buffersink_get_w(fctx));
        result["height"] = v8::Int32::New(isolate, av_buffersink_get_h(fctx));
        result["SAR"] = create_null_or_rational(av_buffersink_get_sample_aspect_ratio(fctx));
        result["colorSpace"] = v8::Int32::New(isolate, av_buffersink_get_colorspace(fctx));
        result["colorRange"] = v8::Int32::New(isolate, av_buffersink_get_color_range(fctx));
        if (AVBufferRef *hw_frames_ctx = av_buffersink_get_hw_frames_ctx(fctx))
        {
            result["hwFramesCtx"] = ffi::JSObject::New<HWFramesContext>(
                    isolate, av_buffer_ref(hw_frames_ctx));
        }
        else
        {
            result["hwFramesCtx"] = v8::Null(isolate);
        }
    }

    return ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, result);
}

ffi::Ret<void> FilterGraph::sendFrame(const std::string& pad_id,
                                      const ffi::Opt<ffi::Class<Frame>>& frame)
{
    if (!ctx_->in_pads.contains(pad_id))
        return FAIL_ERR_FMT("input pad '{}' not found", pad_id);
    PipelineInOutPad& pad = ctx_->in_pads[pad_id];

    AVFrame *avframe = nullptr;
    if (frame)
    {
        if ((*frame)->GetDisposeState() != DisposeState::kNot)
            return FAIL_ERR_FMT("provided `Frame` instance has been disposed");
        avframe = (*frame)->GetAVFrame();
    }

    int res = av_buffersrc_add_frame_flags(pad.filter_ctx, avframe, AV_BUFFERSRC_FLAG_KEEP_REF);
    if (res < 0)
        return FAIL_ERR_FMT("failed to push frame into filter: {}", av_err2str(res));
    return {};
}

ffi::RetLocal<v8::Value> FilterGraph::receiveFrame(const std::string& pad_id,
                                                   const ffi::Opt<ffi::Class<Frame>>& reuse)
{
    if (!ctx_->out_pads.contains(pad_id))
        return FAIL_ERR_FMT("output pad '{}' not found", pad_id);
    PipelineInOutPad& pad = ctx_->out_pads[pad_id];

    AVFrame *avframe;
    if (reuse)
    {
        if (!(*reuse)->IsReusable())
            return FAIL_ERR_FMT("provided `Frame` instance is not reusable");
        avframe = (*reuse)->GetAVFrame();
    }
    else
    {
        avframe = av_frame_alloc();
        CHECK(avframe && "allocation failed");
    }

    int res = av_buffersink_get_frame(pad.filter_ctx, avframe);
    if (res < 0 && !reuse)
        av_frame_free(&avframe);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    using Tuple = std::tuple<int, v8::Local<v8::Value>>;
    Tuple result_tuple;
    if (res == AVERROR(EAGAIN))
    {
        result_tuple = {
            static_cast<int>(FilterGraphReceiveStatus::kNeedInput),
            v8::Null(isolate)
        };
    }
    else if (res == AVERROR_EOF)
    {
        result_tuple = {
            static_cast<int>(FilterGraphReceiveStatus::kEOF),
            v8::Null(isolate)
        };
    }
    else if (res < 0)
    {
        return FAIL_ERR_FMT("failed to receive frame from pad '{}': {}", pad_id, av_err2str(res));
    }
    else
    {
        if (reuse)
            (*reuse)->ResetDisposeState();
        result_tuple = {
            static_cast<int>(FilterGraphReceiveStatus::kSuccess),
            reuse ? (*reuse)->GetThisHandle(isolate)
                  : ffi::JSObject::New<Frame>(isolate, avframe)
        };
    }

    return ffi::Cast<Tuple>::ToChecked(isolate, result_tuple);
}

ffi::Ret<void> FilterGraph::sendCommand(const std::string& target, const std::string& cmd,
                                        const std::string& args, bool propagate)
{
    int res = avfilter_graph_send_command(
        ctx_->graph,
        target.c_str(),
        cmd.c_str(),
        args.c_str(),
        nullptr,
        0,
        propagate ? 0 : AVFILTER_CMD_FLAG_ONE
    );
    if (res == AVERROR(ENOSYS))
        return FAIL_ERR_FMT("unsupported command '{}' for filter '{}'", cmd, target);
    if (res < 0)
        return FAIL_ERR_FMT("failed to send command: {}", av_err2str(res));
    return {};
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
