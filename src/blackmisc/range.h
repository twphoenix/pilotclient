/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_RANGE_H
#define BLACKMISC_RANGE_H

#include "blackmisc/blackmiscexport.h"
#include "blackmisc/iterator.h"
#include "blackmisc/predicates.h"
#include "blackmisc/algorithm.h"
#include "blackmisc/typetraits.h"
#include <QtGlobal>
#include <QDebug>
#include <algorithm>
#include <type_traits>
#include <iterator>
#include <initializer_list>

namespace BlackMisc
{
    template <class> class CRange;

    /*!
     * Any container class with begin and end iterators can inherit from this CRTP class
     * to gain some useful algorithms as member functions.
     * \tparam Derived The most derived container class inheriting from this instantiation.
     */
    template <class Derived>
    class CRangeBase
    {
    public:
        //! Return a new container generated by applying some transformation function to all elements of this one.
        template <class F>
        inline auto transform(F function) const;

        //! Return a copy containing only those elements for which a given predicate returns true.
        template <class Predicate>
        inline auto findBy(Predicate p) const;

        //! Return a copy containing only those elements matching some particular key/value pair(s).
        //! \param k0 A pointer to a member function of T.
        //! \param v0 A value to compare against the value returned by k0.
        //! \param keysValues Zero or more additional pairs of { pointer to member function of T, value to compare it against }.
        template <class K0, class V0, class... KeysValues>
        inline auto findBy(K0 k0, V0 v0, KeysValues... keysValues) const;

        //! Return a reference to the first element for which a given predicate returns true. Undefined if there is none.
        template <class Predicate>
        const auto &findFirstBy(Predicate p) const { return findBy(p).front(); }

        //! Return a reference to the first element matching some particular key/value pair(s). Undefined if there is none.
        template <class K, class V>
        const auto &findFirstBy(K key, V value) const { return findBy(key, value).front(); }

        //! Return a copy of the first element for which a given predicate returns true, or a default value if there is none.
        template <class Predicate, class Value>
        auto findFirstByOrDefault(Predicate p, const Value &def) const { return findBy(p).frontOrDefault(def); }

        //! Return a copy of the first element for which a given predicate returns true, or a default value if there is none.
        template <class Predicate>
        auto findFirstByOrDefault(Predicate p) const { return findBy(p).frontOrDefault(); }

        //! Return a copy of the first element matching some particular key/value pair(s), or a default value if there is none.
        template <class K, class V, class Value>
        auto findFirstByOrDefault(K key, V value, const Value &def) const { return findBy(key, value).frontOrDefault(def); }

        //! Return a copy of the first element matching some particular key/value pair(s), or a default value if there is none.
        template <class T, class K, class V>
        auto findFirstByOrDefault(K T::* key, V value) const { return findBy(key, value).frontOrDefault(); }

        //! Return true if there is an element for which a given predicate returns true.
        template <class Predicate>
        bool containsBy(Predicate p) const
        {
            return std::any_of(derived().cbegin(), derived().cend(), p);
        }

        //! Return true if there is an element equal to given object. Uses the most efficient implementation available in the derived container.
        template <class T>
        bool contains(const T &object) const
        {
            return derived().find(object) != derived().cend();
        }

        //! Return a copy containing only those elements matching some particular key/value pair(s).
        //! \param k0 A pointer to a member function of T.
        //! \param v0 A value to compare against the value returned by k0.
        //! \param keysValues Zero or more additional pairs of { pointer to member function of T, value to compare it against }.
        template <class K0, class V0, class... KeysValues>
        bool contains(K0 k0, V0 v0, KeysValues... keysValues) const
        {
            return containsBy(BlackMisc::Predicates::MemberEqual(k0, v0, keysValues...));
        }

        //! Return true if this container equals another container according to the given element equality predicate.
        template <class T, class Predicate>
        bool equalsBy(const T &other, Predicate c) const
        {
            if (equalPointers(&derived(), &other)) { return true; }
            return std::equal(derived().begin(), derived().end(), other.begin(), other.end(), c);
        }

        //! Return true if this container equals another container, considering only the given element members.
        template <class T, class Key0, class... Keys>
        bool equalsByKeys(const T &other, Key0 k0, Keys... keys) const
        {
            return equalsBy(other, BlackMisc::Predicates::EqualsByMembers(k0, keys...));
        }

