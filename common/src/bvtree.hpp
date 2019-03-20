/*
 * =====================================================================================
 *
 *       Filename: bvtree.hpp
 *        Created: 03/03/2019 07:17:23
 *    Description: https://github.com/bbbbbrenn/BehaviorTree.cpp
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
#include <memory>
#include <vector>
#include <variant>
#include <stdexcept>
#include <functional>
#include "strfunc.hpp"

// bvarg_ref is like a non_empty_pointer
// it can bind to differnet instance but can't never be null
class bvarg_ref
{
    private:
        struct inn_void_t
        {
            void *unused = nullptr;
        };

    private:
        // should always access inn_bvarg_t by bvarg_ref
        // make it private to prevent user declare local instance
        using inn_bvarg_t = std::variant<inn_void_t, bool, int, uint64_t, std::string>;

    private:
        std::shared_ptr<inn_bvarg_t> m_ptr;

    private:
        bvarg_ref(std::shared_ptr<inn_bvarg_t> p)
            : m_ptr(p)
        {
            if(!m_ptr){
                throw std::invalid_argument(str_fflprintf(": bvarg_ref bind to empty instance"));
            }
        }

    public:
        // make bvarg_ref always contains one instance by default
        // otherwise it's too easy to lose result in the program like:
        //
        //     bvres_t update()
        //     {
        //         return m_func(m_outref); // m_outref may not contain an instance
        //     }
        //
        //     m_func([](bvarg_ref pOutput)
        //     {
        //         pOutput.assign<int>(12);
        //         return BV_SUCCESS;
        //     }
        //
        // if m_outref is empty, pOutput will allocate memory inside the lambda
        // and then immediately release the memory, m_outref doesn't capture the result

        bvarg_ref()
            : bvarg_ref(make_bvarg<inn_void_t>())
        {}

    public:
        template<typename I, typename... T> void assign(T && ... t)
        {
            m_ptr->emplace<I>(std::forward<T>(t)...);
        }

    public:
        void copy(const bvarg_ref &r)
        {
            get() = r.get();
        }

    public:
        bvarg_ref clone() const
        {
            return bvarg_ref(std::make_shared<inn_bvarg_t>(get()));
        }

    public:
        inn_bvarg_t *get_ptr()
        {
            return m_ptr.get();
        }

        inn_bvarg_t *get_ptr() const
        {
            return m_ptr.get();
        }

        inn_bvarg_t &get()
        {
            return *(m_ptr.get());
        }

        inn_bvarg_t &get() const
        {
            return *(m_ptr.get());
        }

        template<typename T> auto as_ptr()
        {
            return std::get_if<T>(get_ptr());
        }

        template<typename T> auto as_ptr() const
        {
            return std::get_if<T>(get_ptr());
        }

        template<typename T> auto &as()
        {
            return std::get<T>(get());
        }

        template<typename T> auto &as() const
        {
            return std::get<T>(get());
        }

    public:
        template<typename I, typename... T> static bvarg_ref make_bvarg(T && ... t)
        {
            return std::make_shared<bvarg_ref::inn_bvarg_t>(std::in_place_type_t<I>(), std::forward<T>(t)...);
        }
};

template<typename I, typename... T> bvarg_ref make_bvarg(T && ... t)
{
    return bvarg_ref::make_bvarg<I>(std::forward<T>(t)...);
}

// too error-prone
// don't allow implicit conversion

enum class bvres_t
{
    ABORT,
    FAILURE,
    PENDING,
    SUCCESS,
};

constexpr bvres_t BV_ABORT   = bvres_t::ABORT;
constexpr bvres_t BV_FAILURE = bvres_t::FAILURE;
constexpr bvres_t BV_PENDING = bvres_t::PENDING;
constexpr bvres_t BV_SUCCESS = bvres_t::SUCCESS;

inline const char *bvres_cstr(bvres_t status)
{
    switch(status){
        case BV_ABORT:
            {
                return "BV_ABORT";
            }
        case BV_FAILURE:
            {
                return "BV_FAILURE";
            }
        case BV_PENDING:
            {
                return "BV_PENDING";
            }
        case BV_SUCCESS:
            {
                return "BV_SUCCESS";
            }
        default:
            {
                throw std::runtime_error(str_fflprintf(": Invalid node status: %d", status));
            }
    }
}

namespace bvtree
{
    class node: public std::enable_shared_from_this<node>
    {
        public:
            virtual void reset() {}

        public:
            virtual bvres_t update() = 0;
    };
}
using bvnode_ptr = std::shared_ptr<bvtree::node>;

namespace bvtree
{
    template<typename... T> bvnode_ptr random(T && ... t)
    {
        class node_random: public bvtree::node
        {
            private:
                std::vector<bvnode_ptr> m_nodes;

            private:
                int m_currnode;

            public:
                node_random(std::vector<bvnode_ptr> && v)
                    : m_nodes(std::move(v))
                    , m_currnode(-1)
                {}

            public:
                void reset() override
                {
                    for(auto &node: m_nodes){
                        node->reset();
                    }
                    m_currnode = -1;
                }

            public:
                bvres_t update() override
                {
                    if(m_nodes.empty()){
                        throw std::runtime_error(str_fflprintf(": No valid node"));
                    }

                    if(m_currnode < 0 || m_currnode >= (int)(m_nodes.size())){
                        m_currnode = std::rand() % (int)(m_nodes.size());
                    }

                    switch(auto status = m_nodes[m_currnode]->update()){
                        case BV_SUCCESS:
                        case BV_FAILURE:
                        case BV_PENDING:
                        case BV_ABORT:
                            {
                                return status;
                            }
                        default:
                            {
                                throw std::runtime_error(str_fflprintf(": Invalid node status: %d", status));
                            }
                    }
                }
        };
        return std::make_shared<node_random>(std::vector<bvnode_ptr>{ std::forward<T>(t)... });
    }

    template<typename... T> bvnode_ptr selector(T && ... t)
    {
        class node_selector: public bvtree::node
        {
            private:
                std::vector<bvnode_ptr> m_nodes;

            private:
                int m_currnode;

            public:
                node_selector(std::vector<bvnode_ptr> && v)
                    : m_nodes(std::move(v))
                    , m_currnode(-1)
                {}

            public:
                void reset() override
                {
                    for(auto &node: m_nodes){
                        node->reset();
                    }
                    m_currnode = 0;
                }

            public:
                bvres_t update() override
                {
                    if(m_nodes.empty()){
                        throw std::runtime_error(str_fflprintf(": No valid node"));
                    }

                    while(m_currnode < (int)(m_nodes.size())){
                        switch(auto status = m_nodes[m_currnode]->update()){
                            case BV_ABORT:
                            case BV_SUCCESS:
                            case BV_PENDING:
                                {
                                    return BV_PENDING;
                                }
                            case BV_FAILURE:
                                {
                                    m_currnode++;
                                    break;
                                }
                            default:
                                {
                                    throw std::runtime_error(str_fflprintf(": Invalid node status: %d", status));
                                }
                        }
                    }
                    return BV_FAILURE;
                }
        };
        return std::make_shared<node_selector>(std::vector<bvnode_ptr> {std::forward<T>(t)...});
    }

    template<typename... T> bvnode_ptr sequence(T && ... t)
    {
        class node_sequence: public bvtree::node
        {
            private:
                std::vector<bvnode_ptr> m_nodes;

            private:
                int m_currnode;

            public:
                node_sequence(std::vector<bvnode_ptr> && v)
                    : m_nodes(std::move(v))
                    , m_currnode(-1)
                {}

            public:
                void reset() override
                {
                    for(auto &node: m_nodes){
                        node->reset();
                    }
                    m_currnode = 0;
                }

            public:
                bvres_t update() override
                {
                    if(m_nodes.empty()){
                        throw std::runtime_error(str_fflprintf(": No valid node"));
                    }

                    while(m_currnode < (int)(m_nodes.size())){
                        switch(auto status = m_nodes[m_currnode]->update()){
                            case BV_ABORT:
                            case BV_FAILURE:
                            case BV_PENDING:
                                {
                                    return status;
                                }
                            case BV_SUCCESS:
                                {
                                    m_currnode++;
                                    break;
                                }
                            default:
                                {
                                    throw std::runtime_error(str_fflprintf(": Invalid node status: %d", status));
                                }
                        }
                    }
                    return BV_SUCCESS;
                }
        };
        return std::make_shared<node_sequence>(std::vector {std::forward<T>(t)...});
    }

    template<typename... T> bvnode_ptr parallel(T && ... t)
    {
        class node_parallel: public bvtree::node
        {
            private:
                std::vector<bvnode_ptr> m_nodes;

            private:
                std::vector<int> m_nodes_done;

            public:
                node_parallel(std::vector<bvnode_ptr> && v)
                    : m_nodes(std::move(v))
                    , m_nodes_done(m_nodes.size(), 0)
                {}

            public:
                void reset() override
                {
                    for(auto &node: m_nodes){
                        node->reset();
                    }

                    for(auto &node_done: m_nodes_done){
                        node_done = 0;
                    }
                }

            public:
                bvres_t update() override
                {
                    if(m_nodes.empty() || m_nodes_done.empty()){
                        throw std::runtime_error(str_fflprintf(": No valid node"));
                    }

                    bool has_pending = false;
                    for(size_t index = 0; index < m_nodes.size(); ++index){
                        if(m_nodes_done[index]){
                            continue;
                        }
                        switch(auto op_status = m_nodes[index]->update()){
                            case BV_ABORT:
                            case BV_FAILURE:
                                {
                                    return op_status;
                                }
                            case BV_PENDING:
                                {
                                    has_pending = true;
                                    break;
                                }
                            case BV_SUCCESS:
                                {
                                    m_nodes_done[index] = 1;
                                    break;
                                }
                            default:
                                {
                                    throw std::runtime_error(str_fflprintf(": Invalid node status: %d", op_status));
                                }
                        }
                    }
                    return has_pending ? BV_PENDING : BV_SUCCESS;
                }
        };
        return std::make_shared<node_parallel>(std::vector {std::forward<T>(t)...});
    }
}

namespace bvtree
{
    bvnode_ptr lambda(std::function<bvres_t()>);
    bvnode_ptr lambda(std::function<void()>, std::function<bvres_t()>);

    bvnode_ptr lambda_bool(std::function<bool()>);
    bvnode_ptr lambda_bool(std::function<void()>, std::function<bool()>);

    bvnode_ptr if_check (bvnode_ptr, bvnode_ptr);
    bvnode_ptr if_branch(bvnode_ptr, bvnode_ptr, bvnode_ptr);

    bvnode_ptr loop_while(bvnode_ptr, bvnode_ptr);
    bvnode_ptr loop_while( bvarg_ref, bvnode_ptr);

    bvnode_ptr loop_repeat(      int, bvnode_ptr);
    bvnode_ptr loop_repeat(bvarg_ref, bvnode_ptr);

    bvnode_ptr catch_abort(bvnode_ptr);
    bvnode_ptr abort_failure(bvnode_ptr);
    bvnode_ptr always_success(bvnode_ptr);

    bvnode_ptr op_not(bvnode_ptr);
    bvnode_ptr op_abort();
    bvnode_ptr op_delay(uint64_t, bvnode_ptr);
    bvnode_ptr op_timeout(uint64_t, bvnode_ptr);
}
