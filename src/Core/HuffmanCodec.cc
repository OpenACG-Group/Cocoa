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

#include <queue>
#include <vector>
#include <bitset>
#include <unordered_map>

#include "Core/Data.h"
#include "Core/HuffmanCodec.h"
#include "Core/ScalableWriteBuffer.h"
#include "Core/Errors.h"

#include "fmt/format.h"

namespace cocoa {

namespace {

struct HuffmanNode
{
    using ptr = std::shared_ptr<HuffmanNode>;

    bool        is_leaf_node;
    int         freq_count;
    ptr         left_child;
    ptr         right_child;
    uint8_t     symbol;
};

struct NodeComparator
{
    bool operator()(const HuffmanNode::ptr& a, const HuffmanNode::ptr& b) const
    {
        return (a->freq_count > b->freq_count);
    }
};

using Container = std::vector<HuffmanNode::ptr>;
using PriorityQueue = std::priority_queue<HuffmanNode::ptr, Container, NodeComparator>;

#define HUFFMAN_MAX_BITS    255

struct BitCode
{
    int                             num_bits;
    std::bitset<HUFFMAN_MAX_BITS>   bits;
};

using CodeTable = std::unordered_map<uint8_t, BitCode>;

#define ALPHABET_SIZE   256

void compute_symbol_frequencies(PriorityQueue& out_queue, const std::shared_ptr<Data>& input)
{
    HuffmanNode::ptr alphabet_map_table[ALPHABET_SIZE];

    auto increase_symbol_freq = [&alphabet_map_table](uint8_t symbol) {
        if (alphabet_map_table[symbol] == nullptr)
        {
            alphabet_map_table[symbol] = std::make_shared<HuffmanNode>(HuffmanNode{
                .is_leaf_node = true,
                .freq_count = 1,
                .left_child = nullptr,
                .right_child = nullptr,
                .symbol = symbol
            });
        }
        else
        {
            alphabet_map_table[symbol]->freq_count++;
        }
    };

    if (input->hasAccessibleBuffer())
    {
        auto *ptr = reinterpret_cast<const uint8_t*>(input->getAccessibleBuffer());
        auto *end_ptr = ptr + input->size();

        while (ptr < end_ptr)
        {
            increase_symbol_freq(*ptr);
            ptr++;
        }
    }
    else
    {
        uint8_t ch;
        for (size_t i = 0; i < input->size(); i++)
        {
            input->read(&ch, 1);
            increase_symbol_freq(ch);
        }
    }

    for (const HuffmanNode::ptr& node : alphabet_map_table)
    {
        if (node)
            out_queue.push(node);
    }
}

// NOLINTNEXTLINE
void generate_code_table(BitCode *out, const HuffmanNode::ptr& node, int depth,
                         std::bitset<HUFFMAN_MAX_BITS>& current_bit_code)
{
    if (node->is_leaf_node)
    {
        out[node->symbol] = BitCode {
            .num_bits = depth + 1,
            .bits = current_bit_code
        };
        return;
    }

    generate_code_table(out, node->left_child, depth + 1, current_bit_code);

    current_bit_code.set(depth, true);
    generate_code_table(out, node->right_child, depth + 1, current_bit_code);

    current_bit_code.set(depth, false);
}

} // namespace anonymous

#define INPUT_BUFFER_SLICE_SIZE     512UL

std::shared_ptr<Data> HuffmanEncode(const std::shared_ptr<Data>& input)
{
    PriorityQueue huffman_queue;
    compute_symbol_frequencies(huffman_queue, input);

    // Huffman's algorithm. Code reference from:
    // Thomas H. Cormen, Charles E. Leiserson, Ronald L. Rivest, Clifford Stein.
    // Introduction to Algorithms, Third Edition[M]. Cambridge: MIT Press, 2009.pp.428-435

    while (huffman_queue.size() > 1)
    {
        auto r = huffman_queue.top();
        huffman_queue.pop();
        auto l = huffman_queue.top();
        huffman_queue.pop();

        auto node = std::make_shared<HuffmanNode>(HuffmanNode{
            .is_leaf_node = false,
            .freq_count = r->freq_count + l->freq_count,
            .left_child = r,
            .right_child = l
        });

        huffman_queue.push(node);
    }

    // Generate a huffman code for each symbol by DFS
    BitCode cvtb[ALPHABET_SIZE];
    std::bitset<HUFFMAN_MAX_BITS> current_bit_code;
    generate_code_table(cvtb, huffman_queue.top(), 0, current_bit_code);

    ScalableWriteBuffer write_buffer;

    size_t remaining_size = input->size();
    size_t input_offset = 0;

    uint8_t bits_buffer = 0;
    int bit_offset = 0;

    while (remaining_size > 0)
    {
        size_t slice_size = std::min(remaining_size, INPUT_BUFFER_SLICE_SIZE);
        std::shared_ptr<DataSlice> slice = input->slice(input_offset, slice_size);

        for (size_t i = 0; i < slice_size; i++)
        {
            uint8_t byte = slice->at(i);
            int bitset_offset = 0;

            while (bitset_offset < cvtb[byte].num_bits)
            {
                int write_bits_num = std::min(cvtb[byte].num_bits - bitset_offset, 8);
                bitset_offset += write_bits_num;
                // TODO(sora): complete this
            }
        }

        remaining_size -= slice_size;
        input_offset += slice_size;
    }

    return write_buffer.finalize();
}

} // namespace cocoa
