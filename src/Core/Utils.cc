#include <unistd.h>

#include "Core/Utils.h"
#include "Core/Journal.h"
#include "Core/MeasuredTable.h"
#include "Core/Exception.h"
namespace cocoa::utils {

static void dump_property_tree_recursive(const Printer& printer, PropertyTreeNode *current, std::vector<std::string>& prefix)
{
    size_t childrenCount = std::distance(current->children().begin(),
                                         current->children().end());
    size_t i = 0;
    std::ostringstream os;

    for (PropertyTreeNode *p : current->children())
    {
        os.str("");
        for (std::string& str : prefix)
            os << str;

        if (i == childrenCount - 1)
        {
            /* It's the last one */
            os << "└── ";
            prefix.emplace_back("    ");
        }
        else
        {
            os << "├── ";
            prefix.emplace_back("│   ");
        }
        os << p->name();

        std::string str = os.str();
        printer(str);

        dump_property_tree_recursive(printer, p, prefix);
        prefix.pop_back();
        i++;
    }
}

void DumpPropertyTree(PropertyTreeNode *pRoot, Printer printer)
{
    std::vector<std::string> rec;
    printer("<root>");
    dump_property_tree_recursive(printer, pRoot, rec);
}

void ChangeWorkDirectory(const std::string& dir)
{
    int ret = ::chdir(dir.c_str());
    if (ret < 0)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to change working directory to \"")
                .append(dir)
                .append("\": ")
                .append(strerror(errno))
                .make<RuntimeException>();
    }
}

void DumpRuntimeException(const RuntimeException& except)
{
    Journal::Ref()(LOG_EXCEPTION, "{}:", except.what());
    Journal::Ref()(LOG_EXCEPTION, "Traceback (most recent call last):");

    MeasuredTable table(1);
    for (const RuntimeException::Frame& f : except.frames())
    {
        std::ostringstream hdr, content;
        if (f.symbol == "Unknown")
            hdr << "  " << f.pc << " <...>";
        else
            hdr << "  " << f.pc << " <+" << f.offset << ">";
        content << f.symbol << " from " << f.file;

        table.append(hdr.str(), content.str());
    }

    table.flush([](const std::string& str) -> void {
        Journal::Ref()(LOG_EXCEPTION, "{}", str);
    });
}

std::string GetAbsoluteDirectory(const std::string& dir)
{
    char buf[PATH_MAX];
    realpath(dir.c_str(), buf);

    return buf;
}

} // namespace cocoa::utils
