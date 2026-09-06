=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 10
while.cond -> while.end : 1
while.body -> while.cond1 : 10
while.cond1 -> while.body4 : 110
while.cond1 -> while.end5 : 10
while.body4 -> while.cond1 : 110
while.end5 -> while.cond : 10
END_PROFILE
