// ============================= BIT ALGORITHM ============================== //
// Project:         The C++ Bit Library
// Name:            bit_algorithm.hpp
// Description:     Optimized versions of algorithms for bit manipulation
// Creator:         Vincent Reverdy
// Contributor(s):  Vincent Reverdy [2015-2017]
//                  Maghav Kumar [2016-2017]
// License:         BSD 3-Clause License
// ========================================================================== //
#ifndef _BIT_ALGORITHM_HPP_INCLUDED
#define _BIT_ALGORITHM_HPP_INCLUDED
// ========================================================================== //



// ================================ PREAMBLE ================================ //
// C++ standard library
// Project sources
#include "bit_details.hpp"
#include "bit_value.hpp"
#include "bit_reference.hpp"
#include "bit_pointer.hpp"
#include "bit_iterator.hpp"
// Third-party libraries
// Miscellaneous
namespace bit {
// ========================================================================== //



/* ***************************** BIT CONSTANTS ****************************** */
// Bit constants
static constexpr bit_value zero_bit(0U);
static constexpr bit_value one_bit(1U);
/* ************************************************************************** */



/* ************************** STANDARD ALGORITHMS *************************** */
// Non-modifying sequence operations
template <class InputIt> 
typename bit_iterator<InputIt>::difference_type
count(
    bit_iterator<InputIt> first, 
    bit_iterator<InputIt> last, 
    bit_value value
);

// Modifying sequence operations
template <class BidirIt> 
void reverse(
    bit_iterator<BidirIt> first, 
    bit_iterator<BidirIt> last
);
/* ************************************************************************** */



// ------------------- NON-MODIFYING SEQUENCE OPERATIONS -------------------- //
// Counts the number of bits equal to the provided bit value
template <class InputIt> 
typename bit_iterator<InputIt>::difference_type
count(
    bit_iterator<InputIt> first, 
    bit_iterator<InputIt> last, 
    bit_value value
)
{
    // Assertions
    _assert_range_viability(first, last);
    
    // Initialization
    using underlying_type = typename bit_iterator<InputIt>::underlying_type;
    using difference_type = typename bit_iterator<InputIt>::difference_type;
    constexpr difference_type digits = binary_digits<underlying_type>::value;
    difference_type result = 0;
    auto it = first.base();
    
    // Computation when bits belong to several underlying values
    if (first.base() != last.base()) {
        if (first.position() != 0) {
            result = _popcnt(*first.base() >> first.position());
            ++it;
        }
        for (; it != last.base(); ++it) {
            result += _popcnt(*it);
        }
        if (last.position() != 0) {
            result += _popcnt(*last.base() << (digits - last.position()));
        }
    // Computation when bits belong to the same underlying value
    } else {
        result = _popcnt(_bextr<underlying_type>(
            *first.base(), 
            first.position(), 
            last.position() - first.position()
        ));
    }
    
    // Negates when the number of zero bits is requested
    if (!static_cast<bool>(value)) {
        result = std::distance(first, last) - result;
    }
    
    // Finalization
    return result;
}
// -------------------------------------------------------------------------- //



// --------------------- MODIFYING SEQUENCE OPERATIONS ---------------------- //
// Reverses the order of the bits in the provided range
template <class BidirIt> 
void reverse(
    bit_iterator<BidirIt> first, 
    bit_iterator<BidirIt> last
)
{
    // Assertions
    _assert_range_viability(first, last);
    
    // Initialization
    using underlying_type = typename bit_iterator<BidirIt>::underlying_type;
    using size_type = typename bit_iterator<BidirIt>::size_type;
    constexpr size_type digits = binary_digits<underlying_type>::value;
    const bool is_last_null = last.position() == 0;
    size_type diff = (digits - last.position()) * !is_last_null;
    auto it = first.base();
    underlying_type first_value = {};
    underlying_type last_value = {};
    
    // Reverse when bit iterators are aligned
    if (first.position() == 0 && last.position() == 0) {
        std::reverse(first.base(), last.base());
        for (; it !=  last.base(); ++it) {
            *it = _bitswap(*it);
        }
    // Reverse when bit iterators do not belong to the same underlying value
    } else if (first.base() != last.base()) {
        // Save first and last element
        first_value = *first.base();
        last_value = *std::prev(last.base(), is_last_null);
        // Reverse the underlying sequence
        std::reverse(first.base(), std::next(last.base(), !is_last_null));
        // Shift the underlying sequence to the left
        if (first.position() < diff) {
            it = first.base();
            diff = diff - first.position();
            for (; it != last.base(); ++it) {
                *it = _shld<underlying_type>(*it, *std::next(it), diff);
            }
            *it <<= diff;
            it = first.base();
        // Shift the underlying sequence to the right
        } else if (first.position() > diff) {
            it = std::prev(last.base(), is_last_null);
            diff = first.position() - diff;
            for (; it != first.base(); --it) {
                *it = _shrd<underlying_type>(*it, *std::prev(it), diff);
            }
            *it >>= diff; 
            it = first.base();
        }
        // Bitswap every element of the underlying sequence
        for (; it != std::next(last.base(), !is_last_null); ++it) {
            *it = _bitswap(*it);
        }
        // Blend bits of the first element
        if (first.position() != 0) {
            *first.base() = _bitblend<underlying_type>(
                first_value,
                *first.base(),
                first.position(),
                digits - first.position()
            );
        }
        // Blend bits of the last element
        if (last.position() != 0) {
            *last.base() = _bitblend<underlying_type>(
                *last.base(),
                last_value,
                last.position(),
                digits - last.position()
            );
        }
    // Reverse when bit iterators belong to the same underlying value
    } else {
        *it = _bitblend<underlying_type>(
            *it, 
            _bitswap(*it >> first.position()) >> diff, 
            first.position(), 
            last.position() - first.position()
        );
    }
}
// -------------------------------------------------------------------------- //



// ========================================================================== //
} // namespace bit
#endif // _BIT_ALGORITHM_HPP_INCLUDED
// ========================================================================== //
