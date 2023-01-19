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

#ifdef HAS_EFFECT_DSL_BUILDER_HELPER_MACROS_H
#error "A translation unit only can include this file once"
#endif /* HAS_EFFECT_DSL_BUILDER_HELPER_MACROS_H */

#define HAS_EFFECT_DSL_BUILDER_HELPER_MACROS_H

#define AUTO_SELECT(v)  ((v) ? *(v) : nullptr)
#define AUTO_SELECT_PTR(v) ((v) ? &(*(v)) : nullptr)

#define DEF_BUILDER(name) Effector builder_##name (EffectStack& st, int argc)
#define THROW_IF_NULL(v, arg, flt) \
    do {                           \
        if (!(v)) {                \
            g_throw(Error, "Argument `" #arg "` for `" #flt "` cannot be null"); \
        }                          \
    } while (false)

#define POP_ARGUMENT(name, type) \
    auto name = st.top()->To##type##Safe(); \
    st.pop();

#define POP_ARGUMENT_CHECKED(name, type, filter_name)   \
    POP_ARGUMENT(name, type)                            \
    THROW_IF_NULL(name, name, filter_name);

#define CHECK_ARGC(n, filter_name) \
    if (argc != (n)) g_throw(Error, "Wrong number of arguments for `" #filter_name "` filter");
