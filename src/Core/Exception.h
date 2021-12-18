#ifndef COCOA_EXCEPTION_H
#define COCOA_EXCEPTION_H

#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <ostream>
#include <sstream>
#include <functional>

#include "Core/Project.h"

namespace cocoa {

class ScopeEpilogue
{
public:
    explicit ScopeEpilogue(std::function<void(void)> func);
    ~ScopeEpilogue();

    void abolish();

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
        explicit Builder(std::string  who);

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
        explicit FrameIterable(std::shared_ptr<Frames> frames) : frames_(std::move(frames)) {}
        ~FrameIterable() = default;

        co_nodiscard inline Frames::const_iterator begin() const noexcept {
            return frames_->cbegin();
        }
        co_nodiscard inline Frames::const_iterator end() const noexcept {
            return frames_->cend();
        }

    private:
        std::shared_ptr<Frames> frames_;
    };

    RuntimeException(std::string who, std::string what);
    RuntimeException(const RuntimeException& other);

    co_nodiscard const char *what() const noexcept override;
    co_nodiscard const char *who() const noexcept;
    co_nodiscard FrameIterable frames() const noexcept;

private:
    void recordFrames();

private:
    std::shared_ptr<Frames>     fFrames;
    std::string                 fWho;
    std::string                 fWhat;
};

} // namespace cocoa

#endif //COCOA_EXCEPTION_H
