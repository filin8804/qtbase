/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qmetacontainer.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMetaSequence
    \inmodule QtCore
    \since 6.0
    \brief The QMetaSequence class allows type erased access to sequential containers.

    \ingroup objectmodel

    The class provides a number of primitive container operations, using void*
    as operands. This way, you can manipulate a generic container retrieved from
    a Variant without knowing its type.

    The void* arguments to the various methods are typically created by using
    a \l QVariant of the respective container or value type, and calling
    its \l QVariant::data() or \l QVariant::constData() methods. However, you
    can also pass plain pointers to objects of the container or value type.

    Iterator invalidation follows the rules given by the underlying containers
    and is not expressed in the API. Therefore, for a truly generic container,
    any iterators should be considered invalid after any write operation.
*/

/*!
    \fn template<typename C> QMetaSequence QMetaSequence::fromContainer()
    \since 6.0

    Returns the QMetaSequence corresponding to the type given as template parameter.
*/

/*!
    Returns \c true if the underlying container provides at least a forward
    iterator as defined by std::forward_iterator_tag, otherwise returns
    \c false. Bi-directional iterators and random access iterators are
    specializations of forward iterators. This method will also return
    \c true if the container provides one of those.

    QMetaSequence assumes that const and non-const iterators for the same
    container have the same iterator traits.
 */
bool QMetaSequence::hasForwardIterator() const
{
    if (!d_ptr)
        return false;
    return d_ptr->iteratorCapabilities & QtMetaContainerPrivate::ForwardCapability;
}

/*!
    Returns \c true if the underlying container provides a bi-directional
    iterator or a random access iterator as defined by
    std::bidirectional_iterator_tag and std::random_access_iterator_tag,
    respectively. Otherwise returns \c false.

    QMetaSequence assumes that const and non-const iterators for the same
    container have the same iterator traits.
 */
bool QMetaSequence::hasBidirectionalIterator() const
{
    if (!d_ptr)
        return false;
    return d_ptr->iteratorCapabilities & QtMetaContainerPrivate::BiDirectionalCapability;
}

/*!
    Returns \c true if the underlying container provides a random access
    iterator as defined by std::random_access_iterator_tag, otherwise returns
    \c false.

    QMetaSequence assumes that const and non-const iterators for the same
    container have the same iterator traits.
 */
bool QMetaSequence::hasRandomAccessIterator() const
{
    if (!d_ptr)
        return false;
    return d_ptr->iteratorCapabilities & QtMetaContainerPrivate::RandomAccessCapability;
}

/*!
    Returns the meta type for values stored in the container.
 */
QMetaType QMetaSequence::valueMetaType() const
{
    return d_ptr ? d_ptr->valueMetaType : QMetaType();
}

/*!
    Returns \c true if the underlying container is ordered, otherwise returns
    \c false. A container is considered ordered if values added to it are
    placed in a defined location. Inserting into or adding to an ordered
    container will always succeed. Inserting into or adding to an unordered
    container may not succeed, for example if the container is a QSet that
    already contains the value being inserted.

    \sa addValue(), insertValueAtIterator(), addsAndRemovesValuesAtBegin(),
        addsAndRemovesValuesAtEnd()
 */
bool QMetaSequence::isOrdered() const
{
    if (!d_ptr)
        return false;
    return d_ptr->addRemovePosition != QtMetaContainerPrivate::QMetaSequenceInterface::Random;
}

/*!
    Returns \c true if values added using \l addValue() are placed at the
    beginning of the container, otherwise returns \c false. Likewise
    \l removeValue() removes an value from the beginning of the container
    if this method returns \c true.

    \sa addValue(), removeValue(), addsAndRemovesValuesAtEnd()
 */
bool QMetaSequence::addsAndRemovesValuesAtBegin() const
{
    if (!d_ptr)
        return false;
    return d_ptr->addRemovePosition == QtMetaContainerPrivate::QMetaSequenceInterface::AtBegin;
}

/*!
    Returns \c true if values added using \l addValue() are placed at the
    end of the container, otherwise returns \c false. Likewise
    \l removeValue() removes an value from the end of the container
    if this method returns \c true.

    \sa addValue(), removeValue(), addsAndRemovesValuesAtBegin()
 */