        //! Pick one random element
        template <class T>
        T randomElement() const
        {
            return this->randomElements(1).front();
        }

        //! Copy n elements from the container at random.
        Derived randomElements(int n) const
        {
            Derived result;
            BlackMisc::copyRandomElements(derived().begin(), derived().end(), std::inserter(result, result.end()), n);
            return result;
        }

        //! Copy n elements from the container, randomly selected but evenly distributed.
        Derived sampleElements(int n) const
        {
            Derived result;
            BlackMisc::copySampleElements(derived().begin(), derived().end(), std::inserter(result, result.end()), n);
            return result;
        }

    protected:
        //! Efficiently compare addresses of two objects. Return false if types are not compatible.
        template <typename T, typename U>
        static bool equalPointers(const T *a, const U *b)
        {
            if constexpr (TIsEqualityComparable<const T *, const U *>::value) { return a == b; }
            else { return false; }
        }

    private:
        Derived &derived() { return static_cast<Derived &>(*this); }
        const Derived &derived() const { return static_cast<const Derived &>(*this); }
    };

    /*!
     * A range is a conceptual container which does not contain any elements of its own,
     * but is constructed from a begin iterator and an end iterator of another container.
     *
     * By using iterator wrappers, it is possible to use CRange to iterate over the results of predicate methods without copying elements.
     *
     * \warning Remember that the iterators in the range refer to elements in the original container,
     *          so take care that the original container remains valid and does not invalidate its iterators
     *          during the lifetime of the range.
     */
    template <class I>
    class CRange : public CRangeBase<CRange<I>>
    {
    public:
        //! STL compatibility
        //! @{
        typedef typename std::iterator_traits<I>::value_type value_type;
        typedef typename std::iterator_traits<I>::reference reference;
        typedef typename std::iterator_traits<I>::difference_type difference_type;
        typedef const value_type &const_reference;
        typedef value_type key_type;
        typedef difference_type size_type;
        typedef I iterator;
        typedef I const_iterator;
        typedef std::reverse_iterator<I> reverse_iterator;
        typedef std::reverse_iterator<I> const_reverse_iterator;
        //! @}

        //! Constructor.
        CRange(I begin, I end) : m_begin(begin), m_end(end) { check(&begin, &end); }

        //! Begin and end iterators.
        //! @{
        const_iterator begin() const { return m_begin; }
        const_iterator cbegin() const { return m_begin; }
        const_iterator end() const { return m_end; }
        const_iterator cend() const { return m_end; }
        //! @}

        //! Reverse begin and end iterators.
        //! @{
        const_reverse_iterator rbegin() const { return const_reverse_iterator(m_end); }
        const_reverse_iterator crbegin() const { return const_reverse_iterator(m_end); }
        const_reverse_iterator rend() const { return const_reverse_iterator(m_begin); }
        const_reverse_iterator crend() const { return const_reverse_iterator(m_begin); }
        //! @}

        //! Create a range from reverse iterators.
        CRange<const_reverse_iterator> reverse() const
        {
            static_assert(std::is_same_v<decltype(*rbegin()), decltype(*begin())>, "see https://dev.swift-project.org/T700");
            return { rbegin(), rend() };
        }

        //! Implicit conversion to any container of value_type which supports push_back. This will copy elements.
        template <class T, class = std::enable_if_t<std::is_convertible_v<value_type, typename T::value_type>>>
        operator T() const
        {
            return to<T>();
        }

        //! Explicit conversion to any container of value_type which supports push_back. This will copy elements.
        //! @{
        template <class T>
        T to() const
        {
            T container;
            std::copy(begin(), end(), Iterators::makeInsertIterator(container));
            return container;
        }
        template <template <class...> class T>
        auto to() const
        {
            return to<T<value_type>>();
        }
        //! @}

        //! Returns true if the range is empty.
        //! @{
        bool empty() const { return begin() == end(); }
        bool isEmpty() const { return empty(); }
        //! @}

        //! Returns the number of elements in the range.
        size_type size() const { return std::distance(begin(), end()); }

        //! Returns the element at the beginning of the range. Undefined if the range is empty.
        const_reference front() const { Q_ASSERT(!empty()); return *begin(); }

        //! Returns the element at the beginning of the range, or a default value if the range is empty.
        const_reference frontOrDefault() const
        {
            static const value_type def{};
            return empty() ? def : *begin();
        }

