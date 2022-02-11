#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <optional>
#include <iostream>
#include <string>
#include <ctime>
#include <tinyxml2.h>
#include <fstream>
#include <vector>

#include "fmt/format.h"
#include "fmt/ostream.h"

#include "md5.h"

#define TEMPLATE_XML_NAME   "qresource.template.xml"
#define COMPILED_XML_NAME   "qresource.xml"

#define COMPILER_ID         "Cocoa Official C++ Implementation [qresc]"
#define COMPILER_UUID       "95b44fbd-4df5-47c0-bd46-c0d4699d1e10"

#define QRES_XML_ATTR_QRESOURCE_SPEC    "org.cocoa.qresource.standard"
#define QRES_XML_ATTR_PACKAGE_IMPL      "org.cocoa.qresource.standard.package"
#define QRES_XML_ATTR_COMPILER_IMPL     "org.cocoa.qresource.standard.compiler"

#define CHECKSUM_BUF_SIZE               1024
#define DIR_SCAN_MAX_RECURSIVE_DEPTH    512

using tinyxml2::XMLDocument;
using tinyxml2::XMLError;
using tinyxml2::XMLNode;
using tinyxml2::XMLElement;
using tinyxml2::XMLText;

std::string template_xml_md5sum;
std::string compiled_xml_file;
std::string input_dir;
std::string template_checksum_file;
std::vector<std::string> toplevel_file_objects;

bool path_is_directory(const std::string& path)
{
    if (path.empty())
        return false;

    struct stat stbuf{};
    if (::stat(path.c_str(), &stbuf) < 0)
        return false;

    return S_ISDIR(stbuf.st_mode);
}

XMLText *check_whether_unique_text_child(XMLElement *element)
{
    if (element->NoChildren())
    {
        fmt::print(std::cerr, "Error: Element '{}' should have text content\n", element->Value());
        return nullptr;
    }

    if (element->FirstChild() != element->LastChild())
    {
        fmt::print(std::cerr, "Error: Element '{}' has redundant children\n", element->Value());
        return nullptr;
    }

    XMLText *maybeText = element->FirstChild()->ToText();
    if (!maybeText)
    {
        fmt::print(std::cerr, "Error: Element '{}' should have text content\n", element->Value());
        return nullptr;
    }

    return maybeText;
}

bool process_package_element(XMLElement *package, std::string& package_name)
{
    package->SetAttribute("implements", QRES_XML_ATTR_PACKAGE_IMPL);

    XMLText *textNode = check_whether_unique_text_child(package);
    if (!textNode)
        return false;

    package_name = textNode->Value();

    return true;
}

void insert_compiler_info_element(XMLElement *parent)
{
    XMLElement *node = parent->InsertNewChildElement("compiler");

    node->SetAttribute("implements", QRES_XML_ATTR_COMPILER_IMPL);
    node->SetAttribute("time", fmt::format("{}", std::time(nullptr)).c_str());

    node->InsertNewChildElement("id")->InsertNewText(COMPILER_ID);
    node->InsertNewChildElement("uuid")->InsertNewText(COMPILER_UUID);

    XMLElement *sourceVerify = node->InsertNewChildElement("source-verify");
    sourceVerify->SetAttribute("algorithm", "MD5");
    sourceVerify->InsertNewText(template_xml_md5sum.c_str());
}

void insert_directory(XMLElement *element, const std::string& dir,
                      const std::string& relative_path, int depth)
{
    if (depth >= DIR_SCAN_MAX_RECURSIVE_DEPTH)
    {
        fmt::print(std::cerr, "Fatal: Maybe stackoverflow: directory tree is too deep "
                   "(reaches max recursive depth limitation)\n");
        exit(1);
    }

    DIR *dirp = opendir(dir.c_str());

    struct dirent *entry;
    entry = readdir(dirp);
    while (entry)
    {
        if (std::strcmp(entry->d_name, "..") == 0 || std::strcmp(entry->d_name, ".") == 0 ||
            (std::strcmp(entry->d_name, "qresource.template.xml") == 0 && depth == 0))
        {
            entry = readdir(dirp);
            continue;
        }

        bool is_dir = entry->d_type == DT_DIR;

        auto current_rel_path = fmt::format("{}/{}", relative_path, entry->d_name);
        auto current_path = fmt::format("{}/{}", dir, entry->d_name);

        XMLElement *sub = element->InsertNewChildElement("entry");
        sub->SetAttribute("type", is_dir ? "directory" : "file");
        sub->SetAttribute("path", current_rel_path.c_str());

        if (depth == 0)
            toplevel_file_objects.push_back(current_path);

        if (is_dir)
            insert_directory(element, current_path, current_rel_path, depth + 1);

        entry = readdir(dirp);
    }

    closedir(dirp);
}

