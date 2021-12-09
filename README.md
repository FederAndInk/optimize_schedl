# act_tp4

# How to Build

clone this repo RECURSIVELY or use git submodule init/update afterward.

```bash
git clone --recursive https://gitlab-etu.fil.univ-lille1.fr/jozeau/act_tp4.git
cd act_tp4
mkdir build
cd build
cmake ..
cmake --build . -j
./sched <./problem_instance
```