        //! Returns the element at the beginning of the range, or a default value if the range is empty.
        value_type frontOrDefault(value_type def) const
        {
            return empty() ? def : *begin();
        }

    private:
        I m_begin;
        I m_end;

        void check(...) {}
        template <class I2, class F> void check(Iterators::ConditionalIterator<I2, F> *begin, Iterators::ConditionalIterator<I2, F> *end)
        {
            begin->checkEnd(*end);
        }
    };

    /*!
     * Streaming operators for CRange to qDebug.
     */
    //! @{
    template <class I>
    QDebug operator <<(QDebug d, const CRange<I> &range)
    {
        for (const auto &v : range) { d << v; }
        return d;
    }
    template <class I>
    QNoDebug operator <<(QNoDebug d, const CRange<I> &)
    {
        return d;
    }
    //! @}

    /*!
     * Returns a CRange constructed from begin and end iterators of deduced types.
     * \param begin The begin iterator.
     * \param end The end iterator, which can be any iterator type convertible to I.
     */
    template <class I, class I2>
    auto makeRange(I begin, I2 end) -> CRange<I>
    {
        return { begin, static_cast<I>(end) };
    }

    /*!
     * Returns a CRange constructed from the begin and end iterators of the given container.
     */
    //! @{
    template <class T>
    auto makeRange(T &container) -> CRange<decltype(container.begin())>
    {
        return { container.begin(), container.end() };
    }
    template <class T>
    auto makeRange(const T &container) -> CRange<decltype(container.begin())>
    {
        return { container.begin(), container.end() };
    }
    //! @}

    /*!
     * Returns a const CRange constructed from the cbegin and cend iterators of the given container.
     */
    template <class T>
    auto makeConstRange(const T &container) -> CRange<decltype(container.cbegin())>
    {
        return { container.cbegin(), container.cend() };
    }

    /*!
     * Returns a const CRange for iterating over the keys of a Qt associative container.
     *
     * This is more efficient than the keys() method of the container, as it doesn't allocate memory.
     */
    //! @{
    template <class T>
    auto makeKeysRange(const T &container)
    {
        return makeRange(container.keyBegin(), container.keyEnd());
    }
    template <class T>
    void makeKeysRange(T &container)
    {
        container.detach(); // http://doc.qt.io/qt-5/containers.html#implicit-sharing-iterator-problem
        return makeRange(container.keyValueBegin(), container.keyValueEnd());
    }
    //! @}

    /*!
     * Returns a const CRange for iterating over the keys and values of a Qt associative container.
     * The value_type of the returned range is std::pair<T::key_type &, T::value_type &>.
     *
     * This is more efficient than using the keys() method of the container and thereafter looking up each key,
     * as it neither allocates memory, nor performs any key lookups.
     */
    //! @{
    template <class T>
    auto makePairsRange(const T &container)
    {
        return makeRange(container.keyValueBegin(), container.keyValueEnd());
    }
    template <class T>
    auto makePairsRange(T &container)
    {
        container.detach(); // http://doc.qt.io/qt-5/containers.html#implicit-sharing-iterator-problem
        return makeRange(container.keyValueBegin(), container.keyValueEnd());
    }
    //! @}

    /*!
     * Keys range for a temporary would be unsafe.
     */
    template <class T>
    void makeKeysRange(const T &&container) = delete;

    /*!
     * Pairs range for a temporary would be unsafe.
     */
    template <class T>
    void makePairsRange(const T &&container) = delete;

    /*
     * Member functions of CRangeBase template defined out of line, because they depend on CRange etc.
     */
    template <class Derived>
    template <class F>
    auto CRangeBase<Derived>::transform(F function) const
    {
        return makeRange(Iterators::makeTransformIterator(derived().cbegin(), function), derived().cend());
    }

    template <class Derived>
    template <class Predicate>
    auto CRangeBase<Derived>::findBy(Predicate p) const
    {
        return makeRange(Iterators::makeConditionalIterator(derived().cbegin(), derived().cend(), p), derived().cend());
    }

    template <class Derived>
    template <class K0, class V0, class... KeysValues>
    auto CRangeBase<Derived>::findBy(K0 k0, V0 v0, KeysValues... keysValues) const
    {
        return findBy(BlackMisc::Predicates::MemberEqual(k0, v0, keysValues...));
    }

}

#endif // guard
