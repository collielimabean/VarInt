#pragma once

#include <cstdint>
#include <stdexcept>

static constexpr int MAX_BIT_WIDTH = 64;
static constexpr bool check_width(unsigned width)
{
    return width > 0 && width < MAX_BIT_WIDTH;
}

// behavior when signed & unsigned mix?

namespace VarInt
{
    class VarInt
    {
        public:
            /* constructors */

            VarInt::VarInt(uintmax_t val, unsigned width, bool is_signed)
            {
                if (!check_width(width))
                    throw std::invalid_argument("Bit width is zero or larger than max suppported.");

                this->_val = val;
                this->_width = width;
                this->_signed = is_signed;
                this->_overflowed = false;          
            }

            VarInt(const VarInt& other) noexcept 
            {

            }

            ~VarInt() noexcept
            {
            }

            /* Getters */
            uintmax_t value() const noexcept
            {
                return this->_val;
            }

            unsigned width() const noexcept
            {
                return this->_width;
            }

            bool is_signed() const noexcept
            {
                return this->_signed;
            }

            bool overflowed() const noexcept
            {
                return _overflowed;
            }

            /* Setters */
            void set_value(uintmax_t newval) noexcept
            {

            }

            void set_value(uintmax_t newval, unsigned new_width)
            {

            }




            /* Arithmetic */
            // + - * / % � & | ~ ! < > += -= *= /= %= �= &= |= << >> >>= <<= == != <= >= && || ++ -- , [ ] //
            VarInt add(VarInt other)
            {

            }

            /* Bitwise */
            VarInt arith_rshift(unsigned bits)
            {

            }

            VarInt logical_rshift(unsigned bits)
            {

            }

            /* Relational */


            /* Misc functions */

            VarInt sext(unsigned new_width)
            {
                if (!check_width(new_width) || new_width < _width)
                    throw std::invalid_argument("Invalid width!");
                else if (new_width == _width)
                    return *this;

                // sext //
            }

            VarInt usext(unsigned new_width)
            {
                if (!check_width(new_width) || new_width < _width)
                    throw std::invalid_argument("Invalid width!");
                else if (new_width == _width)
                    return *this;

                // usext //
            }

            VarInt truncate(unsigned new_width)
            {
                if (!check_width(new_width) || new_width > _width)
                    throw std::invalid_argument("Invalid width!");
                else if (new_width == _width)
                    return *this;

                // truncate //
            }

            VarInt slice(unsigned start, unsigned end)
            {
                // inclusive/exclusive?
            }

            VarInt slice(unsigned start)
            {

            }

            /* Operators
            * arithmetic: throw if not same bitwidth
            * relational: no issues
            * + - * / % � & | ~ ! < > += -= *= /= %= �= &= |= << >> >>= <<= == != <= >= && || ++ -- , [ ]
            */

        private:
            uintmax_t _val;
            unsigned _width;
            bool _signed;
            bool _overflowed;
    };
}
