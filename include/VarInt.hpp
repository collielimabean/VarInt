#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <string>
#include <regex>

static constexpr int MAX_BIT_WIDTH = 8 * sizeof(uintmax_t);
static constexpr bool check_width(unsigned width)
{
    return width > 0 && width < MAX_BIT_WIDTH;
}

static constexpr char *INTEGER_LITERAL_REGEX = "(\\d+)'([bohd])(.*)";

namespace varint
{
    /*
     * A variable integer class. The integer is treated as a sequence of bits,
     * so there is no notion of signed-ness.
     */
    class VarInt
    {
        public:
            static uintmax_t max_unsigned(unsigned width) noexcept
            {
                return width ? (1 << width) - 1 : 0;
            }

            static uintmax_t min_unsigned(unsigned width) noexcept
            {
                return 0;
            }

            static intmax_t max_signed(unsigned width) noexcept
            {
                return width ? (1 << (width - 1)) - 1 : 0;
            }

            static intmax_t min_signed(unsigned width) noexcept
            {
                return width ? (~0) << (width - 1) : 0;
            }

            VarInt::VarInt(uintmax_t val, unsigned width)
            {
                if (!check_width(width))
                    throw std::invalid_argument("Bit width is zero or larger than max suppported.");
                
                // check width //
                if (val > VarInt::max_unsigned(width))
                    throw std::invalid_argument("Value is larger than what specified width can hold!");

                // truncate to length //
                this->_val = val;
                this->_width = width;
            }

            VarInt(intmax_t val, unsigned width)
            {
                if (!check_width(width))
                    throw std::invalid_argument("Bit width is zero or larger than max suppported.");

                // check width //
                if (val > VarInt::max_signed(width) || val < VarInt::min_signed(width))
                    throw std::invalid_argument("Value is larger than what specified width can hold!");

                this->_val = val;
                this->_width = width;
                this->_val &= ~((~0) << _width);
            }

            VarInt(const std::string& str)
            {
                std::regex literal_regex(INTEGER_LITERAL_REGEX);
                std::smatch match;
                if (!std::regex_match(str, match, literal_regex))
                    throw std::invalid_argument("Failed to parse literal!");

                if (match.size() != 4)
                    throw std::invalid_argument("Failed to parse literal!");

                this->_width = std::stoi(match[1]);

                unsigned base;
                switch (match[2].str()[0])
                {
                    case 'b':
                        base = 2;
                        break;
                    case 'd':
                        base = 10;
                        break;
                    case 'h':
                        base = 16;
                        break;
                    case 'o':
                        base = 8;
                        break;
                    default:
                        throw std::invalid_argument("Unrecognized base detected!");
                }

                // replace "_", a valid separator in literals
                std::string valstr;
                for (auto& c : match[3].str())
                    if (c != '_')
                        valstr.append(1, c);

                // add'l verification //
                if (base == 2 && valstr.length() != _width)
                    throw std::invalid_argument("Bad binary string supplied!");

                if (base == 8 && valstr.length() * 3 != _width)
                    throw std::invalid_argument("Bad octal string supplied!");

                if (base == 16 && valstr.length() * 4 != _width)
                    throw std::invalid_argument("Bad hex string supplied!");

                // parse, then check //
                this->_val = std::stoull(valstr, 0, base);
                if (base == 10 && _val > VarInt::max_unsigned(_width))
                    throw std::invalid_argument("Decimal value is larger than max value possible by width!");
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
                return static_cast<intmax_t>(_val);
            }

            operator intmax_t() const
            {
                return to_signed();
            }

            unsigned width() const noexcept
            {
                return _width;
            }

            /* Setters */
            void set_value(const VarInt& other) noexcept
            {
                this->_val = other._val;
                this->_width = other._width;
            }

            /* Arithmetic functions */
            VarInt operator + (const VarInt& other) const noexcept
            {
                VarInt ret = *this;
                ret += other;
                return ret;
            }

            VarInt& operator += (const VarInt& other) noexcept
            {
                this->_val += other._val;
                this->clear_out_of_range_bits();
                return *this;
            }

            VarInt operator - (const VarInt& other) const noexcept
            {
                VarInt ret = *this;
                ret -= other;
                return ret;
            }

