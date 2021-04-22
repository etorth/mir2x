/*
 * =====================================================================================
 *
 *       Filename: scopedalloc.h
 *        Created: 06/10/2020 12:53:04
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: g++ -std=c++14
 *
 *         Author: ANHONG
 *          Email:
 *   Organization:
 *
 * =====================================================================================
 */

#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include "fflerror.h"

#define SCOPED_ALLOC_THROW_OVERLIVE
#define SCOPED_ALLOC_SUPPORT_OVERALIGN

// disable dynamic_arena realloc if it aready has buffer attached
// see more details in the function
#define SCOPED_ALLOC_DISABLE_DYNAMIC_ARENA_REALLOC

// use posix_memalign()
// aligned_alloc is not standardized for compiler with c++14
#define SCOPED_ALLOC_USE_POSIX_MEMALIGN

namespace scoped_alloc
{
    constexpr bool is_power2(size_t n)
    {
        return (n > 0) && ((n & (n - 1)) == 0);
    }

    constexpr bool check_alignment(size_t alignment)
    {
        if(!is_power2(alignment)){
            return false;
        }

        if(alignment <= alignof(std::max_align_t)){
            return true;
        }

#ifdef SCOPED_ALLOC_SUPPORT_OVERALIGN
        return alignment % sizeof(void *) == 0;
#else
        return false;
#endif
    }

    template<size_t Alignment> size_t aligned_size(size_t byte_count) noexcept
    {
        static_assert(check_alignment(Alignment), "bad alignment");
        return (byte_count + (Alignment - 1)) & ~(Alignment - 1);
    }

    template<size_t Alignment> struct aligned_buf
    {
        // buffer delegation
        // this class won't take care of ownship

        constexpr static size_t alignment = Alignment;
        static_assert(check_alignment(Alignment), "bad alignment");

        char * const buf  = nullptr;
        const size_t size = 0;
    };

#ifdef SCOPED_ALLOC_SUPPORT_OVERALIGN
    template<size_t Alignment> aligned_buf<Alignment> alloc_overalign(size_t byte_count)
    {
        void *aligned_ptr = nullptr;
        const auto byte_count_aligned = scoped_alloc::aligned_size<Alignment>(byte_count);

        // always use aligned size
        // aligned_alloc() requires this but posix_memalign() doesn't

#ifdef SCOPED_ALLOC_USE_POSIX_MEMALIGN
        if(!posix_memalign(&aligned_ptr, Alignment, byte_count_aligned)){
            return {static_cast<char *>(aligned_ptr), byte_count_aligned};
        }
        throw fflerror("posix_memalign(..., alignment = %zu, byte_count = %zu, byte_count_aligned = %zu) failed", Alignment, byte_count, byte_count_aligned);
#else
        aligned_ptr = aligned_alloc(Alignment, byte_count_aligned);
        if(aligned_ptr){
            return {static_cast<char *>(aligned_ptr), byte_count_aligned};
        }
        throw fflerror("aligned_alloc(alignment = %zu, byte_count = %zu, byte_count_aligned = %zu) failed", Alignment, byte_count, byte_count_aligned);
#endif
    }

    inline void free_overalign(char *p)
    {
        free(p);
    }
#endif

    template<size_t Alignment> aligned_buf<Alignment> alloc_aligned(size_t byte_count)
    {
        if(byte_count == 0){
            throw fflerror("bad argument: byte_count = 0");
        }

        // NOTICE: we either use alloc_overalign() or use the general operator new()
        // but can only support one method

        // that means if over-alignment is supported by SCOPED_ALLOC_SUPPORT_OVERALIGN
        // then we always use alloc_overalign() even for allocation with Alignment <= alignof(std::max_align_t)

        // reason is: we allow aligned_buf with higher alignment assigns to aligned_buf with lower alignment
        // after several rounds of assignment we can't know the original alignment

#ifdef SCOPED_ALLOC_SUPPORT_OVERALIGN
        return scoped_alloc::alloc_overalign<Alignment>(byte_count);
#else
        static_assert(Alignment <= alignof(std::max_align_t), "over aligned allcation is not guaranteed");
        return {static_cast<char*>(::operator new(byte_count)), byte_count};
#endif
    }

