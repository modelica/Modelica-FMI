#ifndef _DS_SUNDIALS_EXTENSIONS_H
#define _DS_SUNDIALS_EXTENSIONS_H

#ifndef ONLY_INCLUDE_INLINE_INTEGRATION

#include <cvode/cvode.h>
#if defined(INCLUDE_SUNDIALS_IDA) || defined(GODESS)
#include <ida/ida.h>
#endif

struct DSSparseAnalyticJacobianFixData;
typedef struct SparseFunctions {
	SUNMatrix (*sun_sparse_matrix)(sunindextype M, sunindextype N, sunindextype NNZ, int sparsetype, SUNContext sunctx);
	SUNLinearSolver (*sun_linsol_superlumt)(N_Vector y, SUNMatrix A, int num_threads, SUNContext sunctx);
	sunindextype* (*sun_sparse_matrix_index_pointers)(SUNMatrix A);
	sunindextype* (*sun_sparse_matrix_index_values)(SUNMatrix A);
	realtype* (*sun_sparse_matrix_data)(SUNMatrix A);
	void (*ds_superlumt_free_global_data)(SUNLinearSolver S);
	void (*ds_superlumt_reinit)(SUNLinearSolver S);
	int (*ds_sunmat_add_diagonal_sparse)(realtype val, SUNMatrix A);
	int (*ds_sunmat_add_partial_diagonal_sparse)(realtype val, sunindextype nxorig, SUNMatrix* partialIPtr, SUNContext sunctx, SUNMatrix A);
	int (*ds_sparse_analytic_jacobian_fix_data_allocate)(struct DSSparseAnalyticJacobianFixData** fix_data_ptr, const int num_states, const int num_non_zero, const int jac_rowvals_reset, void* (*alloc_ptr)(size_t, size_t));
	int (*ds_sparse_analytic_jacobian_fix_data_copy)(struct DSSparseAnalyticJacobianFixData* fix_data_target, const struct DSSparseAnalyticJacobianFixData* fix_data_source);
	void (*ds_sparse_analytic_jacobian_fix_data_free)(struct DSSparseAnalyticJacobianFixData* fix_data, void (*dealloc_ptr)(void*));
	int (*ds_sparse_set_analytic_jacobian_structure)(struct DSSparseAnalyticJacobianFixData* fix_data,
		const int* JacRows, const int* JacCols,
		int* rowvals, int* colptrs);
	int (*ds_sparse_reset_analytic_jacobian_structure)(const struct DSSparseAnalyticJacobianFixData* fix_data, int* rowvals, int* colptrs);
	int (*ds_sparse_set_analytic_jacobian_values)(const struct DSSparseAnalyticJacobianFixData* fix_data, const double* JacValues, double* data);
} SparseFunctions;

#ifdef __cplusplus
extern "C" {
#endif

SUNDIALS_EXPORT int DSCVodeStepRootInit(void *cvode_mem, CVStepRootFn g);
SUNDIALS_EXPORT int DSCVodeUpdateNordsieckXbis(void* cvode_mem, const N_Vector xbis_update);
SUNDIALS_EXPORT int DSCVodeSetStateErrorMonitorFn(void* cvode_mem, CVodeStateErrorMonitorFN fn);

#if defined(INCLUDE_SUNDIALS_IDA) || defined(GODESS)
SUNDIALS_EXPORT int DSIDAStepRootInit(void* IDA_mem, IDAStepRootFn g);
SUNDIALS_EXPORT int DSIDASetMonitorFn(void* IDA_mem, IDAMonitorFN fn);
SUNDIALS_EXPORT int DSIDASetAccurateDAEEventFn(void* IDA_mem, IDAAtDAEEvent fn);
SUNDIALS_EXPORT int DSIDAUpdateHistoryXbis(void* IDA_mem, const N_Vector xbis_update);

SUNDIALS_EXPORT int DSSUNMatAddDiagonal_Dense(realtype val, SUNMatrix A);
SUNDIALS_EXPORT int DSSUNMatAddPartialDiagonal_Dense(realtype val, sunindextype nxorig, SUNMatrix* partialIPtr, SUNContext sunctx, SUNMatrix A);
SUNDIALS_EXPORT int DSIDASetStateErrorMonitorFn(void* IDA_mem, IDAStateErrorMonitorFN fn);
#endif

#if !defined(FMU_SOURCE_CODE_EXPORT) || defined(FMU_SOURCE_CODE_EXPORT_SPARSE)
SUNDIALS_EXPORT void DSSetSparseFunctions(struct SparseFunctions* functions);
#endif

#ifdef __cplusplus
}
#endif

#endif
#endif