bool QMetaSequence::addsAndRemovesValuesAtEnd() const
{
    if (!d_ptr)
        return false;
    return d_ptr->addRemovePosition == QtMetaContainerPrivate::QMetaSequenceInterface::AtEnd;
}

/*!
    Returns \c true if the container can be queried for its size, \c false
    otherwise.

    \sa size()
 */
bool QMetaSequence::hasSize() const
{
    return d_ptr && d_ptr->sizeFn;
}

/*!
    Returns the number of values in the given \a container if it can be
    queried for its size. Otherwise returns \c -1.

    \sa hasSize()
 */
qsizetype QMetaSequence::size(const void *container) const
{
    return hasSize() ? d_ptr->sizeFn(container) : -1;
}

/*!
    Returns \c true if the container can be cleared, \c false otherwise.

    \sa clear()
 */
bool QMetaSequence::canClear() const
{
    return d_ptr && d_ptr->clearFn;
}

/*!
    Clears the given \a container if it can be cleared.

    \sa canClear()
 */
void QMetaSequence::clear(void *container) const
{
    if (canClear())
        d_ptr->clearFn(container);
}

/*!
    Returns \c true if values can be retrieved from the container by index,
    otherwise \c false.

    \sa valueAtIndex()
 */
bool QMetaSequence::canGetValueAtIndex() const
{
    return d_ptr && d_ptr->valueAtIndexFn;
}

/*!
    Retrieves the value at \a index in the \a container and places it in the
    memory location pointed to by \a result, if that is possible.

    \sa canGetValueAtIndex()
 */
void QMetaSequence::valueAtIndex(const void *container, qsizetype index, void *result) const
{
    if (canGetValueAtIndex())
        d_ptr->valueAtIndexFn(container, index, result);
}

/*!
    Returns \c true if an value can be written to the container by index,
    otherwise \c false.

    \sa setValueAtIndex()
*/
bool QMetaSequence::canSetValueAtIndex() const
{
    return d_ptr && d_ptr->setValueAtIndexFn;
}

/*!
    Overwrites the value at \a index in the \a container using the \a value
    passed as parameter if that is possible.

    \sa canSetValueAtIndex()
 */
void QMetaSequence::setValueAtIndex(void *container, qsizetype index, const void *value) const
{
    if (canSetValueAtIndex())
        d_ptr->setValueAtIndexFn(container, index, value);
}

/*!
    Returns \c true if values can be added to the container, \c false
    otherwise.

    \sa addValue(), isOrdered()
 */
bool QMetaSequence::canAddValue() const
{
    return d_ptr && d_ptr->addValueFn;
}

/*!
    Adds \a value to the \a container if possible. If \l canAddValue()
    returns \c false, the \a value is not added. Else, if
    \l addsAndRemovesValuesAtBegin() returns \c true, the \a value is added
    to the beginning of the \a container. Else, if
    \l addsAndRemovesValuesAtEnd() returns \c true, the \a value is added to
    the end of the container. Else, the value is added in an unspecified
    place or not at all. The latter is the case for adding values to an
    unordered container, for example \l QSet.

    \sa canAddValue(), addsAndRemovesValuesAtBegin(),
        addsAndRemovesValuesAtEnd(), isOrdered(), removeValue()
 */
void QMetaSequence::addValue(void *container, const void *value) const
{
    if (canAddValue())
        d_ptr->addValueFn(container, value);
}

/*!
    Returns \c true if values can be removed from the container, \c false
    otherwise.

    \sa removeValue(), isOrdered()
 */
bool QMetaSequence::canRemoveValue() const
{
    return d_ptr && d_ptr->removeValueFn;
}

/*!
    Removes an value from the \a container if possible. If
    \l canRemoveValue() returns \c false, no value is removed. Else, if
    \l addsAndRemovesValuesAtBegin() returns \c true, the first value in
    the \a container is removed. Else, if \l addsAndRemovesValuesAtEnd()
    returns \c true, the last value in the \a container is removed. Else,
    an unspecified value or nothing is removed.

    \sa canRemoveValue(), addsAndRemovesValuesAtBegin(),
        addsAndRemovesValuesAtEnd(), isOrdered(), addValue()
 */
