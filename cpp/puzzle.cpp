#include "puzzle.h"

Puzzle::Node::Node(
    const std::array<std::array<int, 3>, 3>& mat,
    int level,
    int lastDirection,
    int swaped_with,
    const std::shared_ptr<Node>& parent)
{
    this->swaped_with = swaped_with;
    this->lastDirection = lastDirection;
    this->level = level;
    this->mat = mat;
    this->parent_of_node = parent;
    // Find coordinates of empty square
    for (size_t i = 0; i < 3; i++)
        for (size_t j = 0; j < 3; j++)
            if (this->mat[i][j] == 0) {
                this->zero_y = i;
                this->zero_x = j;
            }
}

/**
 * Checks if two states are the same
 * @param  {Node} second_node : Second puzzle to compare to
 * @return {bool}             : Two states are the same or not
 */
bool Puzzle::Node::operator==(const Node& second_node) const
{
    for (size_t i = 0; i < 3; i++)
        for (size_t j = 0; j < 3; j++)
            if (this->mat[i][j] != second_node.mat[i][j])
                return false;
    return true;
}

Puzzle::Puzzle(
    const std::array<std::array<int, 3>, 3>& initial_puzzle,
    const std::array<std::array<int, 3>, 3>& goal_puzzle)
{
    this->initial_puzzle = initial_puzzle;
    this->goal_puzzle = goal_puzzle;
}

bool Puzzle::is_Solvable()
{
    // Parity counter
    int inv_count { 0 };
    int inv_count_goal { 0 };
    std::vector<int> init, goal;

    // Pushing puzzles in 1D vectors
    for (size_t i = 0; i < 3; i++)
        for (size_t j = 0; j < 3; j++) {
            if (this->initial_puzzle[i][j] != 0)
                init.push_back(this->initial_puzzle[i][j]);
            if (this->goal_puzzle[i][j] != 0)
                goal.push_back(this->goal_puzzle[i][j]);
        }

    // Calculate the number of inversions in initial puzzle
    for (int i = 0; i < 7; i++)
        for (int j = i + 1; j < 8; j++)
            if (init[i] > init[j])
                inv_count++;
    // Calculate the number of inversions in goal puzzle
    for (int i = 0; i < 7; i++)
        for (int j = i + 1; j < 8; j++)
            if (goal[i] > goal[j])
                inv_count_goal++;
    // If both inversions are from same parity it's solvable
    // If Both parities are odd or evevn it's solvable
    return inv_count % 2 == inv_count_goal % 2;
}

/**
 * Calculate number of puzzle's disorder for ( A* ) algorithm
 * @param  {Node_ptr} input_node : State to calculate it's cost
 * @param  {int} mode            : Compare to init or goal puzzle ( for Bidirectional) 
 * @return {int}                 : Number of puzzle's disorder
 */
int Puzzle::Calculate_Cost(const Node_ptr& input_node, int mode) const
{
    // In DLS Calculate_Cost is not called
    if (mode == 2) // BFS -> just sort based on level not disorder
        return input_node->level;
    int cost { 0 };
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (input_node->mat[i][j] != 0) {
                // Compare to goal state
                if (mode == 0 && input_node->mat[i][j] != this->goal_puzzle[i][j])
                    cost++; // increase cost if two squares are not equal
                // Compare to init state
                else if (mode == 1 && input_node->mat[i][j] != this->initial_puzzle[i][j])
                    cost++; // increase cost if two squares are not equal
            }
    return cost + input_node->level;
}

