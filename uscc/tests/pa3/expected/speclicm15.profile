=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 40
while.cond -> while.end : 1
while.body -> if.then : 14
while.body -> if.else : 26
if.then -> if.end : 14
if.end -> if.then12 : 2
if.end -> if.end13 : 38
if.else -> if.end : 26
if.then12 -> if.end13 : 2
if.end13 -> while.cond : 40
END_PROFILE