void insert_file_scan(XMLElement *parent)
{
    XMLElement *objects = parent->InsertNewChildElement("objects");
    insert_directory(objects, input_dir, "", 0);
}

bool process_root_template_element(XMLElement *root, std::string& package_name)
{
    if (std::strcmp(root->Value(), "template") != 0)
    {
        fmt::print(std::cerr, "Error: Root element is '{}' instead of 'template'\n", root->Value());
        return false;
    }

    root->SetName("qresource", true);
    root->SetAttribute("spec", QRES_XML_ATTR_QRESOURCE_SPEC);

    XMLElement *cur = root->FirstChildElement();
    while (cur)
    {
        if (std::strcmp(cur->Value(), "package") == 0)
        {
            if (!process_package_element(cur, package_name))
                return false;
        }
        else if (std::strcmp(cur->Value(), "description") == 0 ||
                 std::strcmp(cur->Value(), "copyright") == 0)
        {
            if (!check_whether_unique_text_child(cur))
                return false;
        }
        else
        {
            fmt::print(std::cerr, "Warning: Redundant element '{}' in template XML will be deleted\n",
                       cur->Value());

            XMLElement *stored_cur = cur->NextSiblingElement();
            root->DeleteChild(cur);
            cur = stored_cur;
            continue;
        }
        cur = cur->NextSiblingElement();
    }

    insert_compiler_info_element(root);
    insert_file_scan(root);

    return true;
}

bool process_document_object(XMLDocument *document, std::string& package_name)
{
    if (document->NoChildren())
    {
        fmt::print(std::cerr, "Error: Empty XML document\n");
        return false;
    }

    if (document->FirstChildElement() != document->LastChildElement())
    {
        fmt::print(std::cerr, "Error: More than one root element are provided\n");
        return false;
    }

    return process_root_template_element(document->FirstChildElement(), package_name);
}

bool calculate_checksum(const std::string& file, MD5& checksum)
{
    auto *buffer = new char[CHECKSUM_BUF_SIZE];

    int fd = open(file.c_str(), O_RDONLY);
    if (fd < 0)
    {
        fmt::print(std::cerr, "Error: Could not open file {}\n", file);
        delete[] buffer;
        return false;
    }

    size_t read_size;
    while ((read_size = read(fd, buffer, CHECKSUM_BUF_SIZE)) > 0)
        checksum.update(buffer, read_size);

    checksum.finalize();
    close(fd);
    delete[] buffer;
    return true;
}

bool save_template_checksum_file(const std::string& dir)
{
    template_checksum_file = dir;
    template_checksum_file.push_back('/');
    template_checksum_file.append("qresource.template.checksum");

    std::ofstream fs(template_checksum_file);
    if (!fs.is_open())
    {
        fmt::print(std::cerr, "Error: Failed to create file {}\n", template_checksum_file);
        return false;
    }
    fs << template_xml_md5sum;
    return true;
}

std::optional<std::string> find_system_program_in_path(const std::string& name)
{
    const char *path_env = getenv("PATH");
    if (!path_env)
    {
        fmt::print(std::cerr, "Error: $PATH is not set\n");
        return {};
    }

    const char *sp = path_env;
    const char *ep = path_env;
    while (true)
    {
        if (*ep == ':' || *ep == '\0')
        {
            std::string current(sp, ep - sp);
            if (!current.empty())
            {
                current.push_back('/');
                current.append(name);
                if (!path_is_directory(current) && !access(current.c_str(), F_OK | X_OK))
                    return current;
            }
            sp = ep + 1;
        }

        if (*ep == '\0')
            break;
        ep++;
    }

    return {};
}

void exit_from_vfork(int status)
{
    _exit(status);
}

