//
// Created by yuwenyong.vincent on 2021/8/21.
//

#ifndef EASYEVENT_COMMON_ORDEREDDICT_H
#define EASYEVENT_COMMON_ORDEREDDICT_H

#include "EasyEvent/Common/Config.h"


namespace EasyEvent {

    template <typename KeyT, typename ValueT>
    class OrderedDict {
    public:

        typedef KeyT                                        key_type;
        typedef ValueT                                      mapped_type;
        typedef std::pair<const key_type, mapped_type>      value_type;
        typedef value_type&                                 reference;
        typedef const value_type&                           const_reference;
        typedef std::list<value_type>                       list_type;
        typedef typename list_type::size_type               size_type;


        typedef typename list_type::iterator                iterator;
        typedef typename list_type::const_iterator          const_iterator;
        typedef typename list_type::reverse_iterator        reverse_iterator;
        typedef typename list_type::const_reverse_iterator  const_reverse_iterator;

        typedef std::map<KeyT, std::vector<iterator>>       map_type;

        OrderedDict() = default;

        template<typename InputIterator>
        OrderedDict(InputIterator first, InputIterator last) {
            insert(first, last);
        }

        OrderedDict(std::initializer_list<value_type> il) {
            insert(il);
        }

        iterator begin() noexcept {
            return _list.begin();
        }

        iterator end() noexcept {
            return _list.end();
        }

        const_iterator begin() const noexcept {
            return _list.begin();
        }

        const_iterator end() const noexcept {
            return _list.end();
        }

        reverse_iterator rbegin() noexcept {
            return _list.rbegin();
        }

        reverse_iterator rend() noexcept {
            return _list.rend();
        }

        const_reverse_iterator rbegin() const noexcept {
            return _list.rbegin();
        }

        const_reverse_iterator rend() const noexcept {
            return _list.rend();
        }

        const_iterator cbegin() const noexcept {
            return _list.cbegin();
        }

        const_iterator cend() const noexcept {
            return _list.cend();
        }

        const_reverse_iterator crbegin() const noexcept {
            return _list.crbegin();
        }

        const_reverse_iterator crend() const noexcept {
            return _list.crend();
        }

        bool empty() const noexcept {
            return _list.empty();
        }

        size_type size() const noexcept {
            return _list.size();
        }

        iterator insert(const value_type& val) {
            iterator retVal = _list.insert(_list.end(), val);
            insertIntoMap(retVal);
            return retVal;
        }

        iterator insert(value_type&& val) {
            auto retVal = _list.insert(_list.end(), std::move(val));
            insertIntoMap(retVal);
            return retVal;
        }

        template<typename InputIterator>
        iterator insert(InputIterator first, InputIterator last) {
            auto retVal = _list.insert(_list.end(), first, last);
            insertRangeIntoMap(retVal, _list.end());
            return retVal;
        }

        iterator insert(std::initializer_list<value_type> il) {
            auto retVal = _list.insert(_list.end(), il);
            insertRangeIntoMap(retVal, _list.end());
            return retVal;
        }

        bool erase(const_iterator pos) {
            if (eraseFromMap(pos)) {
                _list.erase(pos);
                return true;
            } else {
                return false;
            }
        }

        size_type erase(const_iterator first, const_iterator last) {
            size_type count = 0;
            for (auto iter = first; iter != last;) {
                if (erase(iter++)) {
                    ++count;
                }
            }
            return count;
        }

        void swap(OrderedDict& x) noexcept {
            _list.swap(x._list);
            _map.swap(x._map);
        }

        void clear() noexcept {
            _list.clear();
            _map.clear();
        }

        template<typename... Args>
        iterator emplace(Args&&... args) {
            return insert(std::make_pair(std::forward<Args>(args)...));
        }

        bool contains(const KeyT& k) const {
            return _map.find(k) != _map.cend();
        }

        iterator find(const KeyT& k) {
            auto mapIter = _map.find(k);
            return mapIter != _map.end() ? mapIter->second.begin() : _list.end();
        }

        const_iterator find(const KeyT& k) const {
            auto mapIter = _map.find(k);
            return mapIter != _map.cend() ? mapIter->second.cbegin() : _list.cend();
        }

        map_type& get(const KeyT& k) {
            auto mapIter = _map.find(k);
            if (mapIter == _map.end()) {
                throw std::out_of_range("key not found");
            }
            return mapIter->second.front()->second;
        }

        const map_type& get(const KeyT& k) const {
            auto mapIter = _map.find(k);
            if (mapIter == _map.cend()) {
                throw std::out_of_range("key not found");
            }
            return mapIter->second.front()->second;
        }

        size_type count(const KeyT& k) const {
            auto mapIter = _map.find(k);
            return mapIter != _map.cend() ? mapIter->second.size() : 0;
        }

        std::vector<iterator> findRange(const KeyT& k) {
            std::vector<iterator> retVal;
            auto mapIter = _map.find(k);
            if (mapIter != _map.end()) {
                retVal = mapIter->second;
            }
            return retVal;
        }

        std::vector<const_iterator> findRange(const KeyT& k) const {
            std::vector<iterator> retVal;
            auto mapIter = _map.find(k);
            if (mapIter != _map.end()) {
                retVal = mapIter->second;
            }
            return retVal;
        }

        std::vector<mapped_type> getAll(const KeyT& k) {
            std::vector<mapped_type> retVal = {};
            auto mapIter = _map.find(k);
            if (mapIter != _map.end()) {
                for (auto& iter: mapIter->second) {
                    retVal.template emplace_back(iter->second);
                }
            }
            return retVal;
        }

        mapped_type& operator[](const key_type& k) {
            auto iter = emplace(k, mapped_type{});
            return iter->second;
        }

        mapped_type& operator[](key_type&& k) {
            auto iter = emplace(std::move(k), mapped_type{});
            return iter->second;
        }
    private:
        void insertRangeIntoMap(iterator first, iterator last) {
            for (auto iter = first; iter != last; ++iter) {
                insertIntoMap(iter);
            }
        }

        void insertIntoMap(iterator pos) {
            auto iter = _map.find(pos->first);
            if (iter != _map.end()) {
                iter->second.push_back(pos);
            } else {
                _map[pos->first] = std::vector<iterator>{pos};
            }
        }

        bool eraseFromMap(iterator pos) {
            auto mapIter = _map.find(pos->first);
            if (mapIter == _map.end()) {
                return false;
            }
            auto vecIter = std::find(mapIter->second.begin(), mapIter->second.end(), pos);
            if (vecIter == mapIter->second.end()) {
                return false;
            }
            mapIter->second.erase(vecIter);
            if (mapIter->second.empty()) {
                _map.erase(mapIter);
            }
            return true;
        }

        list_type _list;
        map_type _map;
    };

}

#endif //EASYEVENT_COMMON_ORDEREDDICT_H