void Puzzle::Solve_Puzzle(const std::array<int, 2>& settings)
{
    // Parse user settings
    int _max_depth { settings[1] };
    int cl_forw { 0 }, cl_rev { 1 };
    // Clear terminal
    // Warn user if puzzle is not solvable
    if (!this->is_Solvable()) {
        answer.push_back(-2);
        return;
    }
    // Check which algorithm to use -> Default is A*
    if (settings[0] == Algo::BFS) // User wants to run with BFS
        cl_forw = cl_rev = 2;
    else if (settings[0] == Algo::DLS) // User wants to run with DLS
        cl_forw = cl_rev = 3;
    else { // A* -> default
        cl_forw = 0;
        cl_rev = 1;
    }
    // Lambda function for comaparing two node's cost (Used in priority_queue)
    // Sorts nodes based on cost for ( A* ) algorithm and BFS
    // Cost = puzzle's disorder + level(depth) -> A* + Bidirectinal
    // Cost = level(depth) -> BFS + Bidirectinal
    // lambda always returm true -> DLS + Bidirectinal
    auto comp {
        [&](const Node_ptr& n1, const Node_ptr& n2) {
            return (cl_forw == 3 ? true : this->Calculate_Cost(n1, cl_forw) > this->Calculate_Cost(n2, cl_forw));
        }
    };
    // Same as above but for reverse traversing of tree (Bidirectional)
    auto rcomp {
        [&](const Node_ptr& n1, const Node_ptr& n2) {
            return (cl_rev == 3 ? true : this->Calculate_Cost(n1, cl_rev) > this->Calculate_Cost(n2, cl_rev));
        }
    };
    // priority_queue -> Container of nodes based on (lowers)cost for ( A* ) algorithm
    // Faster than std::vector + std::sort -> Tested
    std::priority_queue<Node_ptr, std::vector<Node_ptr>, decltype(comp)> Nodes_pq(comp);
    std::priority_queue<Node_ptr, std::vector<Node_ptr>, decltype(rcomp)> Nodes_rpq(rcomp);

    // Root of Tree (init state)
    Node_ptr root { std::make_shared<Node>(this->initial_puzzle, 0, 5, 0, nullptr) };
    // Root of Tree (goal state)(Bidirectional)
    Node_ptr goal_root { std::make_shared<Node>(this->goal_puzzle, 0, 5, 0, nullptr) };

    // Push Nodes to priority_queue
    Nodes_pq.push(root);
    Nodes_rpq.push(goal_root);
    std::vector<Node_ptr> forward {};
    std::vector<Node_ptr> backward {};
    while (!Nodes_pq.empty() && !Nodes_rpq.empty()) {
        // Extract the node with lowest cost for checking
        Node_ptr prior_node { Nodes_pq.top() };
        Node_ptr r_prior_node { Nodes_rpq.top() };
        // Remove extracted node from container
        Nodes_pq.pop();
        Nodes_rpq.pop();
        // Bidirectional algorithm
        // First possible result -> init state reaches goal first
        if (*prior_node == *goal_root) {
            while (prior_node != nullptr) {
                answer.push_back(prior_node->swaped_with);
                prior_node = prior_node->parent_of_node;
            }
            std::vector<int> reverse { answer.rbegin(), answer.rend() };
            answer = std::move(reverse);
            return;
        }
        // Second possible result -> goal state reaches init first
        else if (*r_prior_node == *root) {
            while (r_prior_node != nullptr) {
                answer.push_back(r_prior_node->swaped_with);
                r_prior_node = r_prior_node->parent_of_node;
            }
            return;
        }
        // This is where Bidirectional search comes to benefit really
        /**
         * Third possible result -> inint state and goal state intersect
         * in the middle of their searches
         */
        else {
            // Find intersection
            auto r1 = std::find_if(std::execution::par, forward.begin(), forward.end(),
                [&](Node_ptr temp) { return *temp == *r_prior_node; });
            if (r1 != forward.end()) {
                prior_node = *r1;
                /**
                 *Important -> remove redundant moves
                 *when two searches intersect from the point of intersection
                 *to a specific piont second search just rolls back first searche's moves
                */
                while (*(prior_node->parent_of_node) == *(r_prior_node->parent_of_node)) {
                    r_prior_node = r_prior_node->parent_of_node;
                    prior_node = prior_node->parent_of_node;
                }
                while (prior_node != nullptr) {
                    answer.push_back(prior_node->swaped_with);
                    prior_node = prior_node->parent_of_node;
                }

                std::vector<int> reverse { answer.rbegin(), answer.rend() };
                answer = std::move(reverse);
                while (r_prior_node != nullptr) {
                    answer.push_back(r_prior_node->swaped_with);
                    r_prior_node = r_prior_node->parent_of_node;
                }
                return;
            } else {
                auto r2 = std::find_if(std::execution::par, backward.begin(), backward.end(),
                    [&](Node_ptr temp) { return *temp == *prior_node; });
                if (r2 != backward.end()) {
                    r_prior_node = *r2;
                    /**
                    *Important -> remove redundant moves
                    *when two searches intersect from the point of intersection
                    *to a specific piont second search just rolls back first searche's moves
                    */
                    while (*(prior_node->parent_of_node) == *(r_prior_node->parent_of_node)) {
                        r_prior_node = r_prior_node->parent_of_node;
                        prior_node = prior_node->parent_of_node;
                    }
                    while (prior_node != nullptr) {
                        answer.push_back(prior_node->swaped_with);
                        prior_node = prior_node->parent_of_node;
                    }
                    std::vector<int> reverse { answer.rbegin(), answer.rend() };
                    answer = std::move(reverse);
                    while (r_prior_node != nullptr) {
                        answer.push_back(r_prior_node->swaped_with);
                        r_prior_node = r_prior_node->parent_of_node;
                    }
                    return;
                }
            }
        }
        // Generate every valid neighbor of empty square
        for (int i = 0; i < 4; i++) {
            // Ckeck if next move is valid -> Forward search
            if (
                this->Check_Coordinates(prior_node->zero_x + this->row[i], prior_node->zero_y + this->col[i])
                && prior_node->lastDirection != 3 - i
                && (prior_node->level < _max_depth || settings[0] != Algo::DLS)) // Check depth in DLS
            {
                std::array<std::array<int, 3>, 3> temp { prior_node->mat };
                // Swap empty square with it's neighbor
                std::swap(
                    temp[prior_node->zero_y][prior_node->zero_x],
                    temp[prior_node->zero_y + this->col[i]][prior_node->zero_x + this->row[i]]);
                // Generate new state of puzzle
                Node_ptr child { std::make_shared<Node>(temp, prior_node->level + 1, i, temp[prior_node->zero_y][prior_node->zero_x], prior_node) };
                // Push new state of puzzle to priority_queue
                if (settings[0] == Algo::DLS) {
                    if (std::find_if(std::execution::par, forward.begin(), forward.end(), [&](Node_ptr temp) { return *temp == *child; }) == forward.end()) {
                        Nodes_pq.push(child);
                        forward.push_back(child);
                    }
                } else {
                    Nodes_pq.push(child);
                    forward.push_back(child);
                }
            }
            // Ckeck if next move is valid -> Reverse search
            if (
                this->Check_Coordinates(r_prior_node->zero_x + this->row[i], r_prior_node->zero_y + this->col[i])
                && r_prior_node->lastDirection != 3 - i
                && (r_prior_node->level < _max_depth || settings[0] != Algo::DLS)) // Check depth in DLS
            {
                std::array<std::array<int, 3>, 3> temp { r_prior_node->mat };
                // Swap empty square with it's neighbor
                std::swap(
                    temp[r_prior_node->zero_y][r_prior_node->zero_x],
                    temp[r_prior_node->zero_y + this->col[i]][r_prior_node->zero_x + this->row[i]]);
                // Generate new state of puzzle
                Node_ptr child { std::make_shared<Node>(temp, r_prior_node->level + 1, i, temp[r_prior_node->zero_y][r_prior_node->zero_x], r_prior_node) };
                // Push new state of puzzle to priority_queue
                if (settings[0] == Algo::DLS) {
                    if (std::find_if(std::execution::par, backward.begin(), backward.end(), [&](Node_ptr temp) { return *temp == *child; }) == backward.end()) {
                        Nodes_rpq.push(child);
                        backward.push_back(child);
                    }
                } else {
                    Nodes_rpq.push(child);
                    backward.push_back(child);
                }
            }
        }
    }
    answer.push_back(-1);
    return;
}

int* solve_for_Python(int initial_input[], int goal_input[], int setting[], int& size)
{
    // generate default puzzles
    std::array<int, 2> settings { setting[0], setting[1] };
    std::array<std::array<int, 3>, 3> initial {
        std::array<int, 3> { initial_input[0], initial_input[1], initial_input[2] },
        std::array<int, 3> { initial_input[3], initial_input[4], initial_input[5] },
        std::array<int, 3> { initial_input[6], initial_input[7], initial_input[8] }
    };
    std::array<std::array<int, 3>, 3> goal {
        std::array<int, 3> { goal_input[0], goal_input[1], goal_input[2] },
        std::array<int, 3> { goal_input[3], goal_input[4], goal_input[5] },
        std::array<int, 3> { goal_input[6], goal_input[7], goal_input[8] }
    };
    // Create an object of puzzle class an pass default puzzles
    Puzzle* New_Puzzle { new Puzzle(initial, goal) };
    New_Puzzle->Solve_Puzzle(settings);
    size = New_Puzzle->answer.size();
    return std::move(New_Puzzle->answer.data());
}
