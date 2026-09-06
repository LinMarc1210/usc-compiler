=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 80
while.cond -> while.end : 1
while.body -> if.then : 4
while.body -> if.end : 76
if.then -> if.end : 4
if.end -> if.then9 : 3
if.end -> if.end10 : 77
if.then9 -> if.end10 : 3
if.end10 -> while.cond : 80
END_PROFILE
