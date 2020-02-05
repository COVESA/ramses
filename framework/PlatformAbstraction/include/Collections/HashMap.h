//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_HASHMAP_H
#define RAMSES_UTILS_HASHMAP_H

#include <ramses-capu/container/HashTable.h>
#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformError.h"
#include "PlatformAbstraction/Macros.h"
#include <cmath>

namespace ramses_internal
{
    template <class Key, class T, class C = std::equal_to<Key>>
    class HashMap
    {
    public:
        typedef typename ramses_capu::HashTable<Key, T, C>::Iterator Iterator;
        typedef typename ramses_capu::HashTable<Key, T, C>::ConstIterator ConstIterator;

        HashMap();
        HashMap(const HashMap& other);
        HashMap(HashMap&& other) RNOEXCEPT;
        explicit HashMap(UInt initialCapacity);
        template <class Key2, class T2, class C2>
        explicit HashMap(const typename ramses_capu::HashTable<Key2, T2, C2>& capuHashMap);

        void reserve(UInt capacity);
        Iterator put(const Key& key, const T& value);
        EStatus get(const Key& key, T& value) const;
        T*     get(const Key& key) const;
        T& operator[](const Key& key);
        bool remove(const Key& key, T* value_old = nullptr);
        Iterator remove(Iterator iter, T* value_old = nullptr);
        bool   contains(const Key& key) const;
        UInt   size() const;
        UInt   capacity() const;
        Iterator find(const Key& key);
        ConstIterator find(const Key& key) const;
        void clear();
        Iterator begin();
        Iterator end();
        ConstIterator begin() const;
        ConstIterator end() const;

        HashMap& operator=(const HashMap<Key, T, C>& other);
        HashMap& operator=(HashMap<Key, T, C>&& other) RNOEXCEPT;

    private:
        typename ramses_capu::HashTable<Key, T, C> m_hashTable;
    };

    template <class Key, class T, class C>
    template <class Key2, class T2, class C2>
    ramses_internal::HashMap<Key, T, C>::HashMap(const typename ramses_capu::HashTable<Key2, T2, C2>& capuHashMap)
        : m_hashTable(capuHashMap.count())
    {
        for (typename ramses_capu::HashTable<Key2, T2, C2>::ConstIterator iter = capuHashMap.begin(); iter != capuHashMap.end(); ++iter)
        {
            put(iter->key, iter->value);
        }
    }

    template<class Key, class T, class C>
    inline
    HashMap<Key, T, C>::HashMap()
    {
    }

    template<class Key, class T, class C>
    inline
    HashMap<Key, T, C>::HashMap(const HashMap& other)
        : m_hashTable(other.m_hashTable)
    {
    }

    template<class Key, class T, class C>
    inline
    HashMap<Key, T, C>::HashMap(HashMap&& other) RNOEXCEPT
        : m_hashTable(std::move(other.m_hashTable))
    {
        static_assert(std::is_nothrow_move_constructible<HashMap>::value, "HashMap must be movable");
    }

    template<class Key, class T, class C>
    inline HashMap<Key, T , C>& HashMap<Key, T , C>::operator=(const HashMap<Key, T, C>& other)
    {
        m_hashTable = other.m_hashTable;
        return *this;
    }

    template<class Key, class T, class C>
    inline HashMap<Key, T , C>& HashMap<Key, T , C>::operator=(HashMap<Key, T, C>&& other) RNOEXCEPT
    {
        static_assert(std::is_nothrow_move_assignable<HashMap>::value, "HashMap must be movable");

        m_hashTable = std::move(other.m_hashTable);
        return *this;
    }

    template<class Key, class T, class C>
    inline
    HashMap<Key, T, C>::HashMap(UInt initialCapacity)
        : m_hashTable(initialCapacity)
    {
    }

    template<class Key, class T, class C>
    inline
    void HashMap<Key, T, C>::reserve(UInt capacity)
    {
        m_hashTable.reserve(capacity);
    }

    template<class Key, class T, class C>
    inline
    typename HashMap<Key, T, C>::Iterator HashMap<Key, T, C>::put(const Key& key, const T& value)
    {
        return m_hashTable.put(key, value);
    }

    template<class Key, class T, class C>
    inline
    EStatus HashMap<Key, T, C>::get(const Key& key, T& value)  const
    {
        HashMap<Key, T, C>::Iterator iter = m_hashTable.find(key);
        if (iter == m_hashTable.end())
            return EStatus_RAMSES_NOT_EXIST;
        value = iter->value;
        return EStatus_RAMSES_OK;
    }

    template<class Key, class T, class C>
    inline
    T* HashMap<Key, T, C>::get(const Key& key) const
    {
        auto iter = m_hashTable.find(key);
        if (iter != m_hashTable.end())
            return const_cast<T*>(&iter->value);   // TODO(tobias) constness is broken and has to be fixed
        return nullptr;
    }

    template<class Key, class T, class C>
    inline
    T& HashMap<Key, T, C>::operator[](const Key& key)
    {
        return m_hashTable[key];
    }

    template<class Key, class T, class C>
    inline
    bool HashMap<Key, T, C>::remove(const Key& key, T* value_old)
    {
        return m_hashTable.remove(key, value_old);
    }

    template<class Key, class T, class C>
    inline
    typename HashMap<Key, T, C>::Iterator HashMap<Key, T, C>::remove(Iterator iter, T* value_old)
    {
        return m_hashTable.remove(iter, value_old);
    }

    template<class Key, class T, class C>
    inline
    UInt HashMap<Key, T, C>::size() const
    {
        return m_hashTable.count();
    }

    template<class Key, class T, class C>
    inline
    UInt HashMap<Key, T, C>::capacity() const
    {
        return m_hashTable.capacity();
    }

    template<class Key, class T, class C>
    inline
    typename HashMap<Key, T, C>::Iterator HashMap<Key, T, C>::find(const Key& key)
    {
        return m_hashTable.find(key);
    }

    template<class Key, class T, class C>
    inline
    typename HashMap<Key, T, C>::ConstIterator HashMap<Key, T, C>::find(const Key& key) const
    {
        return m_hashTable.find(key);
    }

    template<class Key, class T, class C>
    inline
    void HashMap<Key, T, C>::clear()
    {
        m_hashTable.clear();
    }

    template<class Key, class T, class C>
    inline
    typename HashMap<Key, T, C>::ConstIterator HashMap<Key, T, C>::begin() const
    {
        return m_hashTable.begin();
    }

    template<class Key, class T, class C>
    inline
    typename HashMap<Key, T, C>::ConstIterator HashMap<Key, T, C>::end() const
    {
        return m_hashTable.end();
    }

    template<class Key, class T, class C>
    inline
    typename HashMap<Key, T, C>::Iterator HashMap<Key, T, C>::begin()
    {
        return m_hashTable.begin();
    }

    template<class Key, class T, class C>
    inline
    typename HashMap<Key, T, C>::Iterator HashMap<Key, T, C>::end()
    {
        return m_hashTable.end();
    }

    template<class Key, class T, class C>
    inline
    bool HashMap<Key, T, C>::contains(const Key& key) const
    {
        return m_hashTable.contains(key);
    }
}

#endif