bool serialize_compress_crpkg(const std::string& outfile)
{
    auto maybePath = find_system_program_in_path("mksquashfs");
    if (!maybePath)
    {
        fmt::print(std::cerr, "Error: mksquashfs: Command not found\n");
        return false;
    }
    std::string& program_path = maybePath.value();

    const char **exec_argv = new const char*[toplevel_file_objects.size() + 8];
    const char **ppArgv = exec_argv;
    *ppArgv++ = program_path.c_str();
    *ppArgv++ = compiled_xml_file.c_str();
    *ppArgv++ = template_checksum_file.c_str();
    for (const std::string& toplevel : toplevel_file_objects)
        *ppArgv++ = toplevel.c_str();
    *ppArgv++ = outfile.c_str();
    *ppArgv++ = "-no-progress";
    *ppArgv++ = "-comp";
    *ppArgv++ = "gzip";
    *ppArgv++ = nullptr;

    pid_t pid = vfork();
    if (pid < 0)
    {
        fmt::print(std::cerr, "Error: Failed in vfork: {}\n", std::strerror(errno));
        return false;
    }
    else if (pid == 0)
    {
        int ret = execvp(program_path.c_str(), const_cast<char *const*>(exec_argv));
        if (ret < 0)
            exit_from_vfork(1);
        /* Unreachable */
    }
    delete[] exec_argv;

    int status;
    int ret = waitpid(pid, &status, 0);
    if (ret < 0 && errno != SIGCHLD)
    {
        fmt::print(std::cerr, "Error: Failed in waitpid: {}\n", std::strerror(errno));
        return false;
    }

    if (WEXITSTATUS(status) != 0)
    {
        fmt::print(std::cerr, "Error: Failed to compress package\n");
        return false;
    }

    return true;
}

int main(int argc, const char **argv)
{
    if (argc > 3 || argc < 2)
    {
        fmt::print(std::cerr, "Usage: {} <input dir> [<output dir>]\n", argv[0]);
        return 1;
    }

    if (!path_is_directory(argv[1]))
    {
        fmt::print(std::cerr, "Error: {}: directory not found or not a directory\n", argv[1]);
        return 1;
    }
    input_dir = argv[1];
    std::string output_dir;

    if (argc == 3)
    {
        if (!path_is_directory(argv[1]))
        {
            fmt::print(std::cerr, "Error: {}: directory not found or not a directory\n", argv[1]);
            return 1;
        }
        output_dir = argv[2];
    }
    else
        output_dir = '.';

    std::string template_xml_file = input_dir + "/" TEMPLATE_XML_NAME;

    if (access(template_xml_file.c_str(), R_OK) < 0)
    {
        fmt::print(std::cerr, "Error: Failed to access {}: {}\n",
                   template_xml_file, std::strerror(errno));
        return 1;
    }
    if (path_is_directory(template_xml_file))
    {
        fmt::print(std::cerr, "Error: {} is a directory\n", template_xml_file);
        return 1;
    }

    MD5 checksum;
    if (!calculate_checksum(template_xml_file, checksum))
        return 1;
    template_xml_md5sum = checksum.hexdigest();

    XMLDocument document;
    XMLError error = document.LoadFile(template_xml_file.c_str());
    if (error != XMLError::XML_SUCCESS)
    {
        fmt::print(std::cerr, "Error: {}\n", XMLDocument::ErrorIDToName(error));
        return 1;
    }

    std::string package_name;

    if (!process_document_object(&document, package_name))
        return 1;

    std::string crpkg_file_path = output_dir + "/" + package_name + ".crpkg";

    std::string tmp_dir = fmt::format("/tmp/qresc-{}", getpid());
    if (mkdir(tmp_dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) < 0)
    {
        fmt::print(std::cerr, "Error: Failed to create temporary files: {}",
                   std::strerror(errno));
        return 1;
    }
    int ret = 0;

    if (!save_template_checksum_file(tmp_dir))
    {
        ret = 1;
        goto out;
    }

    compiled_xml_file = input_dir + "/" COMPILED_XML_NAME;
    if (document.SaveFile(compiled_xml_file.c_str()) != XMLError::XML_SUCCESS)
    {
        fmt::print(std::cerr, "Error: Failed write compiled XML to {}\n", compiled_xml_file);
        ret = 1;
        goto out;
    }

    remove(crpkg_file_path.c_str());
    if (!serialize_compress_crpkg(crpkg_file_path))
        ret = 1;

out:
    remove(compiled_xml_file.c_str());
    remove(template_checksum_file.c_str());
    rmdir(tmp_dir.c_str());

    return ret;
}
