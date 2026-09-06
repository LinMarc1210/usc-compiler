=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 500
while.cond -> while.end : 1
while.body -> if.then : 499
while.body -> if.else : 1
if.then -> if.end : 499
if.end -> while.cond : 500
if.else -> if.end : 1
END_PROFILE