    inline void free_aligned(char *p)
    {
        if(p){
#ifdef SCOPED_ALLOC_SUPPORT_OVERALIGN
            scoped_alloc::free_overalign(p);
#else
            ::operator delete(p);
#endif
        }
    }

    template<size_t Alignment> class dynamic_buf
    {
        // helper class with ownship
        // use this class to implement recyclable buffers with arena_interf

        private:
            static_assert(check_alignment(Alignment), "bad alignment");

        public:
            constexpr static size_t alignment = Alignment;

        private:
            char  *m_buf  = nullptr;
            size_t m_size = 0;

        public:
            // TODO: support swap/r-ref
            //       not implemented for now

            dynamic_buf(dynamic_buf &&) = delete;
            dynamic_buf(const dynamic_buf &) = delete;
            dynamic_buf &operator = (dynamic_buf &&) = delete;
            dynamic_buf &operator = (const dynamic_buf &) = delete;

        public:
            explicit dynamic_buf(size_t byte_count)
            {
                const auto buf = scoped_alloc::alloc_aligned<Alignment>(byte_count);

                m_buf  = buf.buf;
                m_size = buf.size;

                if(!(m_buf && m_size)){
                    throw fflerror("failed to allocate dynamic_buf");
                }
            }

            ~dynamic_buf()
            {
                if(m_buf){
                    scoped_alloc::free_aligned(m_buf);
                }

                m_buf  = nullptr;
                m_size = 0;
            }

        public:
            scoped_alloc::aligned_buf<Alignment> get_buf() const
            {
                return {m_buf, m_size};
            }
    };

    template<size_t Alignment = alignof(std::max_align_t)> class arena_interf
    {
        private:
            static_assert(check_alignment(Alignment), "bad alignment");

        private:
            char  *m_buf  = nullptr;
            size_t m_size = 0;

        private:
            char *m_cursor = nullptr;

        protected:
            template<size_t BufAlignment> void set_buf(const aligned_buf<BufAlignment> &buf)
            {
                static_assert(check_alignment(BufAlignment) && (BufAlignment >= Alignment), "bad aligned_buf alignment");
                if(!(buf.buf && buf.size)){
                    throw fflerror("empty aligned_buf");
                }

                m_cursor = buf.buf;
                m_buf    = buf.buf;
                m_size   = buf.size;
            }

        public:
            bool has_buf() const
            {
                return m_buf && m_size;
            }

        public:
            aligned_buf<Alignment> get_buf() const
            {
                return {m_buf, m_size};
            }

        public:
            aligned_buf<Alignment> get_buf_ex() const
            {
                if(!has_buf()){
                    throw fflerror("no valid buffer attached to arena_interf");
                }
                return get_buf();
            }

        public:
            arena_interf() = default;

        public:
            template<size_t BufAlignment> arena_interf(const scoped_alloc::dynamic_buf<BufAlignment> &buf)
            {
                set_buf(buf.get_buf());
            }

        public:
            arena_interf(arena_interf &&) = delete;
            arena_interf(const arena_interf &) = delete;
            arena_interf &operator = (arena_interf &&) = delete;
            arena_interf &operator = (const arena_interf &) = delete;

        public:
            virtual ~arena_interf()
            {
                m_cursor = nullptr;
                m_buf    = nullptr;
                m_size   = 0;
            }

        public:
            size_t used() const
            {
                return static_cast<size_t>(m_cursor - get_buf_ex().buf);
            }

            float usage() const
            {
                const auto buf = get_buf_ex();
                return (static_cast<size_t>(m_cursor - buf.buf) * 1.0f) / buf.size;
            }

            void reset()
            {
                m_cursor = get_buf_ex().buf;
            }

        protected:
            bool in_buf(char *p) const
            {
                const auto buf = get_buf_ex();
                return buf.buf <= p && p <= buf.buf + buf.size;
            }

        public:
            template<size_t RequestAlignment> char *allocate(size_t byte_count)
            {
                static_assert(check_alignment(RequestAlignment) && (RequestAlignment <= Alignment), "bad requested alignment");
                detect_outlive();

                // don't throw
                // implementation defined

                if(byte_count == 0){
                    return nullptr;
                }

                const auto buf = get_buf_ex();
                const auto byte_count_aligned = scoped_alloc::aligned_size<Alignment>(byte_count);

                if(static_cast<decltype(byte_count_aligned)>(buf.buf + buf.size - m_cursor) >= byte_count_aligned){
                    auto r = m_cursor;
                    m_cursor += byte_count_aligned;
                    return r;
                }

                // TODO normal operator new() only guarantees alignemnt <= alignof(std::max_align_t)
                //      but class user may ask for over-aligned memory, how to handle this?
                //
                //     1. raise compilation error if asks for over-alignment
                //     2. here I try posix_memalign()/aligned_alloc(), this may waste memory, or other issue?
                //

                return dynamic_alloc(byte_count);
            }

        public:
            void deallocate(char *p, size_t byte_count) noexcept
            {
                detect_outlive();

                if(in_buf(p)){
                    if(p + scoped_alloc::aligned_size<Alignment>(byte_count) == m_cursor){
                        m_cursor = p;
                    }

                    else{
                        // TODO: main draw-back
                        // can't recycle memory here without extra flags
                    }
                }

                else{
                    scoped_alloc::free_aligned(p);
                }
            }

        public:
            virtual char *dynamic_alloc(size_t byte_count)
            {
                return scoped_alloc::alloc_aligned<Alignment>(byte_count).buf;
            }

        private:
            void detect_outlive() const
            {
                // TODO: best-effort
                //       detect_outlive() invokes undefined behavior if assertion failed
#ifdef SCOPED_ALLOC_THROW_OVERLIVE
                if(!(in_buf(m_cursor))){
                    throw fflerror("allocator has outlived arena_interf");
                }
#else
                assert(in_buf(m_cursor) && "allocator has outlived arena_interf");
#endif
            }
    };

