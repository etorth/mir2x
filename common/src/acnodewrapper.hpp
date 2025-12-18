#pragma once
#include <stdexcept>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

template<typename C> class ACNodeWrapper
{
    private:
        template<typename T, typename = void> struct has_insert_return_type                                                : std::false_type {};
        template<typename T                 > struct has_insert_return_type<T, std::void_t<typename T::insert_return_type>>: std:: true_type {};

    private:
        const size_t m_maxRatio;
        const size_t m_avgRatioOld;
        const size_t m_avgRatioNew;

    private:
        size_t m_avgCount = 0;

    private:
        std::remove_cvref_t<C> m_container;
        std::vector<typename C::node_type> m_nodes;

    public:
        const std::remove_cvref_t<C> & c = m_container;

    public:
        explicit ACNodeWrapper(
                size_t maxRatio    = SIZE_MAX,
                size_t avgRatioOld = 2,
                size_t avgRatioNew = 1)

            : m_maxRatio   (maxRatio   )
            , m_avgRatioOld(avgRatioOld)
            , m_avgRatioNew(avgRatioNew)
        {
            switch(m_maxRatio){
                case 0:
                case SIZE_MAX:
                    {
                        break;
                    }
                default:
                    {
                        if(m_avgRatioOld + m_avgRatioNew == 0){
                            throw std::invalid_argument(__func__);
                        }
                        break;
                    }
            }
        }

    public:
        size_t has_node() const noexcept
        {
            return m_nodes.size();
        }

    public:
        template<typename Iter> requires (std::same_as<Iter, typename C::iterator> || std::same_as<Iter, typename C::const_iterator>) std::pair<Iter, bool> erase(Iter p)
        {
            switch(m_maxRatio){
                case 0: // never keep nodes
                    {
                        return {m_container.erase(p), true};
                    }
                case SIZE_MAX: // always keep nodes
                    {
                        return {erase_by_extract(p), true};
                    }
                default:
                    {
                        update_avg_count();
                        if(const auto maxNodeCount = m_avgCount * m_maxRatio; m_nodes.size() < maxNodeCount){
                            return {erase_by_extract(p), true};
                        }
                        else{
                            m_nodes.resize(maxNodeCount);
                            return {m_container.erase(p), true};
                        }
                    }
            }
        }

    public:
        template<typename Key> auto erase(this auto && self, const Key & key)
        {
            if(auto iter = self.m_container.find(key); iter != self.m_container.end()){
                return self.erase(iter);
            }
            return std::pair(self.m_container.end(), false);
        }

    public:
        void clear()
        {
            while(!m_container.empty()){
                m_nodes.push_back(m_container.extract(m_container.begin()));
            }
        }

        void shrink()
        {
            decltype(m_nodes)().swap(m_nodes);
        }

    public:
        template<typename Func> auto alloc_insert(Func && func)
        {
            return func(m_container);
        }

        template<typename Func> std::pair<typename C::iterator, bool> node_insert(Func && func)
        {
            func(m_nodes.back());
            if constexpr (has_insert_return_type<C>::value){
                if(auto r = m_container.insert(std::move(m_nodes.back())); r.inserted){
                    m_nodes.pop_back();
                    return {r.position, true};
                }
                else{
                    std::swap(m_nodes.back(), r.node);
                    return {r.position, false};
                }
            }
            else{
                auto r = m_container.insert(std::move(m_nodes.back()));
                m_nodes.pop_back();
                return {r, true};
            }
        }

    private:
        void update_avg_count() noexcept
        {
            m_avgCount = (m_avgCount * m_avgRatioOld + m_container.size() * m_avgRatioNew) / (m_avgRatioOld + m_avgRatioNew);
        }

        template<typename Iter> auto erase_by_extract(Iter p)
        {
            auto nextp = std::next(p);
            m_nodes.push_back(m_container.extract(p));
            return nextp;
        }
};
