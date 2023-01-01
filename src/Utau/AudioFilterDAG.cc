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

#include "Core/Exception.h"
#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Utau/AudioFilterDAG.h"
#include "Utau/AudioBuffer.h"
#include "Utau/ffwrappers/libavfilter.h"
#include "Utau/ffwrappers/libavutil.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau.AudioFilterDAG)

struct NamedInOutFilterCtx
{
    std::string         label_name;

    // For input filters, `abuffersrc` filter is associated;
    // for output filters, `abuffersink` filter is associated.
    AVFilterContext    *context;

    AudioChannelMode    channel_mode;
    SampleFormat        sample_fmt;
    int32_t             sample_rate;
};

struct AudioFilterDAG::FilterDAGPriv
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

using DAG = AudioFilterDAG;

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

        std::string args = fmt::format("sample_fmt={}:sample_rate={}:channel_layout={}",
                                       static_cast<int32_t>(SampleFormatToLibavFormat(params->sample_fmt)),
                                       params->sample_rate,
                                       params->channel_mode == AudioChannelMode::kStereo ? "stereo" : "mono");

        AVFilterContext *af_context;

        int ret = avfilter_graph_create_filter(
                &af_context, af, params->name.c_str(), args.c_str(), nullptr, priv->graph);
        if (ret < 0)
        {
            QLOG(LOG_ERROR, "Failed to create input buffer '{}'", params->name);
            return false;
        }

        ret = avfilter_link(af_context, 0, cur->filter_ctx, cur->pad_idx);
        if (ret < 0)
        {
            QLOG(LOG_ERROR, "Failed to link input buffer with the destination node");
            return false;
        }

        priv->in_filters.emplace_back(NamedInOutFilterCtx{
            .label_name = params->name,
            .context = af_context,
            .channel_mode = params->channel_mode,
            .sample_fmt = params->sample_fmt,
            .sample_rate = params->sample_rate
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

        AVFilterContext *af_context;

        int ret = avfilter_graph_create_filter(
                &af_context, af, params->name.c_str(), nullptr, nullptr, priv->graph);
        if (ret < 0)
        {
            QLOG(LOG_ERROR, "Failed to create output buffer '{}'", params->name);
            return false;
        }

        // TODO(sora): Apply format constraints

        ret = avfilter_link(cur->filter_ctx, cur->pad_idx, af_context, 0);
        if (ret < 0)
        {
            QLOG(LOG_ERROR, "Failed to link input buffer with the destination node");
            return false;
        }

        // TODO(sora): push
        priv->out_filters.emplace_back(NamedInOutFilterCtx{
            .label_name = params->name,
            .context = af_context
        });
    }

    // TODO(sora): Check if there are unused parameters.

    return true;
}

} // namespace anonymous

std::unique_ptr<AudioFilterDAG>
AudioFilterDAG::MakeFromDSL(const std::string& dsl,
                            const std::vector<InBufferParameters>& inparams,
                            const std::vector<OutBufferParameters>& outparams)
{
    if (dsl.empty())
        return nullptr;

    auto graph = std::make_unique<AudioFilterDAG>();
    auto& priv = graph->priv_;

    // Memory of `graph` is managed by `AudioFilterDAG::FilterDAGPriv` object
    // automatically, so explicit `avfilter_graph_free` is not needed.
    priv->graph = avfilter_graph_alloc();
    if (!priv->graph)
        return nullptr;

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

AudioFilterDAG::AudioFilterDAG()
    : priv_(std::make_unique<FilterDAGPriv>())
    , inputs_count_(0)
    , outputs_count_(0)
{
}

AudioFilterDAG::~AudioFilterDAG() = default;

std::vector<DAG::NamedInOutBuffer>
AudioFilterDAG::Filter(const std::vector<NamedInOutBuffer>& inputs)
{
    if (inputs.size() != inputs_count_)
        return {};

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

        auto *frame = inbuf.buffer->CastUnderlyingPointer<AVFrame>();
        if (av_buffersrc_add_frame(itr->context, frame) < 0)
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
        {
            QLOG(LOG_ERROR, "Failed to receive output buffer '{}' from DAG", output.label_name);
            av_frame_free(&frame);
            return {};
        }

        std::shared_ptr<AudioBuffer> audio_buffer = AudioBuffer::MakeFromAVFrame(frame);
        if (!audio_buffer)
        {
            QLOG(LOG_ERROR, "Failed to wrap AVFrame");
            av_frame_free(&frame);
            return {};
        }

        outbufs.emplace_back(NamedInOutBuffer{
            .name = output.label_name,
            .buffer = audio_buffer
        });
    }

    return outbufs;
}

UTAU_NAMESPACE_END
