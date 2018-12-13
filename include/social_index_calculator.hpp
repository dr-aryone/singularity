
#ifndef SOCIAL_INdEX_CALCULATOR_HPP
#define SOCIAL_INDEX_CALCULATOR_HPP

#include <mutex>
#include <ctime>
#include "utils.hpp"
#include "relations.hpp"
#include "rank_interface.hpp"
#include "filters.hpp"

namespace singularity {
    
    class social_index_calculator 
    {
    public:
        const matrix_t::size_type initial_size = 10;
        social_index_calculator(
            parameters_t parameters, 
            bool disable_negative_weights,
            std::shared_ptr<rank_interface> p_rank_calculator
        ): parameters(parameters), disable_negative_weights(disable_negative_weights), p_rank_calculator(p_rank_calculator) {
            p_hierarchy_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
            p_vote_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
            p_decay_manager = std::make_shared<decay_manager_t>(parameters.decay_period, parameters.decay_koefficient);
        }
        void add_block(const std::vector<std::shared_ptr<relation_t> >& transactions);
        void skip_blocks(unsigned int blocks_count);
        rate_t calculate();
        unsigned int get_total_handled_block_count();
        void set_parameters(parameters_t params);
        parameters_t get_parameters();
        void set_filter(std::shared_ptr<filter_interface> filter)
        {
            p_filter = filter;
        };
        void add_stack_vector(const std::map<std::string, double_type>& stacks);
    private:
        friend class boost::serialization::access;
        parameters_t parameters;
        bool disable_negative_weights;
        
        unsigned int total_handled_blocks_count = 0;
        unsigned int handled_blocks_count = 0;
        std::shared_ptr<matrix_t> p_hierarchy_matrix;
        std::shared_ptr<matrix_t> p_vote_matrix;
//         std::map<node_type, std::shared_ptr<account_id_map_t> > node_maps;
        account_id_map_t account_map;
        account_id_map_t content_map;
        
//         singularity::vector_t stack_vector;
        std::map<std::string, double_type> stack_map;
        
        uint64_t accounts_count = 0;
        uint64_t contents_count = 0;
        std::mutex accounts_lock;
        std::mutex weight_matrix_lock;
        
        std::shared_ptr<rank_interface> p_rank_calculator;
        std::shared_ptr<decay_manager_t> p_decay_manager;
        std::shared_ptr<filter_interface> p_filter;

        std::vector<std::shared_ptr<relation_t> > filter_block(const std::vector<std::shared_ptr<relation_t> >& block);
                
        rate_t calculate_score(
            const vector_t& account_rank,
            const vector_t& content_rank
        );
        
        void collect_accounts(
            const std::vector<std::shared_ptr<relation_t> >& transactions
        );
        void calculate_outlink_matrix(
            matrix_t& o,
            matrix_t& weight_matrix,
            additional_matrices_vector& additional_matrices
        );
        void update_weight_matrix(
            const std::vector<std::shared_ptr<relation_t> >& transactions
        );
        void normalize_columns(matrix_t &m, additional_matrices_vector& additional_matrices);
        vector_t create_initial_vector();
        void limit_values(matrix_t& m);
    };
}

BOOST_CLASS_VERSION( singularity::social_index_calculator, 1 )

#endif /* SOCIAL_INDEX_CALCULATOR_HPP */