void QMetaSequence::removeValue(void *container) const
{
    if (canRemoveValue())
        d_ptr->removeValueFn(container);
}

/*!
    Returns \c true if the underlying container offers a non-const iterator,
    \c false otherwise.

    \sa begin(), end(), destroyIterator(), compareIterator(), diffIterator(),
        advanceIterator(), copyIterator()
 */
bool QMetaSequence::hasIterator() const
{
    if (!d_ptr || !d_ptr->createIteratorFn)
        return false;
    Q_ASSERT(d_ptr->destroyIteratorFn);
    Q_ASSERT(d_ptr->compareIteratorFn);
    Q_ASSERT(d_ptr->copyIteratorFn);
    Q_ASSERT(d_ptr->advanceIteratorFn);
    Q_ASSERT(d_ptr->diffIteratorFn);
    return true;
}

/*!
    Creates and returns a non-const iterator pointing to the beginning of
    \a container. The iterator is allocated on the heap using new. It has to be
    destroyed using \l destroyIterator eventually, to reclaim the memory.

    Returns \c nullptr if the container doesn't offer any non-const iterators.

    \sa end(), constBegin(), constEnd(), destroyIterator()
 */
void *QMetaSequence::begin(void *container) const
{
    return hasIterator()
            ? d_ptr->createIteratorFn(
                  container, QtMetaContainerPrivate::QMetaSequenceInterface::AtBegin)
            : nullptr;
}

/*!
    Creates and returns a non-const iterator pointing to the end of
    \a container. The iterator is allocated on the heap using new. It has to be
    destroyed using \l destroyIterator eventually, to reclaim the memory.

    Returns \c nullptr if the container doesn't offer any non-const iterators.

    \sa hasIterator(), end(), constBegin(), constEnd(), destroyIterator()
 */
void *QMetaSequence::end(void *container) const
{
    return hasIterator()
            ? d_ptr->createIteratorFn(
                  container, QtMetaContainerPrivate::QMetaSequenceInterface::AtEnd)
            : nullptr;
}

/*!
    Destroys a non-const \a iterator previously created using \l begin() or
    \l end().

    \sa begin(), end(), destroyConstIterator()
 */
void QMetaSequence::destroyIterator(const void *iterator) const
{
    if (hasIterator())
        d_ptr->destroyIteratorFn(iterator);
}

/*!
    Returns \c true if the non-const iterators \a i and \a j point to the same
    value in the container they are iterating over, otherwise returns \c
    false.

    \sa begin(), end()
 */
bool QMetaSequence::compareIterator(const void *i, const void *j) const
{
    return hasIterator() ? d_ptr->compareIteratorFn(i, j) : false;
}

/*!
    Copies the non-const iterator \a source into the non-const iterator
    \a target. Afterwards compareIterator(target, source) returns \c true.

    \sa begin(), end()
 */
void QMetaSequence::copyIterator(void *target, const void *source) const
{
    if (hasIterator())
        d_ptr->copyIteratorFn(target, source);
}

/*!
    Advances the non-const \a iterator by \a step steps. If \a steps is negative
    the \a iterator is moved backwards, towards the beginning of the container.
    The behavior is unspecified for negative values of \a step if
    \l hasBidirectionalIterator() returns false.

    \sa begin(), end()
 */
void QMetaSequence::advanceIterator(void *iterator, qsizetype step) const
{
    if (hasIterator())
        d_ptr->advanceIteratorFn(iterator, step);
}

/*!
    Returns the distance between the non-const iterators \a i and \a j, the
    equivalent of \a i \c - \a j. If \a j is closer to the end of the container
    than \a i, the returned value is negative. The behavior is unspecified in
    this case if \l hasBidirectionalIterator() returns false.

    \sa begin(), end()
 */
qsizetype QMetaSequence::diffIterator(const void *i, const void *j) const
{
    return hasIterator() ? d_ptr->diffIteratorFn(i, j) : 0;
}

/*!
    Returns \c true if the underlying container can retrieve the value pointed
    to by a non-const iterator, \c false otherwise.

    \sa hasIterator(), valueAtIterator()
 */
