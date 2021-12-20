# act_tp4

## How to Build

clone this repo RECURSIVELY or use git submodule init/update afterward.

```bash
git clone --recursive https://gitlab-etu.fil.univ-lille1.fr/jozeau/act_tp4.git
cd act_tp4
mkdir build
cd build
cmake ..
cmake --build . -j
./schedl <problem_file>
```

## Project structure

- **[heuristics](heuristics.hpp)**:
  - contains the constructive heuristics
  - the select functions to construct the solution
  - the heuristics Function_reflect (used to print heuristics names)

- **[iterated_local_search](iterated_local_search.hpp)**:
  - contains the ILS,
  - its perturbation function(s),
  - its acceptation function(s) and
  - its stop function(s)

- **[local_search](local_search.hpp)**:
  - contains local search algorithms (hill climbing and vnd)
  - their pivot rules

- **[neighborhood](neighborhood.hpp)**:
  - contains the polymorphic(used for vnd) neighborhood ranges
  - the `Backward_neighborhood` template and mixin to reverse the neighborhood traversal
  - type info for neighborhood

- **[schedl](schedl.cpp)**:
  - contains the main
  - read problems from file
  - write solution to file
  - tools to follow long lasting executions progress

- **[Task](Task.hpp)**:
  - Task
  - Scheduling
  - evaluate function

- **[utils](utils.hpp)**:
  - contains utility code (index type, vector using signed size, stop request singleton to handle ctrl+c gracefully)


## How to use our program

Generated solution will be stored in the `sols/` folder.

Hitting **ctrl+C** will gently ask the tasks to gracefully exit (saving the best they found)

If you want you can hit **4** times **ctrl+C** to terminate immediately

```bash
./schedl <problem_file> --sol <solution_file>
./schedl <problem_file> --heuristics
./schedl <problem_file> --random
./schedl <problem_file> --hc [--sol <solution_file>|--random]
./schedl <problem_file> --ils [--sol <solution_file>|--random]
```

## Our results

### constructivist heuristics

Our best heuristics so far are the static ones. Our heuristics which take into account the lateness of tasks are rarely the bests.

Our constructive heuristics work by choosing each time the smallest remaining task according to these functions we call "heuristic".

- `eval_sdelay_divmul_weight` : 

  $`max(\dfrac{taskSignedLateness}{weight},{taskSignedLateness}\times{taskWeight})`$

- `eval_sdelay_div_weight` : 

  $`\dfrac{taskSignedLateness}{taskWeight}`$

- `eval_static_sdelay_div_weight` :

  $`\dfrac{taskExpiryStartTime}{taskWeight}`$

- `eval_static_sdelay_mul_weight` :

  $`taskExpiryStartTime \times taskWeight`$

- `eval_static_sdelay` :

  $`taskExpiryStartTime`$

- `eval_static_expiry` :

  $`-taskExpiryTime`$

- `eval_static_expiry_div_weight_mul_time` :

  $`-taskExpiryTime \times \dfrac{taskLenght}{taskWeight}`$

$taskSignedLateness = Time+taskExpiryStartTime$

$taskExpiryStartTime = (taskExpiryTime - taskLenght)$

### Hill Climbing

Our hill climbing implementation support different neighborhoods and pivot functions.

We have implemented 3 neighborhoods and their backward conterparts can be obtain by composing a template mixin `Backward_neighborhood`.

- `Consecutive_single_swap_neighborhood` :

  A neighborhoods where the first neighbor is the one whose 1st and 2nd tasks are inverted.

  Then the k neighbor is the one with its k and k+1 members swapped.

  As such, we have $`N-1`$ neighbors.

- `Reverse_neighborhood` :

  A neighborhoods where all the tasks in a range are reversed, as if there were a pancake we flipped.

  The first neighbor is the base one with the reversal of tasks 0 to 1.

  The second neighbor is the base one with the reversal of tasks 0 to 2.

  The N_th neighbor is the base one with the reversal of tasks 1 to 2.

  ...

  As such, we have $`\dfrac{N\times(N-1)}{2}`$ neighbors.

- `Sliding_reverse_neighborhood<K>` :

  A neighborhood similar to the Reverse_neighborhood but where we limite the size of the subranges inverted to K and where we do not iterate from starting task to starting task but by range sizes.

  The first neighbor is the base one with the reversal of tasks 0 to 1.

  The second neighbor is the base one with the reversal of tasks 1 to 2.

  ...

  As such, we have $`\dfrac{N\times(N-1)}{2} - \dfrac{(N-K)\times(N-K+1)}{2} = \dfrac{N\times(N-1)-(N-K)\times(N-K+1)}{2}`$ neighbors.

  This neighborhoods made it possible to run faster than Reverse_neighborhood by avoiding some reverse such as the one from 0 to N-1 which should always be very bad, but keeping its advantages.

We have implemented 3 pivot function.

- `select2best` :
  Return the best solution out of the solutions given, which are to be better the the previous solution in the hill climbing

- `select2worst` :
  Return the worst solution out of the solutions given, which are to be better the the previous solution in the hill climbing

- `select2first` :
  Return the first solution found to be better than the previous one.

- `select2best_nfirst` :
  Return the best from the n first solutions found.

The best result we achieved with hill climbing is with the Reverse_neighborhood and the select2best pivot function but we only tested that on some of the problems.

### Iterated Local Search

Our ILS implementation support different :

- local search
- perturbation functions which support different neighborhoods
- acceptation function
- stop function

We have only implemented hill climbing as a local search, vnd was to much work although the dynamic support of multiple neighborhoods is implemented with type erased neighborhoods and inheritance hierarchy.

We have implemented multiples perturbation function `random_distant_neighbor` to select n random succesive neighbors and `Random_dyn_dist_neighbor` which select n random succesive neighbors with n increasing or decreasing but we haven't had the time to test it a lot.

We have implemented only one acceptation function for now.

We have implemented one stop function that stops after n consecutive non improving solutions.

We also have a ctrl+c signal handler that ask the task to finish and save their work that can be seen as a stop function.

The best configuration we've found so far is:

- `ILS` with
  - `hill_climbing`
    - `Sliding_reverse_neighborhood<15>`
    - `select2best`
  - `random_distant_neighbor`
    - `Sliding_reverse_neighborhood<20>`
    - `distance = 30`
  - `accept_best`
  - `stop_n_worse<20>`