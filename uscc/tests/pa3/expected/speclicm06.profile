=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 120
while.cond -> while.end : 1
while.body -> if.then : 8
while.body -> if.end : 112
if.then -> if.end : 8
if.end -> if.then8 : 6
if.end -> if.end9 : 114
if.then8 -> if.end9 : 6
if.end9 -> if.then16 : 3
if.end9 -> if.end17 : 117
if.then16 -> if.end17 : 3
if.end17 -> while.cond : 120
END_PROFILE
