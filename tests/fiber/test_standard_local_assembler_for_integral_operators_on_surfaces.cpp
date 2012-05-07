// Copyright (C) 2011 by the BEM++ Authors
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

#include "../type_template.hpp"
#include "../check_arrays_are_close.hpp"

#include "assembly/laplace_3d_single_layer_potential.hpp"
#include "assembly/standard_local_assembler_factory_for_operators_on_surfaces.hpp"
#include "common/scalar_traits.hpp"
#include "fiber/geometrical_data.hpp"
#include "fiber/laplace_3d_single_layer_potential_kernel.hpp"
#include "grid/grid.hpp"
#include "grid/grid_factory.hpp"
#include "grid/grid_view.hpp"
#include "space/piecewise_constant_scalar_space.hpp"
#include "space/piecewise_linear_continuous_scalar_space.hpp"

#include "grid/entity.hpp"
#include "grid/mapper.hpp"

#include <algorithm>
#include <armadillo>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/version.hpp>
#include <complex>

using namespace Bempp;

const int N_ELEMENTS_X = 2, N_ELEMENTS_Y = 3;

/** \brief Fixture class. */
template <typename BFT, typename RT>
class StandardLocalAssemblerForIntegralOperatorsOnSurfacesManager
{
public:
    typedef typename ScalarTraits<RT>::RealType CT;
    typedef PiecewiseConstantScalarSpace<BFT> TestSpace;
    typedef PiecewiseLinearContinuousScalarSpace<BFT> TrialSpace;
    typedef Laplace3dSingleLayerPotential<BFT, RT> Operator;
    typedef StandardLocalAssemblerFactoryForOperatorsOnSurfaces<BFT, RT> AssemblerFactory;
    typedef Fiber::RawGridGeometry<CT> RawGridGeometry;

    StandardLocalAssemblerForIntegralOperatorsOnSurfacesManager(
            bool cacheSingularIntegrals) :
        openClHandler(Fiber::OpenClOptions())
    {
        // Create a Bempp grid
        grid = createGrid();

        testSpace = std::auto_ptr<TestSpace>(new TestSpace(*grid));
        trialSpace = std::auto_ptr<TrialSpace>(new TrialSpace(*grid));
        testSpace->assignDofs();
        trialSpace->assignDofs();

        op = std::auto_ptr<Operator>(new Operator(*testSpace, *trialSpace));

        // Construct local assembler

        // Gather geometric data
        rawGeometry = std::auto_ptr<RawGridGeometry>(
                    new RawGridGeometry(grid->dim(), grid->dimWorld()));
        std::auto_ptr<GridView> view = grid->leafView();
        view->getRawElementData(
                    rawGeometry->vertices(), rawGeometry->elementCornerIndices(),
                    rawGeometry->auxData());

        geometryFactory = grid->elementGeometryFactory();

        getAllBases(*testSpace, testBases);
        getAllBases(*trialSpace, trialBases);

        Fiber::AccuracyOptions options;
        options.doubleRegular.orderIncrement = 1;
        assemblerFactory = std::auto_ptr<AssemblerFactory>(new AssemblerFactory);

        Fiber::ParallelisationOptions parallelOptions;
        assembler = op->makeAssembler(
                    *assemblerFactory, *geometryFactory, *rawGeometry,
                    testBases, trialBases, openClHandler, parallelOptions,
                    cacheSingularIntegrals);
    }

private:
    std::auto_ptr<Bempp::Grid> createGrid() {
        Bempp::GridParameters params;
        params.topology = Bempp::GridParameters::TRIANGULAR;

        const int dimGrid = 2;
        typedef double ctype;
        arma::Col<double> lowerLeft(dimGrid);
        arma::Col<double> upperRight(dimGrid);
        arma::Col<unsigned int> nElements(dimGrid);
        lowerLeft.fill(0);
        upperRight.fill(1);
        nElements(0) = N_ELEMENTS_X;
        nElements(1) = N_ELEMENTS_Y;

        return Bempp::GridFactory::createStructuredGrid(
                    params, lowerLeft, upperRight, nElements);
    }

public:
    std::auto_ptr<Bempp::Grid> grid;
    std::auto_ptr<TestSpace> testSpace;
    std::auto_ptr<TrialSpace> trialSpace;
    std::auto_ptr<Operator> op;

