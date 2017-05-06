#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <string>

static constexpr int MAX_BIT_WIDTH = 8 * sizeof(uintmax_t);
static constexpr bool check_width(unsigned width)
{
    return width > 0 && width < MAX_BIT_WIDTH;
}

static void assert_same_width(const VarInt::VarInt& a, const VarInt::VarInt& b)
{
    if (a.width() != b.width())
        throw std::invalid_argument("");
}


// provide intmax_t/uintmax_t overloads for arithmetic operations? //

namespace VarInt
{
    /*
     * A variable integer class. The integer is treated as a sequence of bits,
     * so there is no notion of signed-ness.
     */
    class VarInt
    {
        public:
            VarInt::VarInt(uintmax_t val, unsigned width)
            {
                if (!check_width(width))
                    throw std::invalid_argument("Bit width is zero or larger than max suppported.");
                
                this->_val = val;
                this->_width = width;
                this->_overflowed = false;          
            }

            VarInt(intmax_t val, unsigned width)
            {
                if (!check_width(width))
                    throw std::invalid_argument("Bit width is zero or larger than max suppported.");

                // check width //
                if (val >= (1 << width - 1))
                    throw std::invalid_argument("Value is larger than what specified width can hold!");

                this->_val = val;
                this->_width = width;
                this->_overflowed = false;
            }

            VarInt(const std::string& str)
            {
                throw std::exception("unimplemented");
                // convert verilog integer literal syntax to object
            }

            VarInt(const VarInt& other) noexcept 
            {
                set_value(other);
            }

            ~VarInt() noexcept
            {
            }

            /* Getters */
            std::vector<bool> value() const noexcept
            {
                std::vector<bool> bitset;
                bitset.reserve(_width);

                for (unsigned i = 0; i < _width; i++)
                    bitset[i] = is_set(i);
                return bitset;
            }

            uintmax_t to_unsigned() const noexcept
            {
                return _val;
            }

            operator uintmax_t() const
            {
                return to_unsigned();
            }

            intmax_t to_signed() const noexcept
            {
                return static_cast<intmax_t>(_val | (~0 << (_width - 1)));
            }

            operator intmax_t() const
            {
                return to_signed();
            }

            unsigned width() const noexcept
            {
                return _width;
            }

            bool overflowed() const noexcept
            {
                return _overflowed;
            }

            /* Setters */
            void set_value(const VarInt& other) noexcept
            {
                this->_val = other._val;
                this->_width = other._width;
                this->_overflowed = other._overflowed;
            }

            VarInt operator + (const VarInt& other) const noexcept
            {
                VarInt ret = *this;
                ret += other;
                return ret;
            }

            VarInt& operator += (const VarInt& other) noexcept
            {
                this->_val += other._val;
                this->check_and_set_overflow();
                return *this;
            }

            VarInt operator - (const VarInt& other) noexcept
            {
                VarInt ret = *this;
                ret -= other;
                return ret;
            }

            VarInt& operator -= (const VarInt& other) noexcept
            {
                this->_val -= other._val;
                this->check_and_set_overflow();
                return *this;
            }

            VarInt operator * (const VarInt& other) noexcept
            {
            }

            VarInt& operator *= (const VarInt& other) noexcept
            {

            }

            VarInt operator / (const VarInt& other) const
            {

            }

            VarInt& operator /= (const VarInt& other) noexcept
            {
            }

            VarInt operator % (const VarInt& other) noexcept
            {

            }

            VarInt& operator %= (const VarInt& other) noexcept
            {

            }

            VarInt& operator ++ () noexcept
            {
                this->_val++;
                this->check_and_set_overflow();
                return *this;
            }

            VarInt operator ++ (int) noexcept
            {
                VarInt ret = *this;
                ++(*this);
                return ret;
            }

            VarInt& operator -- () noexcept
            {
                this->_val--;
                this->check_and_set_overflow();
                return *this;
            }

            VarInt operator -- (int) noexcept
            {
                VarInt ret = *this;
                --(*this);
                return ret;
            }

            VarInt operator & (const VarInt& other) const
            {
                assert_same_width(*this, other);
                VarInt ret = *this;
                ret._val = this->_val & other._val;
                return ret;
            }

            VarInt operator & (const std::string& str) const
            {
                throw std::exception("bit concat unimplemented");
            }

            VarInt& operator &= (const VarInt& other)
            {
                assert_same_width(*this, other);
                this->_val &= other._val;
                return *this;
            }

