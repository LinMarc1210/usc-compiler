=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200
while.cond -> while.end : 1
while.body -> if.then : 100
while.body -> if.else : 100
if.then -> if.end : 100
if.end -> while.cond : 200
if.else -> if.end : 100
END_PROFILE
