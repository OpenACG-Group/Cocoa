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

#include "fmt/format.h"

#include "Utau/ffwrappers/libavutil.h"
#include "Utau/ffwrappers/libavfilter.h"

#include "Core/Exception.h"
#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Utau/AVFilterDAG.h"
#include "Utau/AudioBuffer.h"
#include "Utau/VideoBuffer.h"
#include "Utau/HWDeviceContext.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau.AVFilterDAG)

struct NamedInOutFilterCtx
{
    std::string         label_name;
    MediaType           media_type;

    // For input filters, `abuffer` or `buffer` filter is associated;
    // for output filters, `abuffersink` or `buffersink` filter is associated.
    AVFilterContext    *context;

    // For video inputs only
    bool                enable_hw_frame;
    AVRational          time_base;
    AVRational          sar;
};

struct AVFilterDAG::FilterDAGPriv
{
    ~FilterDAGPriv() {
        // TODO(sora): should we release `in_filters` and `out_filters`?
        avfilter_graph_free(&graph);
    }

    AVFilterGraph                      *graph = nullptr;
    std::vector<NamedInOutFilterCtx>    in_filters;
    std::vector<NamedInOutFilterCtx>    out_filters;
};

namespace {

using DAG = AVFilterDAG;

bool configure_input_buffers(DAG::FilterDAGPriv *priv, AVFilterInOut *in,
                             const std::vector<DAG::InBufferParameters>& inparams)
{
    CHECK(priv && "Invalid data pointer");

    if (!in)
    {
        QLOG(LOG_ERROR, "Failed to configure input buffers: no input buffers are required in DAG");
        return false;
    }

    const AVFilter *af = avfilter_get_by_name("abuffer");
    if (!af)
        return false;

    const AVFilter *vf = avfilter_get_by_name("buffer");
    if (!vf)
        return false;

    for (AVFilterInOut *cur = in; cur; cur = cur->next)
    {
        if (!cur->name)
            cur->name = av_strdup("in");

        auto params = std::find_if(inparams.begin(), inparams.end(),
                                  [cur](const DAG::InBufferParameters& entry) {
            return (entry.name == cur->name);
        });
        if (params == inparams.end())
        {
            QLOG(LOG_ERROR, "Missing input buffer: '{}'", cur->name);
            return false;
        }

        const AVFilter *filter;
        AVFilterContext *filter_context;
        std::string args;

        if (params->media_type == MediaType::kAudio)
        {
            args = fmt::format("sample_fmt={}:sample_rate={}:channel_layout={}",
                               static_cast<int32_t>(SampleFormatToLibavFormat(params->sample_fmt)),
                               params->sample_rate,
                               params->channel_mode == AudioChannelMode::kStereo ? "stereo" : "mono");
            filter = af;
        }
        else if (params->media_type == MediaType::kVideo)
        {
            args = fmt::format("width={}:height={}:pix_fmt={}:time_base={}/{}:sar={}/{}",
                               params->width,
                               params->height,
                               static_cast<int32_t>(params->pixel_fmt),
                               params->time_base.num, params->time_base.denom,
                               params->sar.num, params->sar.denom);
            filter = vf;
        }
        else
        {
            MARK_UNREACHABLE();
        }

        int ret = avfilter_graph_create_filter(
                &filter_context, filter, params->name.c_str(), args.c_str(), nullptr, priv->graph);
        if (ret < 0)
        {
            QLOG(LOG_ERROR, "Failed to create input buffer '{}'", params->name);
            return false;
        }

        if (params->hw_frame_ctx)
        {
            auto hw_context = GlobalContext::Ref().GetHWDeviceContext();
            if (!hw_context)
            {
                QLOG(LOG_ERROR, "Failed to get hardware device context for HW frame input '{}'",
                     params->name);
                return false;
            }

            AVBufferSrcParameters *src_par = av_buffersrc_parameters_alloc();
            CHECK(src_par && "Failed to allocate memory");

            // `params->pixel_fmt` is ignored if hwaccel is enabled
            src_par->format = hw_context->GetDeviceFormat();
            src_par->time_base = av_make_q(params->time_base.num, params->time_base.denom);
            src_par->width = params->width;
            src_par->height = params->height;
            src_par->sample_aspect_ratio = av_make_q(params->sar.num, params->sar.denom);
            src_par->hw_frames_ctx = params->hw_frame_ctx;

            av_buffersrc_parameters_set(filter_context, src_par);
            av_free(src_par);
        }

        ret = avfilter_link(filter_context, 0, cur->filter_ctx, cur->pad_idx);
        if (ret < 0)
        {
            QLOG(LOG_ERROR, "Failed to link input buffer with the destination node");
            return false;
        }

        priv->in_filters.emplace_back(NamedInOutFilterCtx{
            .label_name = params->name,
            .media_type = params->media_type,
            .context = filter_context,
            .enable_hw_frame = static_cast<bool>(params->hw_frame_ctx),
            .time_base = av_make_q(params->time_base.num, params->time_base.denom),
            .sar = av_make_q(params->sar.num, params->sar.denom)
        });
    }

    // TODO(sora): Check if there are unused parameters.

    return true;
}

bool configure_output_buffers(DAG::FilterDAGPriv *priv, AVFilterInOut *out,
                              const std::vector<DAG::OutBufferParameters>& outparams)
{
    CHECK(priv && "Invalid data pointer");

    if (!out)
    {
        QLOG(LOG_ERROR, "Failed to configure output buffers: no output buffers are required in DAG");
        return false;
    }

    const AVFilter *af = avfilter_get_by_name("abuffersink");
    if (!af)
        return false;

    const AVFilter *vf = avfilter_get_by_name("buffersink");
    if (!vf)
        return false;

    for (AVFilterInOut *cur = out; cur; cur = cur->next)
    {
        if (!cur->name)
            cur->name = av_strdup("out");

        auto params = std::find_if(outparams.begin(), outparams.end(),
                                   [cur](const DAG::OutBufferParameters& entry) {
            return (entry.name == cur->name);
        });
        if (params == outparams.end())
        {
            QLOG(LOG_ERROR, "Missing output buffersink: '{}'", cur->name);
            return false;
        }

        const AVFilter *filter;
        AVFilterContext *filter_context;

        if (params->media_type == MediaType::kAudio)
            filter = af;
        else if (params->media_type == MediaType::kVideo)
            filter = vf;
        else
            MARK_UNREACHABLE();

        int ret = avfilter_graph_create_filter(
                &filter_context, filter, params->name.c_str(), nullptr, nullptr, priv->graph);
        if (ret < 0)
        {
            QLOG(LOG_ERROR, "Failed to create output buffer '{}'", params->name);
            return false;
        }

        // TODO(sora): Apply format constraints

        ret = avfilter_link(cur->filter_ctx, cur->pad_idx, filter_context, 0);
        if (ret < 0)
        {
            QLOG(LOG_ERROR, "Failed to link input buffer with the destination node");
            return false;
        }

        priv->out_filters.emplace_back(NamedInOutFilterCtx{
            .label_name = params->name,
            .media_type = params->media_type,
            .context = filter_context
        });
    }

    // TODO(sora): Check if there are unused parameters.

    return true;
}

} // namespace anonymous

