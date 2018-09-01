
#ifndef PAGE_RANKING_HPP
#define PAGE_RANKING_HPP

#include "rank_interface.hpp"
#include "utils.hpp"

namespace singularity {
    class page_rank: public rank_interface
    {
    public:
        const double_type TELEPORTATION_WEIGHT = double_type(0.1);
        page_rank(parameters_t parameters):parameters(parameters) {};
        const uint32_t MAX_ITERATIONS = 1000;
        virtual std::shared_ptr<vector_t> process(
            const matrix_t& outlink_matrix,
            const vector_t& initial_vector
        );
    private:
        parameters_t parameters;
        double_type const precision = 0.01;
        std::shared_ptr<vector_t> calculate_rank(
            const matrix_t& outlink_matrix, 
            const sparce_vector_t& outlink_vector,
            const vector_t& initial_vector
        );
        std::shared_ptr<vector_t> iterate(
            const matrix_t& outlink_matrix, 
            const sparce_vector_t& outlink_vector, 
            const vector_t& previous,
            const vector_t& teleportation
        );
    };
}

#endif /* PAGE_RANKING_HPP */
