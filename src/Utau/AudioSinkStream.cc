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

#include "Utau/AudioSinkStream.h"
UTAU_NAMESPACE_BEGIN

namespace {

int32_t g_stream_id_counter = 1;

} // namespace anonymous

AudioSinkStream::AudioSinkStream()
    : unique_id_(g_stream_id_counter++)
    , connected_(false)
{
}

AudioSinkStream::ConnectStatus AudioSinkStream::Connect(SampleFormat sample_format,
                                                        AudioChannelMode channel_mode,
                                                        int32_t sample_rate,
                                                        bool realtime)
{
    if (connected_)
        return ConnectStatus::kAlready;

    bool res = this->OnConnect(sample_format, channel_mode, sample_rate, realtime);
    if (res)
    {
        connected_ = true;
        return ConnectStatus::kSuccess;
    }
    else
    {
        return ConnectStatus::kError;
    }
}

AudioSinkStream::ConnectStatus AudioSinkStream::Disconnect()
{
    if (!connected_)
        return ConnectStatus::kAlready;

    if (this->OnDisconnect())
    {
        connected_ = false;
        return ConnectStatus::kSuccess;
    }
    else
        return ConnectStatus::kError;
}

UTAU_NAMESPACE_END