    std::auto_ptr<AssemblerFactory> assemblerFactory;
    std::auto_ptr<GeometryFactory> geometryFactory;
    std::auto_ptr<RawGridGeometry> rawGeometry;
    std::vector<const Fiber::Basis<BFT>*> testBases, trialBases;
    Fiber::OpenClHandler openClHandler;

    std::auto_ptr<typename Operator::LocalAssembler> assembler;
};

// Tests

BOOST_AUTO_TEST_SUITE(StandardLocalAssemblerForIntegralOperatorsOnSurfaces)

template <typename ResultType>
void
both_variants_of_evaluateLocalWeakForms_agree_for_callVariant_TEST_TRIAL_and_cacheSingularIntegrals(
        bool cacheSingularIntegrals)
{
    StandardLocalAssemblerForIntegralOperatorsOnSurfacesManager<
            typename ScalarTraits<ResultType>::RealType, ResultType> mgr(
                cacheSingularIntegrals);

    const int elementCount = N_ELEMENTS_X * N_ELEMENTS_Y * 2;
    const int testIndexCount = elementCount, trialIndexCount = elementCount;
    std::vector<int> testIndices(testIndexCount);
    for (int i = 0; i < testIndexCount; ++i)
        testIndices[i] = i;
    std::vector<int> trialIndices(trialIndexCount);
    for (int i = 0; i < trialIndexCount; ++i)
        trialIndices[i] = i;

    Fiber::Array2d<arma::Mat<ResultType> > resultVariant2;
    mgr.assembler->evaluateLocalWeakForms(testIndices, trialIndices, resultVariant2);

    // Gather successive columns
    Fiber::Array2d<arma::Mat<ResultType> > resultVariant1(
                testIndexCount, trialIndexCount);
    std::vector<arma::Mat<ResultType> > colResult;
    for (int trialI = 0; trialI < trialIndexCount; ++trialI)
    {
        mgr.assembler->evaluateLocalWeakForms(Fiber::TEST_TRIAL, testIndices,
                                              trialIndices[trialI],
                                              Fiber::ALL_DOFS, colResult);
        for (int testI = 0; testI < testIndexCount; ++testI)
            resultVariant1(testI, trialI) = colResult[testI];
    }

    BOOST_CHECK(check_arrays_are_close<ResultType>(
                    resultVariant1, resultVariant2, 1e-6));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
        both_variants_of_evaluateLocalWeakForms_agree_for_callVariant_TEST_TRIAL_and_cacheSingularIntegrals_false,
        ResultType, result_types)
{
    both_variants_of_evaluateLocalWeakForms_agree_for_callVariant_TEST_TRIAL_and_cacheSingularIntegrals<
            ResultType>(false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
        both_variants_of_evaluateLocalWeakForms_agree_for_callVariant_TEST_TRIAL_and_cacheSingularIntegrals_true,
        ResultType, result_types)
{
    both_variants_of_evaluateLocalWeakForms_agree_for_callVariant_TEST_TRIAL_and_cacheSingularIntegrals<
            ResultType>(true);
}

template <typename ResultType>
void
both_variants_of_evaluateLocalWeakForms_agree_for_callVariant_TRIAL_TEST_and_cacheSingularIntegrals(
        bool cacheSingularIntegrals)
{
    StandardLocalAssemblerForIntegralOperatorsOnSurfacesManager<
            typename ScalarTraits<ResultType>::RealType, ResultType> mgr(
                cacheSingularIntegrals);

    const int elementCount = N_ELEMENTS_X * N_ELEMENTS_Y * 2;
    const int testIndexCount = elementCount, trialIndexCount = elementCount;
    std::vector<int> testIndices(testIndexCount);
    for (int i = 0; i < testIndexCount; ++i)
        testIndices[i] = i;
    std::vector<int> trialIndices(trialIndexCount);
    for (int i = 0; i < trialIndexCount; ++i)
        trialIndices[i] = i;

    Fiber::Array2d<arma::Mat<ResultType> > resultVariant2;
    mgr.assembler->evaluateLocalWeakForms(testIndices, trialIndices, resultVariant2);

    // Gather successive rows
    Fiber::Array2d<arma::Mat<ResultType> > resultVariant1(
                testIndexCount, trialIndexCount);
    std::vector<arma::Mat<ResultType> > rowResult;
    for (int testI = 0; testI < testIndexCount; ++testI)
    {
        mgr.assembler->evaluateLocalWeakForms(Fiber::TRIAL_TEST, trialIndices,
                                              testIndices[testI],
                                              Fiber::ALL_DOFS, rowResult);
        for (int trialI = 0; trialI < trialIndexCount; ++trialI)
            resultVariant1(testI, trialI) = rowResult[trialI];
    }

    BOOST_CHECK(check_arrays_are_close<ResultType>(
                    resultVariant1, resultVariant2, 1e-6));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
        both_variants_of_evaluateLocalWeakForms_agree_for_callVariant_TRIAL_TEST_and_cacheSingularIntegrals_false,
        ResultType, result_types)
{
    both_variants_of_evaluateLocalWeakForms_agree_for_callVariant_TRIAL_TEST_and_cacheSingularIntegrals<
            ResultType>(false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
        both_variants_of_evaluateLocalWeakForms_agree_for_callVariant_TRIAL_TEST_and_cacheSingularIntegrals_true,
        ResultType, result_types)
{
    both_variants_of_evaluateLocalWeakForms_agree_for_callVariant_TRIAL_TEST_and_cacheSingularIntegrals<
            ResultType>(true);
}

template <typename ResultType>
void
evaluateLocalWeakForms_for_ALL_DOFS_and_single_dof_agree_for_callVariant_TEST_TRIAL_and_cacheSingularIntegrals(
        bool cacheSingularIntegrals)
{
    StandardLocalAssemblerForIntegralOperatorsOnSurfacesManager<
            typename ScalarTraits<ResultType>::RealType, ResultType> mgr(
                cacheSingularIntegrals);

    const int elementCount = N_ELEMENTS_X * N_ELEMENTS_Y * 2;
    std::vector<int> elementIndicesA(elementCount);
    for (int i = 0; i < elementCount; ++i)
        elementIndicesA[i] = i;

    int elementIndexB = 2;

    std::vector<arma::Mat<ResultType> > completeResult;
    mgr.assembler->evaluateLocalWeakForms(Fiber::TEST_TRIAL, elementIndicesA,
                                          elementIndexB,
                                          Fiber::ALL_DOFS, completeResult);
    int elementBDofCount = completeResult[0].n_cols;

    // Gather successive rows
    std::vector<arma::Mat<ResultType> > resultForSingleDof;
    std::vector<arma::Mat<ResultType> > expected(elementCount);
    for (int i = 0; i < elementCount; ++i)
        expected[i].set_size(completeResult[i].n_rows, completeResult[i].n_cols);

    for (int dof = 0; dof < elementBDofCount; ++dof)
    {
        mgr.assembler->evaluateLocalWeakForms(Fiber::TEST_TRIAL, elementIndicesA,
                                              elementIndexB,
                                              dof, resultForSingleDof);
        for (int i = 0; i < elementCount; ++i)
            expected[i].col(dof) = resultForSingleDof[i];
    }

    BOOST_CHECK(check_arrays_are_close<ResultType>(
                    completeResult, expected, 1e-6));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
        evaluateLocalWeakForms_for_ALL_DOFS_and_single_dof_agree_for_callVariant_TEST_TRIAL_and_cacheSingularIntegrals_false,
        ResultType, result_types)
{
    evaluateLocalWeakForms_for_ALL_DOFS_and_single_dof_agree_for_callVariant_TEST_TRIAL_and_cacheSingularIntegrals<
            ResultType>(false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
        evaluateLocalWeakForms_for_ALL_DOFS_and_single_dof_agree_for_callVariant_TEST_TRIAL_and_cacheSingularIntegrals_true,
        ResultType, result_types)
{
    evaluateLocalWeakForms_for_ALL_DOFS_and_single_dof_agree_for_callVariant_TEST_TRIAL_and_cacheSingularIntegrals<
            ResultType>(true);
}

template <typename ResultType>
void
evaluateLocalWeakForms_for_ALL_DOFS_and_single_dof_agree_for_callVariant_TRIAL_TEST_and_cacheSingularIntegrals(
        bool cacheSingularIntegrals)
{
    StandardLocalAssemblerForIntegralOperatorsOnSurfacesManager<
            typename ScalarTraits<ResultType>::RealType, ResultType> mgr(
                cacheSingularIntegrals);

    const int elementCount = N_ELEMENTS_X * N_ELEMENTS_Y * 2;
    std::vector<int> elementIndicesA(elementCount);
    for (int i = 0; i < elementCount; ++i)
        elementIndicesA[i] = i;

    int elementIndexB = 2;

    std::vector<arma::Mat<ResultType> > completeResult;
    mgr.assembler->evaluateLocalWeakForms(Fiber::TRIAL_TEST, elementIndicesA,
                                          elementIndexB,
                                          Fiber::ALL_DOFS, completeResult);
    int elementBDofCount = completeResult[0].n_rows;

    // Gather successive rows
    std::vector<arma::Mat<ResultType> > resultForSingleDof;
    std::vector<arma::Mat<ResultType> > expected(elementCount);
    for (int i = 0; i < elementCount; ++i)
        expected[i].set_size(completeResult[i].n_rows, completeResult[i].n_cols);

    for (int dof = 0; dof < elementBDofCount; ++dof)
    {
        mgr.assembler->evaluateLocalWeakForms(Fiber::TRIAL_TEST, elementIndicesA,
                                              elementIndexB,
                                              dof, resultForSingleDof);
        for (int i = 0; i < elementCount; ++i)
            expected[i].row(dof) = resultForSingleDof[i];
    }

    BOOST_CHECK(check_arrays_are_close<ResultType>(
                    completeResult, expected, 1e-6));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
        evaluateLocalWeakForms_for_ALL_DOFS_and_single_dof_agree_for_callVariant_TRIAL_TEST_and_cacheSingularIntegrals_false,
        ResultType, result_types)
{
    evaluateLocalWeakForms_for_ALL_DOFS_and_single_dof_agree_for_callVariant_TRIAL_TEST_and_cacheSingularIntegrals<
            ResultType>(false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(
        evaluateLocalWeakForms_for_ALL_DOFS_and_single_dof_agree_for_callVariant_TRIAL_TEST_and_cacheSingularIntegrals_true,
        ResultType, result_types)
{
    evaluateLocalWeakForms_for_ALL_DOFS_and_single_dof_agree_for_callVariant_TRIAL_TEST_and_cacheSingularIntegrals<
            ResultType>(true);
}

template <typename ResultType>
void
evaluateLocalWeakForms_with_and_without_singular_integral_caching_gives_same_results(
        bool cacheSingularIntegrals)
{
    const int elementCount = N_ELEMENTS_X * N_ELEMENTS_Y * 2;
    const int testIndexCount = elementCount, trialIndexCount = elementCount;
    std::vector<int> testIndices(testIndexCount);
    for (int i = 0; i < testIndexCount; ++i)
        testIndices[i] = i;
    std::vector<int> trialIndices(trialIndexCount);
    for (int i = 0; i < trialIndexCount; ++i)
        trialIndices[i] = i;

    Fiber::Array2d<arma::Mat<ResultType> > resultWithCaching;
    Fiber::Array2d<arma::Mat<ResultType> > resultWithoutCaching;

    {
        StandardLocalAssemblerForIntegralOperatorsOnSurfacesManager<
                typename ScalarTraits<ResultType>::RealType, ResultType> mgr(
                    true);
        mgr.assembler->evaluateLocalWeakForms(testIndices, trialIndices,
                                              resultWithCaching);
    }
    {
        StandardLocalAssemblerForIntegralOperatorsOnSurfacesManager<
            typename ScalarTraits<ResultType>::RealType, ResultType> mgr(
                false);
        mgr.assembler->evaluateLocalWeakForms(testIndices, trialIndices,
                                              resultWithoutCaching);
    }

        BOOST_CHECK(check_arrays_are_close<ResultType>(
                    resultWithCaching, resultWithoutCaching, 1e-6));
}

BOOST_AUTO_TEST_SUITE_END()