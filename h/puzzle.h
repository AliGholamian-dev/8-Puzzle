#ifndef PUZZLE_H
#define PUZZLE_H

#include <algorithm>
#include <array>
#include <chrono>
#include <execution>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

class Puzzle {
private:
    // Nested class for Node (each state of puzzle is a node)
    class Node {
    public:
        Node(
            const std::array<std::array<int, 3>, 3>& mat,
            int level,
            int lastDirection,
            int swaped_with,
            const std::shared_ptr<Node>& parent);
        /**
         * Checks if two states are the same
         * @param  {Node} second_node : Second puzzle to compare to
         * @return {bool}             : Two states are the same or not
         */
        bool operator==(const Node& second_node) const;
        std::shared_ptr<Node> parent_of_node;
        std::array<std::array<int, 3>, 3> mat;
        int zero_x, zero_y, level, lastDirection, swaped_with;
    };
    // Avoid verbose writing
    using Node_ptr = std::shared_ptr<Node>;
    // Step of puzzle's answer
    std::array<std::array<int, 3>, 3> initial_puzzle;
    std::array<std::array<int, 3>, 3> goal_puzzle;
    // Indicators of four moves for empty square
    std::array<int, 4> row { 1, 0, 0, -1 };
    std::array<int, 4> col { 0, -1, 1, 0 };
    // Algorithm enumrator
    enum Algo {
        A_Star,
        BFS,
        DLS
    };

    bool is_Solvable();

    /**
     * Checks if the next coordinate is in range of puzzle coordinates
     * @param  {int} x : next coordinate's x
     * @param  {int} y : next coordinate's y
     * @return {bool}  : If the next coordinate is in range of puzzle coordinates
     */
    bool Check_Coordinates(int x, int y) { return (x >= 0 && x < 3 && y >= 0 && y < 3); };

    /**
     * Calculate number of puzzle's disorder for ( A* ) algorithm
     * @param  {Node_ptr} input_node : State to calculate it's cost
     * @param  {int} mode            : Compare to init or goal puzzle ( for Bidirectional) 
     * @return {int}                 : Number of puzzle's disorder
     */
    int Calculate_Cost(const Node_ptr& input_node, int mode) const;

public:
    Puzzle(
        const std::array<std::array<int, 3>, 3>& initial_puzzle,
        const std::array<std::array<int, 3>, 3>& goal_puzzle);

    void Solve_Puzzle(const std::array<int, 2>& settings);

    std::vector<int> answer {};
};

extern "C" {
int* solve_for_Python(int initial_input[], int goal_input[], int setting[], int& size);
};

#endif