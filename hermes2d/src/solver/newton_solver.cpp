// This file is part of Hermes2D
//
// Copyright (c) 2009 hp-FEM group at the University of Nevada, Reno (UNR).
// Email: hpfem-group@unr.edu, home page: http://www.hpfem.org/.
//
// Hermes2D is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// Hermes2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hermes2D; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#include "solver/newton_solver.h"
#include "projections/ogprojection.h"
#include "hermes_common.h"

using namespace Hermes::Algebra;
using namespace Hermes::Solvers;

namespace Hermes
{
  namespace Hermes2D
  {
    template<typename Scalar>
    NewtonSolver<Scalar>::NewtonSolver() : Solver<Scalar>(), NewtonMatrixSolver<Scalar>()
    {
      this->dp = new DiscreteProblem<Scalar>(false, true);
      this->own_dp = true;
    }

    template<typename Scalar>
    NewtonSolver<Scalar>::NewtonSolver(DiscreteProblem<Scalar>* dp) : Solver<Scalar>(dp), NewtonMatrixSolver<Scalar>()
    {
    }

    template<typename Scalar>
    NewtonSolver<Scalar>::NewtonSolver(WeakFormSharedPtr<Scalar> wf, SpaceSharedPtr<Scalar> space) : Solver<Scalar>(wf, space), NewtonMatrixSolver<Scalar>()
    {
      this->dp = new DiscreteProblem<Scalar>(wf, space, false, true);
      this->own_dp = true;
    }

    template<typename Scalar>
    NewtonSolver<Scalar>::NewtonSolver(WeakFormSharedPtr<Scalar> wf, std::vector<SpaceSharedPtr<Scalar> > spaces) : Solver<Scalar>(wf, spaces), NewtonMatrixSolver<Scalar>()
    {
      this->dp = new DiscreteProblem<Scalar>(wf, spaces, false, true);
      this->own_dp = true;
    }

    template<typename Scalar>
    NewtonSolver<Scalar>::~NewtonSolver()
    {
    }

    template<typename Scalar>
    void NewtonSolver<Scalar>::solve(Scalar* coeff_vec)
    {
      NewtonMatrixSolver<Scalar>::solve(coeff_vec);
    }

    template<typename Scalar>
    Scalar* NewtonSolver<Scalar>::get_sln_vector()
    {
      return this->sln_vector;
    }

    template<typename Scalar>
    void NewtonSolver<Scalar>::set_verbose_output(bool to_set)
    {
      MatrixSolver<Scalar>::set_verbose_output(to_set);
      this->dp->set_verbose_output(to_set);
    }

    template<typename Scalar>
    void NewtonSolver<Scalar>::assemble_residual(bool store_previous_residual)
    {
      this->dp->assemble(this->sln_vector, this->get_residual());
      this->process_vector_output(this->get_residual(), this->get_current_iteration_number());
      this->get_residual()->change_sign();
    }

    template<typename Scalar>
    bool NewtonSolver<Scalar>::assemble_jacobian(bool store_previous_jacobian)
    {
      bool result = this->dp->assemble(this->sln_vector, this->get_jacobian());
      /// After the first time we assemble the matrix on the new reference space, we can no longer reuse the previous one.
      this->dp->set_reassembled_states_reuse_linear_system_fn(nullptr);

      this->process_matrix_output(this->get_jacobian(), this->get_current_iteration_number());
      return result;
    }

    template<typename Scalar>
    bool NewtonSolver<Scalar>::assemble(bool store_previous_jacobian, bool store_previous_residual)
    {
      bool result = this->dp->assemble(this->sln_vector, this->get_jacobian(), this->get_residual());
      /// After the first time we assemble the matrix on the new reference space, we can no longer reuse the previous one.
      this->dp->set_reassembled_states_reuse_linear_system_fn(nullptr);
      this->get_residual()->change_sign();
      this->process_vector_output(this->get_residual(), this->get_current_iteration_number());
      this->process_matrix_output(this->get_jacobian(), this->get_current_iteration_number());
      return result;
    }

    template<typename Scalar>
    bool NewtonSolver<Scalar>::isOkay() const
    {
      return Solver<Scalar>::isOkay() && Hermes::Solvers::NewtonMatrixSolver<Scalar>::isOkay();
    }

    template<typename Scalar>
    void NewtonSolver<Scalar>::set_weak_formulation(WeakFormSharedPtr<Scalar> wf)
    {
      Solver<Scalar>::set_weak_formulation(wf);
      this->jacobian_reusable = false;
    }

    template<typename Scalar>
    void NewtonSolver<Scalar>::init_solving(Scalar* coeff_vec)
    {
      this->problem_size = Space<Scalar>::assign_dofs(this->get_spaces());
      NewtonMatrixSolver<Scalar>::init_solving(coeff_vec);
    }

    template<typename Scalar>
    void NewtonSolver<Scalar>::set_spaces(std::vector<SpaceSharedPtr<Scalar> > spaces)
    {
      Solver<Scalar>::set_spaces(spaces);
      this->jacobian_reusable = false;
    }

    template class HERMES_API NewtonSolver < double > ;
    template class HERMES_API NewtonSolver < std::complex<double> > ;
  }
}
