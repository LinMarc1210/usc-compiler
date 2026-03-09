=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.end, while.body -> while.cond1, while.cond1 -> while.body3, while.cond1 -> while.end4, while.body3 -> if.else, while.end4 -> while.cond, if.then -> if.end, if.end -> if.end12, if.else -> if.then5, if.else -> if.else7, if.end6 -> if.end, if.then11 -> if.end12, if.end12 -> while.cond1
INSTRUMENTED: while.cond -> while.body, while.body3 -> if.then, if.end -> if.then11, if.then5 -> if.end6, if.else7 -> if.end6
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 15000
while.cond -> while.end : 1
while.body -> while.cond1 : 0
while.cond1 -> while.body3 : 0
while.cond1 -> while.end4 : 0
while.body3 -> if.then : 112492500
while.body3 -> if.else : 0
while.end4 -> while.cond : 0
if.then -> if.end : 0
if.end -> if.then11 : 2838031
if.end -> if.end12 : 0
if.else -> if.then5 : 0
if.else -> if.else7 : 0
if.then5 -> if.end6 : 15000
if.end6 -> if.end : 0
if.else7 -> if.end6 : 112492500
if.then11 -> if.end12 : 0
if.end12 -> while.cond1 : 0
END_PROFILE
