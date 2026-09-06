=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 5
while.cond -> while.end : 1
while.body -> while.cond1 : 5
while.cond1 -> while.body4 : 100
while.cond1 -> while.end5 : 5
while.body4 -> if.then : 10
while.body4 -> if.end : 90
while.end5 -> while.cond : 5
if.then -> if.end : 10
if.end -> while.cond1 : 100
END_PROFILE
