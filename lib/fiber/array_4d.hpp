// Copyright (C) 2011-2012 by the Bem++ Authors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef fiber_array_4d_hpp
#define fiber_array_4d_hpp

#include "../common/common.hpp"

#include <stdexcept>

namespace Fiber {

/** \brief Simple implementation of a 4D Fortran-ordered array.

Bound checking can optionally be activated by defining the symbol
FIBER_CHECK_ARRAY_BOUNDS. */
template <typename T>
class Array4d
{
public:
    Array4d();
    Array4d(size_t extent0, size_t extent1, size_t extent2, size_t extent3);
    Array4d(size_t extent0, size_t extent1, size_t extent2, size_t extent3, T* data);

    ~Array4d();

    T& operator()(size_t index0, size_t index1, size_t index2, size_t index3);
    const T& operator()(size_t index0, size_t index1, size_t index2, size_t index3) const;

    size_t extent(size_t dimension) const;
    void set_size(size_t extent0, size_t extent1, size_t extent2, size_t extent3);

    typedef T* iterator;
    typedef const T* const_iterator;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

private:
#ifdef FIBER_CHECK_ARRAY_BOUNDS
    void check_dimension(size_t dimension) const;
    void check_extents(size_t extent0, size_t extent1, size_t extent2, size_t extent3) const;
    void check_indices(size_t index0, size_t index1, size_t index2, size_t index3) const;
#endif

private:
    // Disable copy constructor and assignment operator
    Array4d(const Array4d& rhs);
    Array4d& operator=(const Array4d& rhs);

private:
    size_t m_extents[4];
    bool m_owns;
    T* m_storage;
};

} // namespace Fiber

#include "array_4d_imp.hpp"

#endif