=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 100
while.cond -> while.end : 1
while.body -> if.then : 4
while.body -> if.end : 96
if.then -> if.then7 : 2
if.then -> if.else : 2
if.end -> while.cond : 100
if.then7 -> if.end8 : 2
if.end8 -> if.end : 4
if.else -> if.end8 : 2
END_PROFILE