    template<size_t ByteCount, size_t Alignment = alignof(std::max_align_t)> class fixed_arena: public scoped_alloc::arena_interf<Alignment>
    {
        private:
            alignas(Alignment) char m_storage[ByteCount];

        public:
            fixed_arena(): scoped_alloc::arena_interf<Alignment>()
            {
                this->set_buf(scoped_alloc::aligned_buf<Alignment>{m_storage, ByteCount});
            }
    };

    template<size_t Alignment = alignof(std::max_align_t)> class dynamic_arena: public scoped_alloc::arena_interf<Alignment>
    {
        public:
            dynamic_arena(size_t byte_count = 0): scoped_alloc::arena_interf<Alignment>()
            {
                if(!byte_count){
                    return;
                }
                alloc(byte_count);
            }

            template<size_t BufAlignment> dynamic_arena(const scoped_alloc::aligned_buf<BufAlignment> &buf)
                : scoped_alloc::arena_interf<Alignment>()
            {
                set_buf(buf);
            }

        public:
            ~dynamic_arena() override
            {
                if(this->has_buf()){
                    scoped_alloc::free_aligned(this->get_buf().buf);
                }
            }

        public:
            // TODO: should I disable this if current arena already has buffer allcoated?
            //       because this can be the root of all bugs!
            //
            //     scoped_alloc::dynamic_arena<8> d;
            //     std::vector<int, scoped_alloc::arena_interf<8>> v1(d);
            //
            //     d.alloc(128);    // OK
            //     v1.push_back(1);
            //     v1.push_back(2);
            //     ...
            //     v1.clear();
            //
            //     std::vector<int, scoped_alloc::arena_interf<8>> v2(d);
            //
            //     d.alloc(512);    // ERROR!
            //     v2.push_back(1);
            //     v1.push_back(2);
            //     ...
            //
            // d.alloc(512) frees its internal buffer previously allocated by d.alloc(128)
            // however v1 still refers to this buffer
            //
            // following code is OK:
            //
            //     scoped_alloc::dynamic_arena<8> d;
            //     {
            //         std::vector<int, scoped_alloc::arena_interf<8>> v1(d);
            //
            //         d.alloc(128);    // OK
            //         v1.push_back(1);
            //         v1.push_back(2);
            //         ...
            //         v1.clear();
            //     }
            //
            //     d.alloc(512);
            //     std::vector<int, scoped_alloc::arena_interf<8>> v2(d);
            //
            //     v2.push_back(1);
            //     ...
            //
            // before re-alloc, caller needs to confirm no other object refers to its buffer
            // this is hard, sometimes even the standard doesn't guarantee:
            //
            //     std::vector<T>::shrink_to_fit()
            //
            // this doesn't guerantee the vector release the buffer
            // solution:
            //
            //     1. trust caller, don't do anything
            //     2. only allow realloc if arena_interf::used() returns 0, this may overkill
            //     3. forbid any re-alloc
            //
            void alloc(size_t byte_count)
            {
                if(byte_count == 0){
                    throw fflerror("invalid allocation: byte_count = 0");
                }

                if(this->has_buf()){
#ifdef SCOPED_ALLOC_DISABLE_DYNAMIC_ARENA_REALLOC
                    throw fflerror("dynamic_arena has buffer attached");
#else
                    scoped_alloc::free_aligned(this->get_buf().buf);
#endif
                }
                this->set_buf(scoped_alloc::alloc_aligned<Alignment>(byte_count));
            }
    };

