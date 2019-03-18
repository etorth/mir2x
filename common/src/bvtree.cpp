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
            int update() override
            {
                if(m_in_check){
                    switch(auto status = m_check->update()){
                        case bvtree::SUCCESS:
                            {
                                m_in_check = false;
                                break;
                            }
                        case bvtree::ABORT:
                        case bvtree::FAILURE:
                        case bvtree::PENDING:
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
                    case bvtree::SUCCESS:
                    case bvtree::FAILURE:
                        {
                            return bvtree::SUCCESS;
                        }
                    case bvtree::PENDING:
                    case bvtree::ABORT:
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
            int update() override
            {
                if(m_in == IN_CHECK){
                    switch(auto status = m_check->update()){
                        case bvtree::SUCCESS:
                            {
                                m_in = IN_ON_TRUE;
                                break;
                            }
                        case bvtree::FAILURE:
                            {
                                m_in = IN_ON_FALSE;
                                break;
                            }
                        case bvtree::ABORT:
                        case bvtree::PENDING:
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
                    case bvtree::ABORT:
                    case bvtree::PENDING:
                        {
                            return op_status;
                        }
                    case bvtree::SUCCESS:
                    case bvtree::FAILURE:
                        {
                            return bvtree::SUCCESS;
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
        int update() override
        {
            while(true){
                if(m_in_check){
                    switch(auto status = m_check->update()){
                        case bvtree::SUCCESS:
                            {
                                m_in_check = false;
                                break;
                            }
                        case bvtree::ABORT:
                        case bvtree::FAILURE:
                        case bvtree::PENDING:
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
                    case bvtree::SUCCESS:
                    case bvtree::FAILURE:
                        {
                            break;
                        }
                    case bvtree::PENDING:
                    case bvtree::ABORT:
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
    return std::make_shared<node_loop_while>(bvtree::lambda([arg]() -> int
    {
        if(auto p = std::get_if<bool>(arg.get())){
            return *p ? bvtree::SUCCESS : bvtree::FAILURE;
        }

        if(auto p = std::get_if<int>(arg.get())){
            return *p ? bvtree::SUCCESS : bvtree::FAILURE;
        }

        if(auto p = std::get_if<std::string>(arg.get())){
            return p->empty() ? bvtree::FAILURE : bvtree::SUCCESS;
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
        int update() override
        {
            while(m_index < m_repeat){
                switch(auto status = m_operation->update()){
                    case bvtree::ABORT:
                    case bvtree::PENDING:
                        {
                            return status;
                        }
                    case bvtree::FAILURE:
                    case bvtree::SUCCESS:
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
            return bvtree::SUCCESS;
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
            int update() override
            {
                switch(auto status = m_operation->update()){
                    case bvtree::ABORT:
                        {
                            return bvtree::FAILURE;
                        }
                    case bvtree::FAILURE:
                    case bvtree::PENDING:
                    case bvtree::SUCCESS:
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
            int update() override
            {
                switch(auto status = m_operation->update()){
                    case bvtree::ABORT:
                    case bvtree::PENDING:
                    case bvtree::SUCCESS:
                        {
                            return status;
                        }
                    case bvtree::FAILURE:
                        {
                            return bvtree::SUCCESS;
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
            int update() override
            {
                switch(auto status = m_operation->update()){
                    case bvtree::ABORT:
                    case bvtree::PENDING:
                        {
                            return status;
                        }
                    case bvtree::FAILURE:
                        {
                            return bvtree::SUCCESS;
                        }
                    case bvtree::SUCCESS:
                        {
                            return bvtree::FAILURE;
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
            int update() override
            {
                return bvtree::ABORT;
            }
    };
    return std::make_shared<node_op_abort>();
}