bool QMetaSequence::canGetValueAtIterator() const
{
    return d_ptr && d_ptr->valueAtIteratorFn;
}

/*!
    Retrieves the value pointed to by the non-const \a iterator and stores it
    in the memory location pointed to by \a result, if possible.

    \sa canGetValueAtIterator(), begin(), end()
 */
void QMetaSequence::valueAtIterator(const void *iterator, void *result) const
{
    if (canGetValueAtIterator())
        d_ptr->valueAtIteratorFn(iterator, result);
}

/*!
    Returns \c true if the underlying container can write to the value pointed
    to by a non-const iterator, \c false otherwise.

    \sa hasIterator(), setValueAtIterator()
 */
bool QMetaSequence::canSetValueAtIterator() const
{
    return d_ptr && d_ptr->setValueAtIteratorFn;
}

/*!
    Writes \a value to the value pointed to by the non-const \a iterator, if
    possible.

    \sa canSetValueAtIterator(), begin(), end()
 */
void QMetaSequence::setValueAtIterator(const void *iterator, const void *value) const
{
    if (canSetValueAtIterator())
        d_ptr->setValueAtIteratorFn(iterator, value);
}

/*!
    Returns \c true if the underlying container can insert a new value, taking
    the location pointed to by a non-const iterator into account.

    \sa hasIterator(), insertValueAtIterator()
 */
bool QMetaSequence::canInsertValueAtIterator() const
{
    return d_ptr && d_ptr->insertValueAtIteratorFn;
}

/*!
    Inserts \a value into the \a container, if possible, taking the non-const
    \a iterator into account. If \l canInsertValueAtIterator() returns
    \c false, the \a value is not inserted. Else if \l isOrdered() returns
    \c true, the value is inserted before the value pointed to by
    \a iterator. Else, the \a value is inserted at an unspecified place or not
    at all. In the latter case, the \a iterator is taken as a hint. If it points
    to the correct place for the \a value, the operation may be faster than a
    \l addValue() without iterator.

    \sa canInsertValueAtIterator(), isOrdered(), begin(), end()
 */
void QMetaSequence::insertValueAtIterator(void *container, const void *iterator,
                                            const void *value) const
{
    if (canInsertValueAtIterator())
        d_ptr->insertValueAtIteratorFn(container, iterator, value);
}

/*!
    Returns \c true if the value pointed to by a non-const iterator can be
    erased, \c false otherwise.

    \sa hasIterator(), eraseValueAtIterator()
 */
bool QMetaSequence::canEraseValueAtIterator() const
{
    return d_ptr && d_ptr->eraseValueAtIteratorFn;
}

/*!
    Erases the value pointed to by the non-const \a iterator from the
    \a container, if possible.

    \sa canEraseValueAtIterator(), begin(), end()
 */
void QMetaSequence::eraseValueAtIterator(void *container, const void *iterator) const
{
    if (canEraseValueAtIterator())
        d_ptr->eraseValueAtIteratorFn(container, iterator);
}

/*!
    Returns \c true if the underlying container offers a const iterator,
    \c false otherwise.

    \sa constBegin(), constEnd(), destroyConstIterator(),
        compareConstIterator(), diffConstIterator(), advanceConstIterator(),
        copyConstIterator()
 */
bool QMetaSequence::hasConstIterator() const
{
    if (!d_ptr || !d_ptr->createConstIteratorFn)
        return false;
    Q_ASSERT(d_ptr->destroyConstIteratorFn);
    Q_ASSERT(d_ptr->compareConstIteratorFn);
    Q_ASSERT(d_ptr->copyConstIteratorFn);
    Q_ASSERT(d_ptr->advanceConstIteratorFn);
    Q_ASSERT(d_ptr->diffConstIteratorFn);
    return true;
}

/*!
    Creates and returns a const iterator pointing to the beginning of
    \a container. The iterator is allocated on the heap using new. It has to be
    destroyed using \l destroyConstIterator eventually, to reclaim the memory.

    Returns \c nullptr if the container doesn't offer any const iterators.

    \sa constEnd(), begin(), end(), destroyConstIterator()
 */
