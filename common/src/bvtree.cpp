/*
 * =====================================================================================
 *
 *       Filename: bvtree.cpp
 *        Created: 03/17/2019 05:10:48
 *    Description: 
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

#include "bvtree.hpp"
#include "raiitimer.hpp"

bvnode_ptr bvtree::if_check(bvnode_ptr check, bvnode_ptr operation)
{
    class node_if_check: public bvtree::node
    {
        private:
            bvnode_ptr m_check;
            bvnode_ptr m_operation;

        private:
            bool m_in_check;

        public:
            node_if_check(bvnode_ptr check, bvnode_ptr operation)
                : m_check(check)
                , m_operation(operation)
                , m_in_check(false)
            {}

        public:
            void reset() override
            {
                m_in_check = true;
                m_check->reset();
                m_operation->reset();
            }

        public:
            bvres_t update() override
            {
                if(m_in_check){
                    switch(auto status = m_check->update()){
                        case BV_SUCCESS:
                            {
                                m_in_check = false;
                                break;
                            }
                        case BV_ABORT:
                        case BV_FAILURE:
                        case BV_PENDING:
                            {
                                return status;
                            }
                        default:
                            {
                                throw std::invalid_argument(str_fflprintf(": Invalid node status: %d", status));
                            }
                    }
                }

                switch(auto op_status = m_operation->update()){
                    case BV_SUCCESS:
                    case BV_FAILURE:
                        {
                            return BV_SUCCESS;
                        }
                    case BV_PENDING:
                    case BV_ABORT:
                        {
                            return op_status;
                        }
                    default:
                        {
                            throw std::invalid_argument(str_fflprintf(": Invalid node status: %d", op_status));
                        }
                }
            }
    };
    return std::make_shared<node_if_check>(check, operation);
}

bvnode_ptr bvtree::if_branch(bvnode_ptr check, bvnode_ptr on_true, bvnode_ptr on_false)
{
    class node_if_branch: public bvtree::node
    {
        private:
            enum
            {
                IN_CHECK,
                IN_ON_TRUE,
                IN_ON_FALSE,
            };

        private:
            bvnode_ptr m_check;
            bvnode_ptr m_on_true;
            bvnode_ptr m_on_false;

        private:
            int m_in;

        public:
            node_if_branch(bvnode_ptr check, bvnode_ptr on_true, bvnode_ptr on_false)
                : m_check(check)
                , m_on_true(on_true)
                , m_on_false(on_false)
                , m_in(IN_CHECK)
            {}

        public:
            void reset() override
            {
                m_check->reset();
                m_on_true->reset();
                m_on_false->reset();
                m_in = IN_CHECK;
            }

        public:
            bvres_t update() override
            {
                if(m_in == IN_CHECK){
                    switch(auto status = m_check->update()){
                        case BV_SUCCESS:
                            {
                                m_in = IN_ON_TRUE;
                                break;
                            }
                        case BV_FAILURE:
                            {
                                m_in = IN_ON_FALSE;
                                break;
                            }
                        case BV_ABORT:
                        case BV_PENDING:
                            {
                                return status;
                            }
                        default:
                            {
                                throw std::invalid_argument(str_fflprintf(": Invalid node status: %d", status));
                            }
                    }
                }

                switch(auto op_status = ((m_in == IN_ON_TRUE) ? m_on_true->update() : m_on_false->update())){
                    case BV_ABORT:
                    case BV_PENDING:
                        {
                            return op_status;
                        }
                    case BV_SUCCESS:
                    case BV_FAILURE:
                        {
                            return BV_SUCCESS;
                        }
                    default:
                        {
                            throw std::invalid_argument(str_fflprintf(": Invalid node status: %d", op_status));
                        }
                }
            }
    };
    return std::make_shared<node_if_branch>(check, on_true, on_false);
}

class node_loop_while: public bvtree::node
{
    private:
        bvnode_ptr m_check;
        bvnode_ptr m_operation;

    private:
        bool m_in_check;

    public:
        node_loop_while(bvnode_ptr check, bvnode_ptr operation)
            : m_check(check)
            , m_operation(operation)
        {}

    public:
        void reset() override
        {
            m_in_check = true;
            m_check->reset();
            m_operation->reset();
        }

    public:
        bvres_t update() override
        {
            while(true){
                if(m_in_check){
                    switch(auto status = m_check->update()){
                        case BV_SUCCESS:
                            {
                                m_in_check = false;
                                break;
                            }
                        case BV_ABORT:
                        case BV_FAILURE:
                        case BV_PENDING:
                            {
                                return status;
                            }
                        default:
                            {
                                throw std::invalid_argument(str_fflprintf(": Invalid node status: %d", status));
                            }
                    }
                }

                switch(auto op_status = m_operation->update()){
                    case BV_SUCCESS:
                    case BV_FAILURE:
                        {
                            break;
                        }
                    case BV_PENDING:
                    case BV_ABORT:
                        {
                            return op_status;
                        }
                    default:
                        {
                            throw std::invalid_argument(str_fflprintf(": Invalid node status: %d", op_status));
                        }
                }
            }
        }
};

bvnode_ptr bvtree::loop_while(bvnode_ptr check, bvnode_ptr operation)
{
    return std::make_shared<node_loop_while>(check, operation);
}

bvnode_ptr bvtree::loop_while(bvarg_ptr arg, bvnode_ptr operation)
{
    return std::make_shared<node_loop_while>(bvtree::lambda([arg]()
    {
        if(auto p = arg.as_if<bool>()){
            return *p ? BV_SUCCESS : BV_FAILURE;
        }

        if(auto p = arg.as_if<int>()){
            return *p ? BV_SUCCESS : BV_FAILURE;
        }

        if(auto p = arg.as_if<std::string>()){
            return p->empty() ? BV_FAILURE : BV_SUCCESS;
        }

        // should I return ABORT?
        // need to conclude difference between std::exception vs ABORT
        throw std::runtime_error(str_fflprintf(": Captured argument doesn't contain valid type"));
    }), operation);
}

class node_repeat_loop: public bvtree::node
{
    private:
        std::function<int()> m_repeat_func;

    private:
        int m_index;
        int m_repeat;

    private:
        bvnode_ptr m_operation;

    public:
        template<typename F> node_repeat_loop(F && f, bvnode_ptr operation)
            : m_repeat_func(std::forward<F>(f))
            , m_index(0)
            , m_repeat(0)
            , m_operation(operation)
        {}

    public:
        void reset()
        {
            m_index = 0;
            m_repeat = m_repeat_func();

            // everytime when reset we get new number
            // during the repeating we won't change this number

            if(m_repeat < 0){
                throw std::runtime_error(str_fflprintf(": negative repeat number: %d", m_repeat));
            }
            m_operation->reset();
        }

    public:
        bvres_t update() override
        {
            while(m_index < m_repeat){
                switch(auto status = m_operation->update()){
                    case BV_ABORT:
                    case BV_PENDING:
                        {
                            return status;
                        }
                    case BV_FAILURE:
                    case BV_SUCCESS:
                        {
                            m_index++;
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

bvnode_ptr bvtree::loop_repeat(int n, bvnode_ptr operation)
{
    return std::make_shared<node_repeat_loop>([n](){return n;}, operation);
}

bvnode_ptr bvtree::loop_repeat(bvarg_ptr arg, bvnode_ptr operation)
{
    return std::make_shared<node_repeat_loop>([arg]()->int
    {
        if(auto p = std::get_if<int>(arg.get())){
            return *p;
        }
        throw std::runtime_error(str_fflprintf(": Captured argument doesn't contain valid type"));
    }, operation);
}

bvnode_ptr bvtree::catch_abort(bvnode_ptr operation)
{
    class node_catch_abort: public bvtree::node
    {
        private:
            bvnode_ptr m_operation;

        public:
            node_catch_abort(bvnode_ptr operation)
                : m_operation(operation)
            {}

        public:
            void reset() override
            {
                m_operation->reset();
            }

        public:
            bvres_t update() override
            {
                switch(auto status = m_operation->update()){
                    case BV_ABORT:
                        {
                            return BV_FAILURE;
                        }
                    case BV_FAILURE:
                    case BV_PENDING:
                    case BV_SUCCESS:
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
    return std::make_shared<node_catch_abort>(operation);
}

bvnode_ptr bvtree::abort_failure(bvnode_ptr operation)
{
    class node_abort_failure: public bvtree::node
    {
        private:
            bvnode_ptr m_operation;

        public:
            node_abort_failure(bvnode_ptr operation)
                : m_operation(operation)
            {}

        public:
            void reset() override
            {
                m_operation->reset();
            }

        public:
            bvres_t update() override
            {
                switch(auto op_status = m_operation->update()){
                    case BV_ABORT:
                    case BV_PENDING:
                    case BV_SUCCESS:
                        {
                            return op_status;
                        }
                    case BV_FAILURE:
                        {
                            return BV_ABORT;
                        }
                    default:
                        {
                            throw std::runtime_error(str_fflprintf(": Invalid node status: %d", op_status));
                        }
                }
            }
    };
    return std::make_shared<node_abort_failure>(operation);
}

bvnode_ptr bvtree::always_success(bvnode_ptr operation)
{
    class node_always_success: public bvtree::node
    {
        private:
            bvnode_ptr m_operation;

        public:
            node_always_success(bvnode_ptr operation)
                : m_operation(operation)
            {}

        public:
            void reset() override
            {
                m_operation->reset();
            }

        public:
            bvres_t update() override
            {
                switch(auto status = m_operation->update()){
                    case BV_ABORT:
                    case BV_PENDING:
                    case BV_SUCCESS:
                        {
                            return status;
                        }
                    case BV_FAILURE:
                        {
                            return BV_SUCCESS;
                        }
                    default:
                        {
                            throw std::runtime_error(str_fflprintf(": Invalid node status: %d", status));
                        }
                }
            }
    };
    return std::make_shared<node_always_success>(operation);
}

bvnode_ptr bvtree::op_not(bvnode_ptr operation)
{
    class node_op_not: public bvtree::node
    {
        private:
            bvnode_ptr m_operation;

        public:
            node_op_not(bvnode_ptr operation)
                : m_operation(operation)
            {}

        public:
            void reset() override
            {
                m_operation->reset();
            }

        public:
            bvres_t update() override
            {
                switch(auto status = m_operation->update()){
                    case BV_ABORT:
                    case BV_PENDING:
                        {
                            return status;
                        }
                    case BV_FAILURE:
                        {
                            return BV_SUCCESS;
                        }
                    case BV_SUCCESS:
                        {
                            return BV_FAILURE;
                        }
                    default:
                        {
                            throw std::runtime_error(str_fflprintf(": Invalid node status: %d", status));
                        }
                }
            }
    };
    return std::make_shared<node_op_not>(operation);
}

bvnode_ptr bvtree::op_abort()
{
    class node_op_abort: public bvtree::node
    {
        public:
            bvres_t update() override
            {
                return BV_ABORT;
            }
    };
    return std::make_shared<node_op_abort>();
}

bvnode_ptr bvtree::op_delay(uint64_t ms, bvnode_ptr operation)
{
    class node_op_delay: public bvtree::node
    {
        private:
            const uint64_t m_delay;

        private:
            bool m_running;

        private:
            hres_timer m_timer;

        private:
            bvnode_ptr m_operation;

        public:
            node_op_delay(uint32_t ms, bvnode_ptr operation)
                : m_delay(ms)
                , m_running(false)
                , m_timer()
                , m_operation(operation)
            {}

        public:
            void reset() override
            {
                m_running = false;
                m_operation->reset();
            }

        public:
            bvres_t update() override
            {
                if(!m_running){
                    m_timer.reset();
                    m_running = true;
                }

                if(m_timer.diff_msec() <= m_delay){
                    return BV_PENDING;
                }

                switch(auto op_status = m_operation->update()){
                    case BV_ABORT:
                    case BV_PENDING:
                    case BV_FAILURE:
                    case BV_SUCCESS:
                        {
                            return op_status;
                        }
                    default:
                        {
                            throw std::runtime_error(str_fflprintf(": Invalid node status: %d", op_status));
                        }
                }
            }
    };
    return std::make_shared<node_op_delay>(ms, operation);
}
