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

#ifndef COCOA_CORE_EXCEPTION_H
#define COCOA_CORE_EXCEPTION_H

#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <ostream>
#include <sstream>
#include <functional>

#include "Core/Project.h"

namespace cocoa {

/**
 * A helper class to implement RAII (Resource Acquisition Is Initialization).
 * Constructing a `ScopeExitAutoInvoker` on stack with a callable object
 * (functor, function pointer, or lambda expression), it will be called when
 * program exits current scope.
 * Typically, `ScopeExitAutoInvoker` is used as a helper class to release resources.
 * Like:
 * @code
 * auto ptr = new uint8_t[size];
 * ScopeExitAutoInvoker scope([ptr]() { delete[] ptr; });
 * @endcode
 */
class ScopeExitAutoInvoker
{
public:
    explicit ScopeExitAutoInvoker(std::function<void(void)> func);
    ~ScopeExitAutoInvoker();

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

        g_nodiscard inline Frames::const_iterator begin() const noexcept {
            return frames_->cbegin();
        }
        g_nodiscard inline Frames::const_iterator end() const noexcept {
            return frames_->cend();
        }

    private:
        std::shared_ptr<Frames> frames_;
    };

    RuntimeException(std::string who, std::string what);
    RuntimeException(const RuntimeException& other);

    g_nodiscard const char *what() const noexcept override;
    g_nodiscard const char *who() const noexcept;
    g_nodiscard FrameIterable frames() const noexcept;

private:
    void recordFrames();

private:
    std::shared_ptr<Frames>     fFrames;
    std::string                 fWho;
    std::string                 fWhat;
};

} // namespace cocoa

#endif //COCOA_CORE_EXCEPTION_H
