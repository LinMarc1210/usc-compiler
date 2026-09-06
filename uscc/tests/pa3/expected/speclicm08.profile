=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 50
while.cond -> while.end : 1
while.body -> if.then : 5
while.body -> if.end : 45
if.then -> if.end : 5
if.end -> while.cond : 50
END_PROFILE