    template<class T, size_t Alignment = alignof(std::max_align_t)> class allocator
    {
        public:
            using value_type = T;
            template <class U, size_t A> friend class scoped_alloc::allocator;

        public:
            template <class UpperType> struct rebind
            {
                using other = allocator<UpperType, Alignment>;
            };

        private:
            arena_interf<Alignment> &m_arena;

        public:
            allocator(const allocator &) = default;
            allocator &operator=(const allocator &) = delete;

        public:
            allocator(arena_interf<Alignment> &arena_ref) noexcept
                : m_arena(arena_ref)
            {}

        public:
            template<class U> allocator(const allocator<U, Alignment>& alloc_ref) noexcept
                : m_arena(alloc_ref.m_arena)
            {}

        public:
            T* allocate(size_t n)
            {
                return reinterpret_cast<T *>(m_arena.template allocate<alignof(T)>(n * sizeof(T)));
            }

            void deallocate(T *p, size_t n) noexcept
            {
                m_arena.deallocate(reinterpret_cast<char *>(p), n * sizeof(T));
            }

        public:
            template<typename T2, size_t A2> bool operator == (const scoped_alloc::allocator<T2, A2> &parm) noexcept
            {
                return Alignment == A2 && &(this->m_arena) == &(parm.m_arena);
            }

            template<typename T2, size_t A2> bool operator != (const scoped_alloc::allocator<T2, A2> &parm) noexcept
            {
                return !(*this == parm);
            }
    };

