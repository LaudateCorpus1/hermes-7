#include "definitions.h"

using namespace Hermes;
using namespace Hermes::Hermes2D;

// This example shows how to solve a simple PDE that describes stationary
// heat transfer in an object consisting of two materials (aluminum and
// copper). The object is heated by constant volumetric heat sources
// (generated, for example, by a DC electric current). The temperature
// on the boundary is fixed. We will learn how to:
//
//   - load the mesh,
//   - perform initial refinements,
//   - create a H1 space over the mesh,
//   - define weak formulation,
//   - initialize matrix solver,
//   - assemble and solve the matrix system,
//   - output the solution and element orders in VTK format
//     (to be visualized, e.g., using Paraview),
//   - visualize the solution using Hermes' native OpenGL-based functionality.
//
// PDE: Poisson equation -div(LAMBDA grad u) - VOLUME_HEAT_SRC = 0.
//
// Boundary conditions: Dirichlet u(x, y) = FIXED_BDY_TEMP on the boundary.
//
// Geometry: L-Shape domain (see file domain.mesh).
//
// The following parameters can be changed:

// Set to "false" to suppress Hermes OpenGL visualization.
const bool HERMES_VISUALIZATION = true;
// Set to "true" to enable VTK output.
const bool VTK_VISUALIZATION = false;
// Uniform polynomial degree of mesh elements.
const int P_INIT = 3;
// Number of initial uniform mesh refinements.
const int INIT_REF_NUM = 3;

// Problem parameters.
// Thermal cond. of Al for temperatures around 20 deg Celsius.
const double LAMBDA_AL = 236.0;
// Thermal cond. of Cu for temperatures around 20 deg Celsius.
const double LAMBDA_CU = 386.0;
// Volume heat sources generated (for example) by electric current.
const double VOLUME_HEAT_SRC = 5;
// Fixed temperature on the boundary.
const double FIXED_BDY_TEMP = 20;

int main(int argc, char* argv[])
{
#ifdef WITH_PARALUTION
  HermesCommonApi.set_integral_param_value(matrixSolverType, SOLVER_PARALUTION_ITERATIVE);
#endif

  // Load the mesh.
  MeshSharedPtr mesh(new Mesh);
  Hermes::Hermes2D::MeshReaderH2DXML mloader;
  mloader.load("mesh.msh", std::vector<MeshSharedPtr>({ mesh }));

  // Refine all elements, do it INIT_REF_NUM-times.
  for (unsigned int i = 0; i < INIT_REF_NUM; i++)
    mesh->refine_all_elements();

  // Initialize essential boundary conditions.
  Hermes::Hermes2D::DefaultEssentialBCConst<double> bc_essential(std::vector<std::string>({ "0", "1", "2", "3", "4", "5", "6" }), 1000);
  Hermes::Hermes2D::EssentialBCs<double> bcs(&bc_essential);

  // Initialize space.
  SpaceSharedPtr<double> space(new Hermes::Hermes2D::H1Space<double>(mesh, &bcs, P_INIT));

  std::cout << "Ndofs: " << space->get_num_dofs() << std::endl;

  // Initialize the weak formulation.
  WeakFormSharedPtr<double> wf(new CustomWeakFormPoisson("Aluminum", new Hermes::Hermes1DFunction<double>(LAMBDA_AL), "Copper",
    new Hermes::Hermes1DFunction<double>(LAMBDA_CU), new Hermes::Hermes2DFunction<double>(VOLUME_HEAT_SRC)));

  // Initialize the solution.
  MeshFunctionSharedPtr<double> sln(new Solution<double>);

  // Initialize linear solver.
  Hermes::Hermes2D::LinearSolver<double> linear_solver(wf, space);
  linear_solver.output_matrix();
  linear_solver.output_rhs();

  // Solve the linear problem.
  try
  {
    linear_solver.solve();

    // Get the solution vector.
    double* sln_vector = linear_solver.get_sln_vector();

    // Translate the solution vector into the previously initialized Solution.
    Hermes::Hermes2D::Solution<double>::vector_to_solution(sln_vector, space, sln);

    // VTK output.
    if (VTK_VISUALIZATION)
    {
      // Output solution in VTK format.
      Hermes::Hermes2D::Views::Linearizer lin(FileExport);
      bool mode_3D = false;
      lin.save_solution_vtk(sln, "sln.vtk", "Temperature", mode_3D, 1);

      // Output mesh and element orders in VTK format.
      Hermes::Hermes2D::Views::Orderizer ord;
      ord.save_mesh_vtk(space, "mesh.vtk");
      ord.save_orders_vtk(space, "ord.vtk");
      ord.save_markers_vtk(space, "markers.vtk");
    }

    if (HERMES_VISUALIZATION)
    {
      // Visualize the solution.
      Hermes::Hermes2D::Views::ScalarView viewS("Solution", new Hermes::Hermes2D::Views::WinGeom(0, 0, 500, 400));
      Hermes::Hermes2D::Views::OrderView viewSp("Space", new Hermes::Hermes2D::Views::WinGeom(0, 400, 500, 400));
      viewS.get_linearizer()->set_criterion(Views::LinearizerCriterionFixed(0));
      viewS.show(sln);
      viewSp.show(space);
      viewS.wait_for_close();
    }
  }
  catch (Exceptions::Exception& e)
  {
    std::cout << e.info();
  }
  catch (std::exception& e)
  {
    std::cout << e.what();
  }
  return 0;
}