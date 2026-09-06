=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 40
while.cond -> while.end : 1
while.body -> if.then : 2
while.body -> if.end : 38
if.then -> if.end : 2
if.end -> while.cond : 40
END_PROFILE