std::unique_ptr<AVFilterDAG>
AVFilterDAG::MakeFromDSL(const std::string& dsl,
                         const std::vector<InBufferParameters>& inparams,
                         const std::vector<OutBufferParameters>& outparams)
{
    if (dsl.empty())
        return nullptr;

    auto graph = std::make_unique<AVFilterDAG>();
    auto& priv = graph->priv_;

    // Memory of `graph` is managed by `AudioFilterDAG::FilterDAGPriv` object
    // automatically, so explicit `avfilter_graph_free` is not needed.
    priv->graph = avfilter_graph_alloc();
    if (!priv->graph)
        return nullptr;

    // TODO(sora): allow user to specify this from commandline
    priv->graph->nb_threads = 4;

    // Parse filter DAG descriptor (DSL)
    AVFilterInOut *inputs = nullptr, *outputs = nullptr;
    int ret = avfilter_graph_parse2(priv->graph, dsl.c_str(), &inputs, &outputs);
    ScopeExitAutoInvoker inout_releaser([&inputs, &outputs] {
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
    });
    if (ret < 0)
        return nullptr;

    // Configure inputs and outputs
    if (!configure_input_buffers(priv.get(), inputs, inparams))
        return nullptr;
    if (!configure_output_buffers(priv.get(), outputs, outparams))
        return nullptr;

    graph->inputs_count_ = static_cast<int32_t>(inparams.size());
    graph->outputs_count_ = static_cast<int32_t>(outparams.size());

    // Configure the whole filters DAG
    ret = avfilter_graph_config(priv->graph, nullptr);
    if (ret < 0)
        return nullptr;

    return graph;
}

