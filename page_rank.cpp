#include "include/page_rank.hpp"
#include <boost/numeric/ublas/io.hpp>

using namespace boost;
using namespace boost::numeric::ublas;
using namespace singularity;

std::shared_ptr<vector_t> page_rank::process(
        const matrix_t& outlink_matrix,
        const vector_t& initial_vector
) {
    sparce_vector_t v = matrix_tools::calculate_correction_vector(outlink_matrix);
    
    return calculate_rank(outlink_matrix, v, initial_vector);
}

std::shared_ptr<vector_t> page_rank::iterate(
        const matrix_t& outlink_matrix, 
        const sparce_vector_t& outlink_vector, 
        const vector_t& previous,
        const vector_t& teleportation
) {
    unsigned int num_accounts = outlink_matrix.size2();
    
    std::shared_ptr<vector_t> next(new vector_t(outlink_matrix.size1(), 0));
    
    matrix_tools::prod(*next, outlink_matrix, previous, parameters.num_threads);
    
    vector_t correction_vector(num_accounts, inner_prod(outlink_vector, previous));
    
    *next += correction_vector;
    *next += teleportation;
    
    return next;
}

std::shared_ptr<vector_t> page_rank::calculate_rank(
        const matrix_t& outlink_matrix, 
        const sparce_vector_t& outlink_vector,
        const vector_t& initial_vector
) {
//     unsigned int num_accounts = outlink_matrix.size2();
//     double_type initialValue = 1.0/num_accounts;
    std::shared_ptr<vector_t> next;
    std::shared_ptr<vector_t> previous(new vector_t(initial_vector));
    vector_t teleportation = (*previous) * TELEPORTATION_WEIGHT;
    
    matrix_t outlink_matrix_weighted = outlink_matrix * parameters.outlink_weight;
    sparce_vector_t outlink_vector_weighted = outlink_vector * parameters.outlink_weight;
    
    for (uint i = 0; i < MAX_ITERATIONS; i++) {
        next  = iterate(outlink_matrix_weighted, outlink_vector_weighted, *previous, teleportation);
        double_type norm = norm_1(*next - *previous);
        if (norm <= precision) {
            return next;
        } else {
            previous = next;
        }
    }
    
    return next;
}
