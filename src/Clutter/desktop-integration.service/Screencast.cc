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

/**
 * This bridge program should be a child process which is started by Cocoa,
 * and we print logs to standard output directly as Cocoa main process
 * will redirect them appropriately.
 * Screencast bridge program communicates with Cocoa via `subprocess` protocol
 * and transports pixel buffers through dmabuf provided by pipewire.
 */

#include <string_view>
#include <regex>
#include <functional>

#include <libdrm/drm_fourcc.h>

#include "fmt/format.h"

#include "Errors.h"
#include "DesktopPortal.h"

SCREENCAST_NAMESPACE_BEGIN

template<typename T>
std::vector<T> separate_string_list(const std::string_view& str, char dm,
                                    std::function<T(const std::string_view&)> transformer)
{
    std::vector<T> result;
    size_t p = 0;
    int64_t last_p = -1;
    while ((p = str.find(dm, p + 1)) < str.length())
    {
        std::string_view view(str);
        view.remove_prefix(last_p + 1);
        view.remove_suffix(str.size() - p);
        result.emplace_back(transformer(view));
        last_p = static_cast<int64_t>(p);
    }
    std::string_view view(str);
    view.remove_prefix(last_p + 1);
    result.emplace_back(transformer(view));

    return result;
}

TextureFormat texture_format_transformer(const std::string_view& src)
{
    static std::map<std::string_view, TextureFormat> tb{
            { "BGRA", TextureFormat::kBGRA },
            { "RGBA", TextureFormat::kRGBA },
            { "BGRX", TextureFormat::kBGRX },
            { "RGBX", TextureFormat::kRGBX }
    };

    if (tb.count(src) == 0)
        return TextureFormat::kUnknown;

    return tb[src];
}

struct DrmFormatModsPair
{
    TextureFormat   format;
    ParametersFromHost::ModifiersList mods;
};

DrmFormatModsPair drm_format_mods_pair_transformer(const std::string_view& src)
{
    size_t bp = src.find_first_of(':');
    if (bp >= src.length())
        bp = src.length();

    std::string_view sfmt(src);
    sfmt.remove_suffix(src.length() - bp);

    DrmFormatModsPair pair{};
    pair.format = texture_format_transformer(sfmt);

    if (bp < src.length())
    {
        std::string_view smods(src);
        smods.remove_prefix(bp + 1);

        pair.mods = separate_string_list<uint64_t>(smods, ',', [](const std::string_view& m) -> uint64_t {
            std::string dump(m);

            char *endp = nullptr;
            uint64_t num = strtoull(dump.c_str(), &endp, 16);

            if (!endp || *endp != '\0')
                return 0;
            return num;
        });
    }

    return pair;
}

ParametersFromHost *g_host_params = nullptr;

bool initialize_parameters(int argc, const char **argv)
{
    g_host_params = new ParametersFromHost;
    g_host_params->support_dmabuf_implicit_modifiers = false;

    std::regex pattern("--([a-zA-Z-]+)(=([-_a-zA-Z0-9,:!]+)){0,1}");

    for (int i = 1; i < argc; i++)
    {
        const char *sv = argv[i];

        if (strlen(sv) <= 2 || (sv[0] != '-' && sv[1] != '-'))
        {
            fmt::print("[main] Malformed commandline option {}\n", sv);
            return false;
        }

        std::smatch toplevel_match_result;
        std::string str_sv(sv);
        if (!std::regex_match(str_sv, toplevel_match_result, pattern))
        {
            fmt::print("[main] Malformed commandline option {}\n", sv);
            return false;
        }

        std::string option = toplevel_match_result[1].str();
        std::string value = toplevel_match_result[3].str();

        if (option == "help")
        {
            fmt::print("Cocoa ScreenCast helper for Linux Wayland platform.\n");
            fmt::print("This program should be launched by Cocoa as a subprocess,\n");
            fmt::print("and anyone should not run this program independently.\n\n");
            fmt::print("Compatibility signature: {}\n", SCREENCAST_COMPAT_SIGNATURE);
            fmt::print("OpenACG Group, Cocoa Project <https://github.com/OpenACG-Group/Cocoa>\n");
            exit(0);
        }
        else if (option == "compatibility-signature")
        {
            g_host_params->compatibility_signature = value;
        }
        else if (option == "drm-support-dmabuf-implicit-modifiers")
        {
            g_host_params->support_dmabuf_implicit_modifiers = true;
        }
        else if (option == "raster-formats")
        {
            g_host_params->raster_formats = separate_string_list<TextureFormat>(value, ',',
                                                                                texture_format_transformer);
            // TODO: Check if they are valid formats
        }
        else if (option == "drm-formats")
        {
            auto vformats = separate_string_list<DrmFormatModsPair>(value, '!',
                                                                    drm_format_mods_pair_transformer);
            for (const auto& pair : vformats)
                g_host_params->drm_formats[pair.format] = pair.mods;
        }
        else if (option == "fps-fraction")
        {
            // NOLINTNEXTLINE
            sscanf(value.c_str(), "%u:%u", &g_host_params->fps_num, &g_host_params->fps_den);
        }
        else if (option == "host-accept-dmabuf")
        {
            g_host_params->host_accept_dmabuf = true;
        }
        else
        {
            fmt::print("[main] Invalid commandline option {}\n", sv);
            return false;
        }
    }

    return true;
}

int wayland_screencast_bridge_main(int argc, const char **argv)
{
    if (!initialize_parameters(argc, argv))
        return EXIT_FAILURE;

    if (g_host_params->compatibility_signature.empty())
    {
        fmt::print("[main] Compatibility signature was not provided\n");
        return EXIT_FAILURE;
    }

    if (g_host_params->compatibility_signature != SCREENCAST_COMPAT_SIGNATURE)
    {
        fmt::print("[main] Mismatched compatibility signature. "
                   "This program is not compatible with this version of Cocoa\n");
        return EXIT_FAILURE;
    }

    if (g_host_params->support_dmabuf_implicit_modifiers)
    {
        // Add an implicit modifier. Host process should NOT pass implicit modifiers (0xffffffffffffff)
        // by commandline option `--drm-formats`.
        for (auto& pair : g_host_params->drm_formats)
            pair.second.push_back(DRM_FORMAT_MOD_INVALID);
    }

    auto portal = DesktopPortal::Make();

    GMainLoop *loop = g_main_loop_new(nullptr, false);
    CHECK(loop);

    g_main_loop_run(loop);

    g_main_loop_unref(loop);
    return 0;
}

SCREENCAST_NAMESPACE_END

int main(int argc, const char **argv)
{
    return cocoa::wlcast::wayland_screencast_bridge_main(argc, argv);
}