            VarInt operator | (const VarInt& other) const
            {
                assert_same_width(*this, other);
                VarInt ret = *this;
                ret._val = this->_val | other._val;
                return ret;
            }

            VarInt& operator |= (const VarInt& other)
            {
                assert_same_width(*this, other);
                this->_val |= other._val;
                return *this;
            }

            VarInt operator ~ () const noexcept
            {
                VarInt ret = *this;
                ret._val = ~ret._val;
                return ret;
            }

            VarInt operator ^ (const VarInt& other) const
            {
                assert_same_width(*this, other);
                VarInt ret = *this;
                ret._val = this->_val ^ other._val;
                return ret;
            }

            VarInt& operator ^= (const VarInt& other)
            {
                assert_same_width(*this, other);
                this->_val ^= other._val;
                return *this;
            }

            VarInt operator << (unsigned bits) const noexcept
            {
                VarInt ret = *this;
                ret._val <<= bits;
                ret._val &= (~0 << ret._width);
                return ret;
            }

            VarInt& operator <<= (unsigned bits) noexcept
            {
                this->_val <<= bits;
                this->_val &= (~0 << this->_width);
                return *this;
            }

            VarInt arith_rshift(unsigned bits)
            {
                VarInt ret = *this;
                ret._val >>= bits;
                return ret;
            }

            VarInt logical_rshift(unsigned bits)
            {
                VarInt ret = *this;
                bool sext_required = ret[ret.width() - 1];
                ret._val >>= bits;
                
                // generate mask for sext
                uintmax_t mask = ((~0) >> (_width - bits)) << (_width - bits);
                mask &= ~((~0) << _width);
                ret._val &= mask;
                return ret;
            }

            bool is_set(unsigned bit_index) const noexcept
            {
                return (_val >> bit_index) & 0x01;
            }

            const bool operator [] (const unsigned index)
            {
                if (index >= _width)
                    throw std::invalid_argument("Invalid index!");
                return is_set(index);
            }

            /* Misc functions */
            std::string to_binary_str() const noexcept
            {
                std::string bstr;
                bstr.append(std::to_string(_width));
                bstr.append("'b");
                for (unsigned i = 0; i < _width; i++)
                    bstr.append(is_set(_width - i - 1) ? "1" : "0");
                return bstr;                    
            }

            std::string to_hex_str() const noexcept
            {
                static const std::string hex_digits("0123456789ABCDEF");
                std::string hstr;

                // if length mismatch, pad //
                unsigned hex_width = (_width % 4 == 0) ? _width : _width + (4 - _width);
                hstr.append(std::to_string(hex_width));
                hstr.append("'h");

                for (unsigned i = 0; i < hex_width; i += 4)
                {
                    unsigned hex_digit = (_val >> (hex_width - i - 4)) & 0x0F;
                    hstr.append(std::string(1, hex_digits[hex_digit]));
                }

                return hstr;
            }

            std::string to_decimal_str(bool is_signed) const noexcept
            {
                return (is_signed) ? std::to_string(this->to_signed()) : std::to_string(to_unsigned());
            }

            VarInt sext(unsigned new_width) const
            {
                if (!check_width(new_width) || new_width < _width)
                    throw std::invalid_argument("Invalid width!");
                else if (new_width == _width)
                    return *this;

                // test MSB of current //
                VarInt ret = *this;
                ret._val |= (~0 << _width);
                ret._width = new_width;
                return ret;
            }

            VarInt usext(unsigned new_width) const
            {
                if (!check_width(new_width) || new_width < _width)
                    throw std::invalid_argument("Invalid width!");
                else if (new_width == _width)
                    return *this;

                // usext //
                VarInt ret = *this;
                ret._width = new_width;
                return ret;
            }

            VarInt truncate(unsigned new_width)
            {
                if (!check_width(new_width) || new_width > _width)
                    throw std::invalid_argument("Invalid width!");
                else if (new_width == _width)
                    return *this;

                // truncate //
                VarInt ret = *this;
                ret._val &= ~((~0) << new_width);
                ret._width = new_width;
            }

            VarInt slice(unsigned start, unsigned end) const
            {
                // inclusive/exclusive end?
            }

            VarInt slice(unsigned start) const
            {

            }

        private:
            uintmax_t _val;
            unsigned _width;
            bool _overflowed;

            void check_and_set_overflow() noexcept
            {

            }
    };
}
