=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 48
while.cond -> while.end : 1
while.body -> if.then : 2
while.body -> if.end : 46
if.then -> if.end : 2
if.end -> while.cond : 48
END_PROFILE
