#include "VarInt.hpp"
#include <string>

using varint::VarInt;

int main()
{
    VarInt a("8'b1000_0001");
    auto b = a.to_binary_str();
    a &= "1000";
    auto c = a.to_binary_str();

    return 0;
    // division/remainder is broken - needs to know sign //
}