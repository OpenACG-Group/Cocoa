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

#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#include <fstream>

#include "fmt/format.h"

#include "Core/Project.h"
#include "Core/Exception.h"
#include "CRPKG/Composer.h"
COCOA_BEGIN_NAMESPACE

using Entry = crpkg::Composer::Entry;

// NOLINTNEXTLINE
Entry read_directory_entries(const std::string& path,
                             const std::string& this_entry_name)
{
    DIR *dirp = opendir(path.c_str());
    if (!dirp)
    {
        fmt::print(stderr, "Failed to open directory {}: {}\n",
                   path, strerror(errno));
        return {};
    }

    ScopeExitAutoInvoker closer([dirp]() {
        closedir(dirp);
    });

    Entry entry(this_entry_name);
    while (struct dirent *d = readdir(dirp))
    {
        std::string entry_name = d->d_name;
        if (entry_name == "." || entry_name == "..")
            continue;

        std::string subpath = fmt::format("{}/{}", path, entry_name);

        if (d->d_type == DT_DIR)
        {
            Entry subentry = read_directory_entries(subpath, entry_name);
            if (subentry.type == crpkg::Composer::kEmpty_EntryType)
                return {};
            entry.children.emplace_back(std::move(subentry));
        }
        else if (d->d_type == DT_REG)
        {
            entry.children.emplace_back(
                    entry_name, crpkg::Composer::DataAccessor::MakeFromFile(subpath));
        }
        else
        {
            fmt::print("Skipped non-regular file {}\n", subpath);
        }
    }

    return entry;
}

int packager_main(const char *output, const char *path)
{
    std::ofstream fs(output, std::ios_base::out | std::ios_base::binary);
    if (!fs.is_open())
    {
        fmt::print(stderr, "Failed to open output file: {}\n", output);
        return 1;
    }

    Entry entries = read_directory_entries(path, "root");

    crpkg::Composer::Compose(entries, [&fs](const uint8_t *data, size_t size) {
        fs.write(reinterpret_cast<const char*>(data),
                 static_cast<std::streamsize>(size));
        return size;
    });

    return 0;
}

COCOA_END_NAMESPACE

int main(int argc, const char **argv)
{
    if (argc != 3)
    {
        fmt::print(stderr, "Usage: {} <output.crpkg> <path>\n", argv[0]);
        return 1;
    }

    return cocoa::packager_main(argv[1], argv[2]);
}