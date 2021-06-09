#ifndef COCOA_EXCEPTION_H
#define COCOA_EXCEPTION_H

#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <ostream>
#include <sstream>
#include <functional>

namespace cocoa {

class BeforeLeaveScope
{
public:
    explicit BeforeLeaveScope(std::function<void(void)> func);
    ~BeforeLeaveScope();

    void cancel();

private:
    std::function<void(void)>   fFunction;
};

class RuntimeException : public std::exception
{
public:
    struct Frame
    {
        void        *procAddress;
        void        *pc;
        off64_t     offset;
        std::string symbol;
        std::string file;
    };
    using Frames = std::vector<Frame>;

    class Builder
    {
    public:
        explicit Builder(const std::string& who);

        Builder(const Builder&) = delete;
        Builder(Builder&&) = delete;
        Builder& operator=(const Builder&) = delete;
        Builder& operator=(Builder&&) = delete;

        template<typename T>
        Builder& append(T&& val)
        {
            fStream << val;
            return *this;
        }

        template<typename Except>
        Except make()
        {
            return Except(fWho, fStream.str());
        }

    private:
        std::ostringstream      fStream;
        std::string             fWho;
    };

    class FrameIterable
    {
    public:
        FrameIterable(Frames::const_iterator begin, Frames::const_iterator end);

        Frames::const_iterator begin() const noexcept;
        Frames::const_iterator end() const noexcept;

    private:
        Frames::const_iterator   fBegin;
        Frames::const_iterator   fEnd;
    };

    RuntimeException(const std::string& who, const std::string& what);
    RuntimeException(const RuntimeException& other);

    const char *what() const noexcept override;
    const char *who() const noexcept;
    FrameIterable frames() const noexcept;

private:
    void recordFrames();

private:
    std::shared_ptr<Frames>     fFrames;
    std::string                 fWho;
    std::string                 fWhat;
};

#define RUNTIME_EXCEPTION_ASSERT(expr)                  \
    if (!(expr))                                        \
    {                                                   \
        throw RuntimeException::Builder(__FUNCTION__)   \
                .append("Expression ")                  \
                .append(#expr)                          \
                .append(" should be true")              \
                .make<RuntimeException>();              \
    }                                                   \

} // namespace cocoa

#endif //COCOA_EXCEPTION_H