void *QMetaSequence::constBegin(const void *container) const
{
    return hasConstIterator()
            ? d_ptr->createConstIteratorFn(
                  container, QtMetaContainerPrivate::QMetaSequenceInterface::AtBegin)
            : nullptr;
}

/*!
    Creates and returns a const iterator pointing to the end of
    \a container. The iterator is allocated on the heap using new. It has to be
    destroyed using \l destroyConstIterator eventually, to reclaim the memory.

    Returns \c nullptr if the container doesn't offer any const iterators.

    \sa constBegin(), begin(), end(), destroyConstIterator()
 */
void *QMetaSequence::constEnd(const void *container) const
{
    return hasConstIterator()
            ? d_ptr->createConstIteratorFn(
                  container, QtMetaContainerPrivate::QMetaSequenceInterface::AtEnd)
            : nullptr;
}

/*!
    Destroys a const \a iterator previously created using \l constBegin() or
    \l constEnd().

    \sa constBegin(), constEnd(), destroyIterator()
 */
void QMetaSequence::destroyConstIterator(const void *iterator) const
{
    if (hasConstIterator())
        d_ptr->destroyConstIteratorFn(iterator);
}

/*!
    Returns \c true if the const iterators \a i and \a j point to the same
    value in the container they are iterating over, otherwise returns \c
    false.

    \sa constBegin(), constEnd()
 */
bool QMetaSequence::compareConstIterator(const void *i, const void *j) const
{
    return hasConstIterator() ? d_ptr->compareConstIteratorFn(i, j) : false;
}

/*!
    Copies the const iterator \a source into the const iterator
    \a target. Afterwards compareConstIterator(target, source) returns \c true.

    \sa constBegin(), constEnd()
 */
void QMetaSequence::copyConstIterator(void *target, const void *source) const
{
    if (hasConstIterator())
        d_ptr->copyConstIteratorFn(target, source);
}

/*!
    Advances the const \a iterator by \a step steps. If \a steps is negative
    the \a iterator is moved backwards, towards the beginning of the container.
    The behavior is unspecified for negative values of \a step if
    \l hasBidirectionalIterator() returns false.

    \sa constBegin(), constEnd()
 */
void QMetaSequence::advanceConstIterator(void *iterator, qsizetype step) const
{
    if (hasConstIterator())
        d_ptr->advanceConstIteratorFn(iterator, step);
}

/*!
    Returns the distance between the const iterators \a i and \a j, the
    equivalent of \a i \c - \a j. If \a j is closer to the end of the container
    than \a i, the returned value is negative. The behavior is unspecified in
    this case if \l hasBidirectionalIterator() returns false.

    \sa constBegin(), constEnd()
 */
qsizetype QMetaSequence::diffConstIterator(const void *i, const void *j) const
{
    return hasConstIterator() ?  d_ptr->diffConstIteratorFn(i, j) : 0;
}

/*!
    Returns \c true if the underlying container can retrieve the value pointed
    to by a const iterator, \c false otherwise.

    \sa hasConstIterator(), valueAtConstIterator()
 */
bool QMetaSequence::canGetValueAtConstIterator() const
{
    return d_ptr && d_ptr->valueAtConstIteratorFn;
}

/*!
    Retrieves the value pointed to by the const \a iterator and stores it
    in the memory location pointed to by \a result, if possible.

    \sa canGetValueAtConstIterator(), constBegin(), constEnd()
 */
void QMetaSequence::valueAtConstIterator(const void *iterator, void *result) const
{
    if (canGetValueAtConstIterator())
        d_ptr->valueAtConstIteratorFn(iterator, result);
}

/*!
    \fn bool operator==(QMetaSequence a, QMetaSequence b)
    \since 6.0
    \relates QMetaSequence

    Returns \c true if the QMetaSequence \a a represents the same container type
    as the QMetaSequence \a b, otherwise returns \c false.
*/

/*!
    \fn bool operator!=(QMetaSequence a, QMetaSequence b)
    \since 6.0
    \relates QMetaSequence

    Returns \c true if the QMetaSequence \a a represents a different container
    type than the QMetaSequence \a b, otherwise returns \c false.
*/

QT_END_NAMESPACE
