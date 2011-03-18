#define HERMES_REPORT_ALL
#define HERMES_REPORT_FILE "application.log"
#include "hermes2d.h"

// This example explains how to create multiple spaces over a mesh and use them
// to solve a simple problem of linear elasticity. Note how Tuples are used, 
// they replace variable-length argument lists. At the end, VonMises filter is 
// used to visualize the stress.
//
// PDE: Lame equations of linear elasticity.
//
// BC: du_1/dn = f_0 on Gamma_3 and du_1/dn = 0 on Gamma_2, Gamma_4, Gamma_5,
//     du_2/dn = f_1 on Gamma_3 and du_2/dn = 0 on Gamma_2, Gamma_4, Gamma_5,
//     u_1 = 0 and u_2 = 0 on Gamma_1.
//
// The following parameters can be changed:

const int P_INIT = 6;                                      // Initial polynomial degree of all elements.
MatrixSolverType matrix_solver = SOLVER_UMFPACK;           // Possibilities: SOLVER_AMESOS, SOLVER_AZTECOO, SOLVER_MUMPS,
                                                           // SOLVER_PETSC, SOLVER_SUPERLU, SOLVER_UMFPACK.

// Boundary markers.
const std::string BDY_1 = "1", BDY_2 = "2", BDY_3 = "3", BDY_4 = "4", BDY_5 = "5";

// Problem parameters.
const double E  = 200e9;                                   // Young modulus (steel).
const double nu = 0.3;                                     // Poisson ratio.
const double f_0  = 0;                                     // External force in x-direction.
const double f_1  = 1e4;                                   // External force in y-direction.
const double lambda = (E * nu) / ((1 + nu) * (1 - 2*nu));  // First Lame constant.
const double mu = E / (2*(1 + nu));                        // Second Lame constant.

// Weak forms.
#include "forms.cpp"

int main(int argc, char* argv[])
{
  // Load the mesh.
  Mesh mesh;
  H2DReader mloader;
  mloader.load("domain.mesh", &mesh);

  // Perform uniform mesh refinement.
  mesh.refine_all_elements();

  // Initialize boundary conditions.
  DirichletConstantBoundaryCondition bc_dirichlet(BDY_1, 0.0);
  NaturalBoundaryCondition bc_natural(Hermes::vector<std::string>(BDY_2, BDY_3, BDY_4, BDY_5));
  BoundaryConditions bcs(Hermes::vector<BoundaryCondition*>(&bc_dirichlet, &bc_natural));

  // Create x- and y- displacement space using the default H1 shapeset.
  H1Space u_space(&mesh, &bcs, P_INIT);
  H1Space v_space(&mesh, &bcs, P_INIT);
  info("ndof = %d.", Space::get_num_dofs(Hermes::vector<Space *>(&u_space, &v_space)));

  // Initialize the weak formulation.
  WeakFormLameEquations wf(mu, lambda, BDY_3, f_0, f_1);

  // Initialize the FE problem.
  bool is_linear = true;
  DiscreteProblem dp(&wf, Hermes::vector<Space *>(&u_space, &v_space), is_linear);

  // Set up the solver, matrix, and rhs according to the solver selection.
  SparseMatrix* matrix = create_matrix(matrix_solver);
  Vector* rhs = create_vector(matrix_solver);
  Solver* solver = create_linear_solver(matrix_solver, matrix, rhs);

  // Initialize the solutions.
  Solution u_sln, v_sln;

  // Assemble the stiffness matrix and right-hand side vector.
  info("Assembling the stiffness matrix and right-hand side vector.");
  dp.assemble(matrix, rhs);

  // Solve the linear system and if successful, obtain the solutions.
  info("Solving the matrix problem.");
  if(solver->solve()) Solution::vector_to_solutions(solver->get_solution(), Hermes::vector<Space *>(&u_space, &v_space), 
                                                    Hermes::vector<Solution *>(&u_sln, &v_sln));
  else error ("Matrix solver failed.\n");
  
  // Visualize the solution.
  ScalarView view("Von Mises stress [Pa]", new WinGeom(0, 0, 800, 400));
  VonMisesFilter stress(Hermes::vector<MeshFunction *>(&u_sln, &v_sln), lambda, mu);
  view.show_mesh(false);
  view.show(&stress, HERMES_EPS_HIGH, H2D_FN_VAL_0, &u_sln, &v_sln, 1.5e5);

  // Wait for the view to be closed.
  View::wait();

  // Clean up.
  delete solver;
  delete matrix;
  delete rhs;

  return 0;
}

