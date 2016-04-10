/*
 * =====================================================================================
 *
 *       Filename: sfinaecheck.hpp
 *        Created: 04/09/2016 03:48:41 AM
 *  Last Modified: 04/09/2016 22:31:22
 *
 *    Description: copy from wikibooks: more c++ idioms/member detector
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once

#include <type_traits> // To use 'std::integral_constant'.
// #include <iostream>    // To use 'std::cout'.
// #include <iomanip>     // To use 'std::boolalpha'.

#define GENERATE_HAS_MEMBER(member)                                                 \
                                                                                    \
template<typename T>                                                                \
class _Mir2x_Impl_HasMember_##member                                                \
{                                                                                   \
    private:                                                                        \
        using Yes = char[2];                                                        \
        using No  = char[1];                                                        \
                                                                                    \
        struct Fallback { int member; };                                            \
        struct Derived: T, Fallback {  };                                           \
                                                                                    \
        template<typename U>                                                        \
        static No& test (decltype(U::member)*);                                     \
        template<typename U>                                                        \
        static Yes& test (U*);                                                      \
                                                                                    \
    public:                                                                         \
        static constexpr bool RESULT                                                \
            = (sizeof(test<Derived>(nullptr)) == sizeof(Yes));                      \
};                                                                                  \
                                                                                    \
template<typename T> struct has_member_##member                                     \
    :public std::integral_constant<bool, _Mir2x_Impl_HasMember_##member<T>::RESULT> \
{};                                                                                 \

// GENERATE_HAS_MEMBER(att)  // Creates 'has_member_att'.
// GENERATE_HAS_MEMBER(func) // Creates 'has_member_func'.
//
// struct A
// {
//     int att;
//     void func ( double );
// };
//
// struct B
// {
//     char att[3];
//     double func ( const char* );
// };
//
// struct C : A, B { }; // It will also work with ambiguous members.
//
//
// int main ( )
// {
//     std::cout << std::boolalpha
//               << "\n" "'att' in 'C' : "
//               << has_member_att<C>::value // <type_traits>-like interface.
//               << "\n" "'func' in 'C' : "
//               << has_member_func<C>() // Implicitly convertible to 'bool'.
//               << "\n";
// }
