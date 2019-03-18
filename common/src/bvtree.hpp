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

class bvarg_ptr
{
    private:
        using argtype = std::variant<bool, int, std::string>;

    private:
        std::shared_ptr<argtype> m_ptr;

    private:
        bvarg_ptr(std::shared_ptr<argtype> p)
            : m_ptr(p)
        {}

    public:
        bvarg_ptr() = default;

    public:
        template<typename I, typename... T> void assign(T && ... t)
        {
            if(m_ptr){
                m_ptr->emplace<I>(std::forward<T>(t)...);
            }else{
                m_ptr = std::make_shared<argtype>(std::in_place_type_t<I>(), std::forward<T>(t)...);
            }
        }

    public:
        bvarg_ptr clone() const
        {
            return m_ptr ? bvarg_ptr(m_ptr) : bvarg_ptr();
        }

    public:
        auto *get()
        {
            return m_ptr.get();
        }

        auto *get() const
        {
            return m_ptr.get();
        }

        auto &get_ref()
        {
            return *(m_ptr.get());
        }

        auto &get_ref() const
        {
            return *(m_ptr.get());
        }
};

// too error-prone
// don't allow implicit conversion
enum bvres_t
{
    BV_ABORT   = 0,
    BV_FAILURE = 1,
    BV_PENDING = 2,
    BV_SUCCESS = 3,
};

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
        protected:
            bvarg_ptr m_output;

        public:
            virtual void reset() {}

        public:
            std::shared_ptr<node> bind_output(bvarg_ptr output)
            {
                m_output = output;
                return shared_from_this();
            }

        public:
            virtual bvres_t update() = 0;
    };
}
using bvnode_ptr = std::shared_ptr<bvtree::node>;

namespace bvtree
{
    template<typename F> bvnode_ptr lambda(F && f)
    {
        class node_lambda: public bvtree::node
        {
            private:
                std::function<bvres_t()> m_func;

            public:
                node_lambda(F && f)
                    : m_func(std::forward<F>(f))
                {}

            public:
                bvres_t update() override
                {
                    switch(auto status = m_func()){
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
        return std::make_shared<node_lambda>(std::forward<F>(f));
    }

    template<typename F> bvnode_ptr lambda_bool(F && f)
    {
        // this is error-prone
        // we declare f_bool as std::function<bool()> but it accepts []() -> int
        // how to strictly allow only lambda returning bool ???

        return lambda([f_bool = std::function<bool()>(std::forward<F>(f))]() -> bvres_t
        {
            return f_bool() ? BV_SUCCESS : BV_FAILURE;
        });
    }

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
}

namespace bvtree
{
    bvnode_ptr if_check (bvnode_ptr, bvnode_ptr);
    bvnode_ptr if_branch(bvnode_ptr, bvnode_ptr, bvnode_ptr);

    bvnode_ptr loop_while(bvnode_ptr, bvnode_ptr);
    bvnode_ptr loop_while( bvarg_ptr, bvnode_ptr);

    bvnode_ptr loop_repeat(      int, bvnode_ptr);
    bvnode_ptr loop_repeat(bvarg_ptr, bvnode_ptr);

    bvnode_ptr catch_abort(bvnode_ptr);
    bvnode_ptr always_success(bvnode_ptr);

    bvnode_ptr op_not(bvnode_ptr);
    bvnode_ptr op_abort();
    bvnode_ptr op_delay(uint64_t, bvnode_ptr);
}