    template<typename Key, typename Value, size_t Alignment = alignof(std::max_align_t), typename KeyHash = std::hash<Key>, typename KeyEq = std::equal_to<Key>> class hash_wrapper
    {
        private:
            scoped_alloc::dynamic_arena<Alignment> m_arena;

        public:
            std::unordered_map<Key, Value, KeyHash, KeyEq, scoped_alloc::allocator<std::pair<const Key, Value>, Alignment>> c;

        public:
            hash_wrapper(): c(typename decltype(c)::allocator_type(m_arena)) {}

        public:
            hash_wrapper(size_t n): hash_wrapper()
            {
                alloc(n);
            }

        public:
            void alloc(size_t n)
            {
                if(m_arena.has_buf()){
                    throw fflerror("arena buffer has already been allocated");
                }

                // estimate memory usage
                // only work well for std::unordered_map gcc libstdc++-v3

                const size_t m = [n]()
                {
                    constexpr static unsigned long long prime_table[]
                    {
                        2ull,
                        3ull,
                        5ull,
                        7ull,
                        11ull,
                        13ull,
                        17ull,
                        19ull,
                        23ull,
                        29ull,
                        31ull,
                        37ull,
                        41ull,
                        43ull,
                        47ull,
                        53ull,
                        59ull,
                        61ull,
                        67ull,
                        71ull,
                        73ull,
                        79ull,
                        83ull,
                        89ull,
                        97ull,
                        103ull,
                        109ull,
                        113ull,
                        127ull,
                        137ull,
                        139ull,
                        149ull,
                        157ull,
                        167ull,
                        179ull,
                        193ull,
                        199ull,
                        211ull,
                        227ull,
                        241ull,
                        257ull,
                        277ull,
                        293ull,
                        313ull,
                        337ull,
                        359ull,
                        383ull,
                        409ull,
                        439ull,
                        467ull,
                        503ull,
                        541ull,
                        577ull,
                        619ull,
                        661ull,
                        709ull,
                        761ull,
                        823ull,
                        887ull,
                        953ull,
                        1031ull,
                        1109ull,
                        1193ull,
                        1289ull,
                        1381ull,
                        1493ull,
                        1613ull,
                        1741ull,
                        1879ull,
                        2029ull,
                        2179ull,
                        2357ull,
                        2549ull,
                        2753ull,
                        2971ull,
                        3209ull,
                        3469ull,
                        3739ull,
                        4027ull,
                        4349ull,
                        4703ull,
                        5087ull,
                        5503ull,
                        5953ull,
                        6427ull,
                        6949ull,
                        7517ull,
                        8123ull,
                        8783ull,
                        9497ull,
                        10273ull,
                        11113ull,
                        12011ull,
                        12983ull,
                        14033ull,
                        15173ull,
                        16411ull,
                        17749ull,
                        19183ull,
                        20753ull,
                        22447ull,
                        24281ull,
                        26267ull,
                        28411ull,
                        30727ull,
                        33223ull,
                        35933ull,
                        38873ull,
                        42043ull,
                        45481ull,
                        49201ull,
                        53201ull,
                        57557ull,
                        62233ull,
                        67307ull,
                        72817ull,
                        78779ull,
                        85229ull,
                        92203ull,
                        99733ull,
                        107897ull,
                        116731ull,
                        126271ull,
                        136607ull,
                        147793ull,
                        159871ull,
                        172933ull,
                        187091ull,
                        202409ull,
                        218971ull,
                        236897ull,
                        256279ull,
                        277261ull,
                        299951ull,
                        324503ull,
                        351061ull,
                        379787ull,
                        410857ull,
                        444487ull,
                        480881ull,
                        520241ull,
                        562841ull,
                        608903ull,
                        658753ull,
                        712697ull,
                        771049ull,
                        834181ull,
                        902483ull,
                        976369ull,
                        1056323ull,
                        1142821ull,
                        1236397ull,
                        1337629ull,
                        1447153ull,
                        1565659ull,
                        1693859ull,
                        1832561ull,
                        1982627ull,
                        2144977ull,
                        2320627ull,
                        2510653ull,
                        2716249ull,
                        2938679ull,
                        3179303ull,
                        3439651ull,
                        3721303ull,
                        4026031ull,
                        4355707ull,
                        4712381ull,
                        5098259ull,
                        5515729ull,
                        5967347ull,
                        6456007ull,
                        6984629ull,
                        7556579ull,
                        8175383ull,
                        8844859ull,
                        9569143ull,
                        10352717ull,
                        11200489ull,
                        12117689ull,
                        13109983ull,
                        14183539ull,
                        15345007ull,
                        16601593ull,
                        17961079ull,
                        19431899ull,
                        21023161ull,
                        22744717ull,
                        24607243ull,
                        26622317ull,
                        28802401ull,
                        31160981ull,
                        33712729ull,
                        36473443ull,
                        39460231ull,
                        42691603ull,
                        46187573ull,
                        49969847ull,
                        54061849ull,
                        58488943ull,
                        63278561ull,
                        68460391ull,
                        74066549ull,
                        80131819ull,
                        86693767ull,
                        93793069ull,
                        101473717ull,
                        109783337ull,
                        118773397ull,
                        128499677ull,
                        139022417ull,
                        150406843ull,
                        162723577ull,
                        176048909ull,
                        190465427ull,
                        206062531ull,
                        222936881ull,
                        241193053ull,
                        260944219ull,
                        282312799ull,
                        305431229ull,
                        330442829ull,
                        357502601ull,
                        386778277ull,
                        418451333ull,
                        452718089ull,
                        489790921ull,
                        529899637ull,
                        573292817ull,
                        620239453ull,
                        671030513ull,
                        725980837ull,
                        785430967ull,
                        849749479ull,
                        919334987ull,
                        994618837ull,
                        1076067617ull,
                        1164186217ull,
                        1259520799ull,
                        1362662261ull,
                        1474249943ull,
                        1594975441ull,
                        1725587117ull,
                        1866894511ull,
                        2019773507ull,
                        2185171673ull,
                        2364114217ull,
                        2557710269ull,
                        2767159799ull,
                        2993761039ull,
                        3238918481ull,
                        3504151727ull,
                        3791104843ull,
                        4101556399ull,
                        4294967291ull,
                        6442450933ull,
                        8589934583ull,
                        12884901857ull,
                        17179869143ull,
                        25769803693ull,
                        34359738337ull,
                        51539607367ull,
                        68719476731ull,
                        103079215087ull,
                        137438953447ull,
                        206158430123ull,
                        274877906899ull,
                        412316860387ull,
                        549755813881ull,
                        824633720731ull,
                        1099511627689ull,
                        1649267441579ull,
                        2199023255531ull,
                        3298534883309ull,
                        4398046511093ull,
                        6597069766607ull,
                        8796093022151ull,
                        13194139533241ull,
                        17592186044399ull,
                        26388279066581ull,
                        35184372088777ull,
                        52776558133177ull,
                        70368744177643ull,
                        105553116266399ull,
                        140737488355213ull,
                        211106232532861ull,
                        281474976710597ull,
                        562949953421231ull,
                        1125899906842597ull,
                        2251799813685119ull,
                        4503599627370449ull,
                        9007199254740881ull,
                        18014398509481951ull,
                        36028797018963913ull,
                        72057594037927931ull,
                        144115188075855859ull,
                        288230376151711717ull,
                        576460752303423433ull,
                        1152921504606846883ull,
                        2305843009213693951ull,
                        4611686018427387847ull,
                        9223372036854775783ull,
                        18446744073709551557ull,
                    };

                    const auto prime_table_end = prime_table + std::extent_v<decltype(prime_table)>;
                    const auto found = std::lower_bound(prime_table, prime_table_end, (unsigned long long)(n));

                    if(found == prime_table_end){
                        throw fflerror("invalid size to reserve: %zu", n);
                    }
                    return (size_t)(*found);
                }();

                const static auto extra_mem = []() -> int
                {
                    if(const auto envstr = std::getenv("SCOPED_ALLOC_HASH_WRAPPER_EXTRA_MEM")){
                        return std::stoi(envstr);
                    }
                    return 10;
                }();

                // estimate buffer to use with default load factor
                //     s1: bucket pointer buffer
                //     s2: the key-value pair memory usage, with the next pointer
                //     s3: used as margin, and also when n = 0 the unordered_map still needs memory
                // this s1/s2 estimation highly depends on the internal implementation of std::unordered_map

                const size_t s1 = m * scoped_alloc::aligned_size<Alignment>(sizeof(void *));
                const size_t s2 = n * scoped_alloc::aligned_size<Alignment>(sizeof(void *) + sizeof(typename decltype(c)::value_type));
                const size_t s3 = 4096;

                m_arena.alloc((s1 + s2 + s3) * (1.0f + extra_mem / 100.0f));
                c.reserve(n);
            }

        public:
            float usage() const
            {
                return m_arena.usage();
            }

            size_t used() const
            {
                return m_arena.used();
            }

            size_t buf_size() const
            {
                return m_arena.get_buf().size;
            }
    };