AVFilterDAG::AVFilterDAG()
    : priv_(std::make_unique<FilterDAGPriv>())
    , inputs_count_(0)
    , outputs_count_(0)
{
}

AVFilterDAG::~AVFilterDAG() = default;

std::vector<DAG::NamedInOutBuffer>
AVFilterDAG::Filter(const std::vector<NamedInOutBuffer>& inputs)
{
    for (const auto& inbuf : inputs)
    {
        auto itr = std::find_if(priv_->in_filters.begin(), priv_->in_filters.end(),
                                [inbuf](const NamedInOutFilterCtx& ctx) {
            return (ctx.label_name == inbuf.name);
        });
        if (itr == priv_->in_filters.end())
        {
            QLOG(LOG_WARNING, "No input buffer named '{}' in the graph", inbuf.name);
            continue;
        }

        if (inbuf.media_type != itr->media_type)
        {
            QLOG(LOG_ERROR, "Media type mismatched on input buffer '{}'", inbuf.name);
            return {};
        }

        AVFrame *frame;
        if (itr->media_type == MediaType::kAudio)
        {
            if (!inbuf.audio_buffer)
            {
                QLOG(LOG_ERROR, "Invalid input buffer '{}'", inbuf.name);
                return {};
            }
            frame = inbuf.audio_buffer->CastUnderlyingPointer<AVFrame>();
        }
        else if (itr->media_type == MediaType::kVideo)
        {
            if (!inbuf.video_buffer)
            {
                QLOG(LOG_ERROR, "Invalid input buffer '{}'", inbuf.name);
                return {};
            }
            frame = inbuf.video_buffer->CastUnderlyingPointer<AVFrame>();
        }
        else
        {
            MARK_UNREACHABLE();
        }

        if (frame->hw_frames_ctx && !itr->enable_hw_frame)
        {
            QLOG(LOG_ERROR, "Input buffer '{}' does not accept HW frames", inbuf.name);
            return {};
        }

        if (av_buffersrc_add_frame_flags(itr->context, frame, 0) < 0)
        {
            QLOG(LOG_ERROR, "Failed to push input buffer '{}' into DAG", inbuf.name);
            return {};
        }
    }

    std::vector<NamedInOutBuffer> outbufs;

    for (const auto& output : priv_->out_filters)
    {
        AVFrame *frame = av_frame_alloc();
        CHECK(frame && "Failed to allocate memory");

        if (av_buffersink_get_frame(output.context, frame) < 0)
            continue;

        ScopeExitAutoInvoker frame_releaser([&frame] {
            av_frame_free(&frame);
        });

        outbufs.emplace_back(NamedInOutBuffer{
            .name = output.label_name,
            .media_type = output.media_type
        });

        if (output.media_type == MediaType::kAudio)
        {
            outbufs.back().audio_buffer = AudioBuffer::MakeFromAVFrame(frame);
            if (!outbufs.back().audio_buffer)
            {
                QLOG(LOG_ERROR, "Failed in wrapping output frame of pad '{}'", output.label_name);
                return {};
            }
        }
        else if (output.media_type == MediaType::kVideo)
        {
            outbufs.back().video_buffer = VideoBuffer::MakeFromAVFrame(frame);
            if (!outbufs.back().video_buffer)
            {
                QLOG(LOG_ERROR, "Failed in wrapping output frame of pad '{}'", output.label_name);
                return {};
            }
        }
        else
        {
            MARK_UNREACHABLE();
        }
    }

    return outbufs;
}

UTAU_NAMESPACE_END
