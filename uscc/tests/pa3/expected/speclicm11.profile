=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 8
while.cond -> while.end : 1
while.body -> while.cond1 : 8
while.cond1 -> while.body4 : 80
while.cond1 -> while.end5 : 8
while.body4 -> while.cond1 : 80
while.end5 -> if.then : 2
while.end5 -> if.end : 6
if.then -> if.end : 2
if.end -> while.cond : 8
END_PROFILE
