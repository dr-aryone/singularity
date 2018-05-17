#include "include/emission.hpp"

using namespace singularity;

activity_period::activity_period()
{
    p_weight_matrix = std::make_shared<matrix_t>(initial_size, initial_size);
}

void activity_period::collect_accounts(
    account_id_map_t& account_id_map,
    const std::vector<transaction_t>& transactions
) {
    std::lock_guard<std::mutex> lock(accounts_lock);
    unsigned int account_id = account_id_map.size();
    for (unsigned int i=0; i<transactions.size(); i++) {
        transaction_t transaction = transactions[i];
        account_id_map_t::iterator found_source = account_id_map.find(transaction.source_account);
        account_id_map_t::iterator found_target = account_id_map.find(transaction.target_account);
        if (found_source == account_id_map.end()) {
            account_id_map.insert(account_id_map_t::value_type(transaction.source_account, account_id++));
        }
        if (found_target == account_id_map.end()) {
            account_id_map.insert(account_id_map_t::value_type(transaction.target_account, account_id++));
        }
    }
}

void activity_period::add_block(const std::vector<transaction_t>& transactions)
{
    std::lock_guard<std::mutex> lock(weight_matrix_lock);
    collect_accounts(account_map, transactions);
    
    if (p_weight_matrix->size1() < account_map.size()) {
        matrix_t::size_type new_size = p_weight_matrix->size1();
        while (new_size < account_map.size()) {
            new_size *= 2;
        }
        p_weight_matrix = matrix_tools::resize(*p_weight_matrix, new_size, new_size);
    }

    update_weight_matrix(*p_weight_matrix, account_map, transactions);
}

byte_matrix_t activity_period::calculate_link_matrix(
    matrix_t::size_type size,
    matrix_t& weight_matrix
)
{
    matrix_t l(size, size);
    {
        std::lock_guard<std::mutex> lock(weight_matrix_lock);

        for (matrix_t::iterator1 i = weight_matrix.begin1(); i != weight_matrix.end1(); i++)
        {
            if (i.index1() >= size) {
                break;
            }
            for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
            {
                if (j.index2() >= size) {
                    break;
                }
                l(j.index1(), j.index2()) -= *j;
                l(j.index2(), j.index1()) += *j;
            }
        }
    }
    
    byte_matrix_t result(size, size);
    
    for (matrix_t::iterator1 i = l.begin1(); i != l.end1(); i++)
    {
        for (matrix_t::iterator2 j = i.begin(); j != i.end(); j++)
        {
            if (*j > 0) {
                result(j.index1(), j.index2()) = 1;
            }
        }
    }
    
    return result;
}

void activity_period::update_weight_matrix(matrix_t& weight_matrix, account_id_map_t& account_id_map, const std::vector<transaction_t>& transactions) {
    for (unsigned int i=0; i<transactions.size(); i++) {
        transaction_t t = transactions[i];
        weight_matrix(account_id_map[t.source_account], account_id_map[t.target_account]) += t.amount;
    }
}

double activity_period::get_activity()
{
    byte_matrix_t n = calculate_link_matrix(account_map.size(), *p_weight_matrix);
    
    return (double) n.nnz();
}

uint64_t emission_calculator::calculate(uint64_t total_emission, double activity)
{
    uint64_t current_supply = parameters.initial_supply + total_emission;

    double emission_limit = pow((1 + (double) parameters.year_emission_limit / 100), 1.0 / parameters.emission_event_count_per_year) - 1;

    double argument = (parameters.emission_scale * activity - total_emission) / current_supply;
    
    if (argument <= 0) {
        return 0;
    } else {
        return current_supply * emission_limit * tanh(parameters.delay_koefficient * argument / emission_limit);
    }
}


void activity_period::clear()
{
    std::lock_guard<std::mutex> lock(weight_matrix_lock);
    
    p_weight_matrix->clear();
}

