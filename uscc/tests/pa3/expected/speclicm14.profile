=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 3
while.cond -> while.end : 1
while.body -> while.cond1 : 3
while.cond1 -> while.body4 : 12
while.cond1 -> while.end5 : 3
while.body4 -> while.cond6 : 12
while.end5 -> if.then : 2
while.end5 -> if.end : 1
while.cond6 -> while.body9 : 60
while.cond6 -> while.end10 : 12
while.body9 -> while.cond6 : 60
while.end10 -> while.cond1 : 12
if.then -> if.end : 2
if.end -> while.cond : 3
END_PROFILE
