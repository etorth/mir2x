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
#include <vector>
#include <stdexcept>
#include "strfunc.hpp"

namespace bvtree
{
    class node
    {
        public:
            virtual void enter() {}
            virtual void exit () {}

        public:
            virtual int tick() = 0;
    };

    class sequence: public node
    {
        private:
            std::vector<node*> m_nodes;

        private:
            int m_currnode;

        private:
            bool m_running;

        public:
            sequence(const std::vector<node*> &nodes)
                : m_nodes(nodes)
                , m_currnode(-1)
                , m_running(false)
            {}

        public:
            void enter() override
            {
                if(!m_running){
                    m_currnode = 0;
                }
            }

        public:
            int tick() override
            {
                while(m_currnode < (int)(this->m_nodes.size())){
                    auto curr_node = m_nodes[m_currnode];

                    if(!m_running){
                        curr_node->enter();
                    }

                    switch(auto status = m_nodes[m_currnode]->tick()){
                        case bvtree::RUNNING:
                            {
                                m_running = true;
                                return bvtree::RUNNING;
                            }
                        case bvtree::SUCCESS:
                            {
                                m_running = false;
                                curr_node->exit();

                                m_currnode++;
                                if(m_currnode < (int)(m_nodes.size())){
                                    continue;
                                }else{
                                    return bvtree::SUCCESS;
                                }
                            }
                        case bvtree::ERROR:
                            {
                                m_running = false;
                                curr_node->exit();

                                return bvtree::ERROR;
                            }
                        default:
                            {
                                throw std::runtime_error(str_fflprintf(": Invalid bvtree status: %d", status));
                            }
                    }
                }
                return bvtree::SUCCESS;
            }
    };

    class selector: public node
    {
        private:
            std::vector<node*> m_nodes;

        private:
            int m_currnode;

        private:
            bool m_running;

        public:
            selector(const std::vector<node*> &nodes)
                : m_nodes(nodes)
                , m_currnode(-1)
                , m_running(false)
            {}

        public:
            void enter() override
            {
                if(!m_running){
                    m_currnode = 0;
                }
            }

        public:
            int tick() override
            {
                while(m_currnode < (int)(m_nodes.size())){
                    auto curr_node = m_nodes[m_currnode];

                    if(!m_running){
                        curr_node->enter();
                    }

                    switch(auto status = curr_node->tick()){
                        case bvtree::RUNNING:
                            {
                                m_running = true;
                                return bvtree::RUNNING;
                            }
                        case bvtree::SUCCESS:
                            {
                                m_running = false;
                                curr_node->exit();
                                return bvtree::SUCCESS;
                            }
                        case bvtree::ERROR:
                            {
                                m_running = false;
                                curr_node->exit();

                                m_currnode++;
                                if(m_currnode < (int)(m_nodes.size())){
                                    continue;
                                }else{
                                    return bvtree::ERROR;
                                }
                            }
                        default:
                            {
                                throw std::runtime_error(str_fflprintf(": Invalid bvtree status: %d", status));
                            }
                    }
                }
                return bvtree::ERROR;
            }
    };

    class random: public node
    {
        private:
            std::vector<node*>m_nodes;

        private:
            int m_currnode;

        private:
            bool m_running;

        public:
            random(const std::vector<node*> &nodes)
                : m_nodes()
                , m_currnode(-1)
                , m_running(false)
            {}

        public:
            void enter() override
            {
                if(!m_running){
                    m_currnode = 0;
                }
                m_currnode = std::rand() % (int)(m_nodes.size());
            }

        public:
            int tick() override
            {
                while(m_currnode < (int)(m_nodes.size())){
                    auto curr_node = m_nodes[m_currnode];

                    if(!m_running){
                        curr_node->enter();
                    }

                    switch(auto status = curr_node->tick()){
                        case bvtree::RUNNING:
                            {
                                m_running = true;
                                return bvtree::RUNNING;
                            }
                        case bvtree::SUCCESS:
                            {
                                m_running = false;
                                curr_node->exit();
                                return bvtree::SUCCESS;
                            }
                        case bvtree::ERROR:
                            {
                                m_running = false;
                                curr_node->exit();
                                return bvtree::ERROR;
                            }
                        default:
                            {
                                throw std::runtime_error(str_fflprintf(": Invalid bvtree status: %d", status));
                            }
                    }
                }
                return bvtree::ERROR;
            }
    };

    class root: public node
    {
        private:
            node* root;

        private:
            bool m_started;

        public:
            root(node *root)
                : m_root(root)
                , m_started(0)
            {}

        public:
            int tick() override
            {
                if(m_started){
                    return bvtree::RUNNING;
                }

                m_started = true;
                m_root->enter();

                switch(auto status = m_root->tick()){
                    case bvtree::RUNNING:
                        {
                            m_started = false;
                            return bvtree::RUNNING;
                        }
                    case bvtree::SUCCESS:
                        {
                            m_root->exit();
                            m_started = false;
                            return bvtree::SUCCESS;
                        }
                    case bvtree::ERROR:
                        {
                            root->exit();
                            m_started = false;
                            return bvtree::ERROR;
                        }
                    default:
                        {
                            throw std::runtime_error(str_fflprintf(": Invalid bvtree status: %d", status));
                        }
                }
            }
    };
}
