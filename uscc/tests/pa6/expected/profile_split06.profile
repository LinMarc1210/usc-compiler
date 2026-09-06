=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 10
while.cond -> while.end : 1
while.body -> while.cond1 : 10
while.cond1 -> while.body4 : 150
while.cond1 -> while.end5 : 10
while.body4 -> while.cond6 : 150
while.end5 -> while.cond : 10
while.cond6 -> while.body9 : 3000
while.cond6 -> while.end10 : 150
while.body9 -> while.cond6 : 3000
while.end10 -> while.cond1 : 150
END_PROFILE
