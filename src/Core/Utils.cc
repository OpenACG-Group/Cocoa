#include <unistd.h>

#include "Core/Utils.h"
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

void dump_plaintext_runtime_exception(const RuntimeException& except, const Printer& printer)
{
    printer(std::string(except.what()) + ":");
    printer("Traceback (most recent call last):");

    MeasuredTable table(1);
    for (const RuntimeException::Frame& f : except.frames())
    {
        std::ostringstream hdr, content;
        hdr << "  " << f.pc << " <+" << f.offset << ">";
        content << f.symbol << " from " << f.file;

        table.append(hdr.str(), content.str());
    }

    table.flush(printer);
}

void dump_colored_runtime_exception(const RuntimeException& except, const Printer& printer)
{
#ifndef __linux__
    dump_plaintext_runtime_exception(except, printer);
#else

    printer(std::string("\033[1m") + except.what() + "\033[0m");
    printer("Traceback (most recent call last):");

    MeasuredTable table(1);
    for (const RuntimeException::Frame& f : except.frames())
    {
        std::ostringstream hdr, content;
        hdr << "  \033[34m" << f.pc << "\033[0m \033[36m<+" << f.offset
            << ">\033[0m";
        content << "\033[33m" << f.symbol << "\033[0m from \033[32m " << f.file;

        table.append(hdr.str(), content.str());
    }

    table.flush(printer);

#endif // __linux__
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

void DumpRuntimeException(const RuntimeException& except, bool color, Printer printer)
{
    if (color)
        dump_colored_runtime_exception(except, printer);
    else
        dump_plaintext_runtime_exception(except, printer);
}

} // namespace cocoa
