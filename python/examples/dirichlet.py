#!/usr/bin/env python

# This script imports a grid in the Gmsh format and exports the z coordinate 
# of each vertex to the file "output.vtu" in the VTK format.

# Help Python find the bempp module
import sys
sys.path.append("..")

import bempp
import numpy as np


def evalDirichletData(point):
    return -1

print "Importing grid..."
grid = bempp.GridFactory.importGmshGrid("triangular",
                                        "../../examples/meshes/sphere-152.msh")

factory = bempp.numericalQuadratureStrategy("float64", "float64")
options = bempp.AssemblyOptions()
options.switchToAca(bempp.AcaOptions())

context = bempp.context(factory, options)

pwiseConstants = bempp.piecewiseConstantScalarSpace(context, grid)
pwiseLinears = bempp.piecewiseLinearContinuousScalarSpace(context, grid)
pwiseConstants.assignDofs()
pwiseLinears.assignDofs()

slpOp = bempp.laplace3dSingleLayerBoundaryOperator(
    context, pwiseConstants, pwiseLinears, pwiseConstants)
dlpOp = bempp.laplace3dDoubleLayerBoundaryOperator(
    context, pwiseLinears, pwiseLinears, pwiseConstants)
idOp = bempp.identityOperator(
    context, pwiseLinears, pwiseLinears, pwiseConstants)

lhsOp = slpOp
rhsOp = -0.5 * idOp + dlpOp

print "Evaluating Dirichlet data..."
dirichletData = bempp.gridFunctionFromSurfaceNormalIndependentFunction(
    context, pwiseLinears, pwiseLinears, evalDirichletData)

rhs = rhsOp * dirichletData

solver = bempp.defaultIterativeSolver(lhsOp)
params = bempp.defaultGmresParameterList(1e-5)
solver.initializeSolver(params)
solution = solver.solve(rhs)
neumannData = solution.gridFunction()

neumannData.exportToVtk("vertex_data","neumann_data", "neumann_data_vertex")
neumannData.exportToVtk("cell_data","neumann_data", "neumann_data_cell")
