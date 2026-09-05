=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.end, while.body -> while.cond1, while.cond1 -> while.body3, while.cond1 -> while.end4, while.body3 -> if.else, while.end4 -> while.cond, if.then -> if.end, if.end -> if.end19, if.else -> if.then9, if.else -> if.else11, if.end10 -> if.end, if.then18 -> if.end19, if.end19 -> while.cond1
INSTRUMENTED: while.cond -> while.body, while.body3 -> if.then, if.end -> if.then18, if.then9 -> if.end10, if.else11 -> if.end10
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
if.end -> if.then18 : 2838031
if.end -> if.end19 : 0
if.else -> if.then9 : 0
if.else -> if.else11 : 0
if.then9 -> if.end10 : 15000
if.end10 -> if.end : 0
if.else11 -> if.end10 : 112492500
if.then18 -> if.end19 : 0
if.end19 -> while.cond1 : 0
END_PROFILE
