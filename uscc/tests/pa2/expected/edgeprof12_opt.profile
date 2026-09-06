=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> while.cond1, while.cond1 -> while.body3, while.cond1 -> while.end4, while.body3 -> if.then, while.body3 -> if.else, if.end -> while.cond1, if.else -> if.then6, if.else -> if.else8, if.end7 -> if.end
INSTRUMENTED: while.end4 -> while.cond, if.then -> if.end, if.then6 -> if.end7, if.else8 -> if.end7
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> while.cond1 : 0
while.cond1 -> while.body3 : 0
while.cond1 -> while.end4 : 0
while.body3 -> if.then : 0
while.body3 -> if.else : 0
while.end4 -> while.cond : 15000
if.then -> if.end : 15000
if.end -> while.cond1 : 0
if.else -> if.then6 : 0
if.else -> if.else8 : 0
if.then6 -> if.end7 : 112492500
if.end7 -> if.end : 224985000
if.else8 -> if.end7 : 112492500
END_PROFILE