    template<typename T, size_t BufSize> class svobuf_wrapper
    {
        private:
            scoped_alloc::fixed_arena<BufSize * sizeof(T), alignof(T)> m_arena;

        public:
            std::vector<T, scoped_alloc::allocator<T, alignof(T)>> c;

        public:
            svobuf_wrapper(): c(typename decltype(c)::allocator_type(m_arena))
            {
                // immediately use all static buffer
                // 1. to prevent push_back() to allocate on heap, best effort
                // 2. to prevent waste of memory
                //
                //     svobuf_wrapper<int, 4> buf;
                //     auto buf_cp = buf.c;
                //
                //     buf_cp.push_back(1);
                //     buf_cp.push_back(2);
                //
                // here buf has ran out of all static buffer
                // the copy constructed buf_cp will always allocates memory on heap
                //
                // ill-formed code:
                //
                //    auto f()
                //    {
                //        svobuf_wrapper<int, 4> buf;
                //
                //        buf.c.push_back(0);
                //        buf.c.push_back(1);
                //        ...
                //
                //        return buf.c;
                //    }
                //
                // return a copy of buf.c also copies its scoped allocator
                // this causes dangling reference

                c.reserve(BufSize);
                if(c.capacity() > BufSize){
                    throw fflerror("allocate initial buffer dynamically");
                }
            }

        public:
            constexpr size_t svocap() const
            {
                return BufSize;
            }
    };
}
