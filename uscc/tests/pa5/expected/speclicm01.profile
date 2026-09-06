=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 64
while.cond -> while.end : 1
while.body -> if.then : 4
while.body -> if.end : 60
if.then -> if.end : 4
if.end -> while.cond : 64
END_PROFILE
