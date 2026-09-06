=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 6
while.cond -> while.end : 1
while.body -> while.cond1 : 6
while.cond1 -> while.body4 : 72
while.cond1 -> while.end5 : 6
while.body4 -> if.then : 12
while.body4 -> if.end : 60
while.end5 -> while.cond : 6
if.then -> if.end : 12
if.end -> while.cond1 : 72
END_PROFILE
