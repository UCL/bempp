// Copyright (C) 2011-2012 by the BEM++ Authors
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

#ifndef bempp_boundary_operator_hpp
#define bempp_boundary_operator_hpp

#include "../common/common.hpp"
#include "../common/shared_ptr.hpp"
#include "transposition_mode.hpp"

#include <boost/mpl/set.hpp>
#include <boost/mpl/has_key.hpp>
#include <boost/utility/enable_if.hpp>
#include <string>

namespace Bempp
{

template <typename BasisFunctionType> class Space;
template <typename ResultType> class DiscreteBoundaryOperator;
template <typename BasisFunctionType, typename ResultType> class AbstractBoundaryOperator;
template <typename BasisFunctionType, typename ResultType> class Context;
template <typename BasisFunctionType, typename ResultType> class GridFunction;

/** \brief Operator acting on functions defined on a surface.
 *
 *  \note Different threads should not share BoundaryOperator objects, since
 *  notably the weakForm() function is not thread-safe. Instead, each thread
 *  should hold its own copy of a BoundaryOperator (note that copying
 *  BoundaryOperators is cheap).
 */
template <typename BasisFunctionType, typename ResultType>
class BoundaryOperator
{
public:
    BoundaryOperator();

    BoundaryOperator(const shared_ptr<const Context<
                     BasisFunctionType, ResultType> >& context,
                     const shared_ptr<const AbstractBoundaryOperator<
                     BasisFunctionType, ResultType> >& abstractOp);

    void initialize(const shared_ptr<const Context<
                    BasisFunctionType, ResultType> >& context,
                    const shared_ptr<const AbstractBoundaryOperator<
                    BasisFunctionType, ResultType> >& abstractOp);
    void uninitialize();
    bool isInitialized() const;

    shared_ptr<const AbstractBoundaryOperator<BasisFunctionType, ResultType> >
    abstractOperator() const;

    shared_ptr<const Context<BasisFunctionType, ResultType> > context() const;

    shared_ptr<const DiscreteBoundaryOperator<ResultType> > weakForm() const;

    shared_ptr<const Space<BasisFunctionType> > domain() const;
    shared_ptr<const Space<BasisFunctionType> > range() const;
    shared_ptr<const Space<BasisFunctionType> > dualToRange() const;

    std::string label() const;

    /** \brief Set <tt>y_inout := alpha * A * x_in + beta * y_inout</tt>, where
     *  \c A is this operator. */
    void apply(const TranspositionMode trans,
               const GridFunction<BasisFunctionType, ResultType>& x_in,
               GridFunction<BasisFunctionType, ResultType>& y_inout,
               ResultType alpha, ResultType beta) const;

private:
    shared_ptr<const Context<BasisFunctionType, ResultType> > m_context;
    shared_ptr<const AbstractBoundaryOperator<BasisFunctionType, ResultType> >
    m_abstractOp;
    mutable shared_ptr<const DiscreteBoundaryOperator<ResultType> > m_weakForm;
};

template <typename BasisFunctionType, typename ResultType>
BoundaryOperator<BasisFunctionType, ResultType> operator+(
        const BoundaryOperator<BasisFunctionType, ResultType>& op1,
        const BoundaryOperator<BasisFunctionType, ResultType>& op2);

template <typename BasisFunctionType, typename ResultType>
BoundaryOperator<BasisFunctionType, ResultType> operator-(
        const BoundaryOperator<BasisFunctionType, ResultType>& op1,
        const BoundaryOperator<BasisFunctionType, ResultType>& op2);

// This type machinery is needed to disambiguate between this operator and
// the one taking a BoundaryOperator and a GridFunction
template <typename BasisFunctionType, typename ResultType, typename ScalarType>
typename boost::enable_if<
    typename boost::mpl::has_key<
        boost::mpl::set<float, double, std::complex<float>, std::complex<double> >,
        ScalarType>,
    BoundaryOperator<BasisFunctionType, ResultType> >::type
operator*(
        const BoundaryOperator<BasisFunctionType, ResultType>& op,
        const ScalarType& scalar);

template <typename BasisFunctionType, typename ResultType, typename ScalarType>
BoundaryOperator<BasisFunctionType, ResultType> operator*(
        const ScalarType& scalar,
        const BoundaryOperator<BasisFunctionType, ResultType>& op);

template <typename BasisFunctionType, typename ResultType, typename ScalarType>
BoundaryOperator<BasisFunctionType, ResultType> operator/(
        const BoundaryOperator<BasisFunctionType, ResultType>& op,
        const ScalarType& scalar);

template <typename BasisFunctionType, typename ResultType>
GridFunction<BasisFunctionType, ResultType> operator*(
        const BoundaryOperator<BasisFunctionType, ResultType>& op,
        const GridFunction<BasisFunctionType, ResultType>& fun);

//template <typename BasisFunctionType, typename ResultType>
//BoundaryOperatorComposition<BasisFunctionType, ResultType> operator*(
//        const BoundaryOperator<BasisFunctionType, ResultType>& op1,
//        const BoundaryOperator<BasisFunctionType, ResultType>& op2);

} // namespace Bempp

#endif