            VarInt& operator -= (const VarInt& other) noexcept
            {
                this->_val -= other._val;
                this->clear_out_of_range_bits();
                return *this;
            }

            VarInt operator * (const VarInt& other) const noexcept
            {
                VarInt ret = *this;
                ret *= other;
                return ret;
            }

            VarInt& operator *= (const VarInt& other) noexcept
            {
                this->_val *= other._val;
                this->clear_out_of_range_bits();
                return *this;
            }

            VarInt operator / (const VarInt& other) const
            {
                if (other._val == 0)
                    throw std::logic_error("Attempted division by zero!");
                
                VarInt ret = *this;
                ret /= other;
                return ret;
            }

            VarInt& operator /= (const VarInt& other) noexcept
            {
                this->_val /= other._val;
                this->clear_out_of_range_bits();
                return *this;
            }

            VarInt operator % (const VarInt& other) noexcept
            {
                VarInt ret = *this;
                ret %= other;
                return ret;
            }

            VarInt& operator %= (const VarInt& other) noexcept
            {
                this->_val %= other._val;
                this->clear_out_of_range_bits();
                return *this;
            }

            VarInt& operator ++ () noexcept
            {
                this->_val++;
                this->clear_out_of_range_bits();
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
                this->clear_out_of_range_bits();
                return *this;
            }

            VarInt operator -- (int) noexcept
            {
                VarInt ret = *this;
                --(*this);
                return ret;
            }

            /* Bitwise functions */
            VarInt operator & (const VarInt& other) const noexcept
            {
                VarInt ret = *this;
                ret._val &= other._val;
                return ret;
            }

            VarInt& operator &= (const VarInt& other) noexcept
            {
                this->_val &= other._val;
                return *this;
            }

            VarInt operator | (const VarInt& other) const noexcept
            {
                VarInt ret = *this;
                ret._val |= other._val;
                return ret;
            }

            VarInt& operator |= (const VarInt& other) noexcept
            {
                this->_val |= other._val;
                return *this;
            }

            VarInt operator ~ () const noexcept
            {
                VarInt ret = *this;
                ret._val = ~ret._val;
                return ret;
            }

            VarInt operator ^ (const VarInt& other) const noexcept
            {
                VarInt ret = *this;
                ret._val ^= this->_val;
                return ret;
            }

            VarInt& operator ^= (const VarInt& other)  noexcept
            {
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
                std::string str;
                
                str.append(std::to_string(_width));
                str.append("'d");
                if (is_signed)
                    str.append(std::to_string(this->to_signed()));
                else
                    str.append(std::to_string(to_unsigned()));
                return str;
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
                if (!check_width(start) || !check_width(end) 
                    || start > _width || end > _width || start == end)
                    throw std::invalid_argument("Bad start/end indices!");

                if (start > end)
                {
                    unsigned tmp = start;
                    start = end;
                    end = tmp;
                }

                if (start == end + 1)
                    return VarInt(uintmax_t(is_set(start) ? 1 : 0), 1);

                VarInt ret = *this;
                ret._val >>= start;
                ret._val &= ~((~0) << (end - start));
                return ret;
            }

            VarInt slice(unsigned start) const
            {
                return this->slice(start, _width);
            }

            VarInt operator & (const std::string& str) const
            {
                VarInt ret = *this;
                ret &= str;
                return *this;
            }

            VarInt& operator &= (const std::string& str)
            {
                std::string valstr;
                for (auto& c : str)
                {
                    switch (c)
                    {
                    case '0': case '1':
                        valstr.append(1, c);
                    case '_':
                        continue;
                    default:
                        throw std::invalid_argument("Bad bitstring supplied!");
                    }
                }

                if (!check_width(_width + valstr.length()))
                    throw std::invalid_argument("Concat would result in integer past supported widths!");

                _width += valstr.length();
                for (size_t i = 0; i < valstr.length(); i++)
                    _val = (_val << 1) | ((valstr[i] == '0') ? 0 : 1);

                return *this;
            }

        private:
            uintmax_t _val;
            unsigned _width;

            void clear_out_of_range_bits() noexcept
            {
                _val &= ~((~0) << _width);
            }
    };
}
