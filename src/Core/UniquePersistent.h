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

#ifndef COCOA_CORE_UNIQUEPERSISTENT_H
#define COCOA_CORE_UNIQUEPERSISTENT_H

#include <utility>
#include <stdexcept>

#include "Core/Errors.h"

namespace cocoa {

template<typename T>
class UniquePersistent
{
public:
    static T *Instance() {
        CHECK(self_pointer_);
        return self_pointer_;
    }

    static bool HasInstance() {
        return self_pointer_;
    }

    static T& Ref() {
        return *Instance();
    }

    template<typename...ArgsT>
    static void New(ArgsT&&...args) {
        self_pointer_ = new T(std::forward<ArgsT>(args)...);
    }

    static void Delete() {
        delete self_pointer_;
        self_pointer_ = nullptr;
    }

private:
    static T *self_pointer_;
};

template<typename T>
T *UniquePersistent<T>::self_pointer_ = nullptr;

template<typename T>
class ThreadLocalUniquePersistent
{
public:
    static T *GetCurrent() {
        CHECK(local_self_ptr_);
        return local_self_ptr_;
    }

    static bool HasInstance() {
        return local_self_ptr_;
    }

    template<typename...ArgsT>
    static void New(ArgsT&&...args) {
        local_self_ptr_ = new T(std::forward<ArgsT>(args)...);
    }

    static void Delete() {
        delete local_self_ptr_;
        local_self_ptr_ = nullptr;
    }

private:
    thread_local static T *local_self_ptr_;
};

template<typename T>
thread_local T *ThreadLocalUniquePersistent<T>::local_self_ptr_ = nullptr;

}

#endif //COCOA_CORE_UNIQUEPERSISTENT_H
