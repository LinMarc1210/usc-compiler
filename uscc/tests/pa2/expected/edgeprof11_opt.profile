=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, while.body -> if.else, if.then -> if.then6, if.then -> if.else8, if.end -> while.cond, if.then6 -> if.then14, if.then6 -> if.else16, if.end7 -> if.end, if.then14 -> if.then21, if.then14 -> if.else23, if.end15 -> if.end7, if.end22 -> if.end15
INSTRUMENTED: if.else -> if.end, if.else8 -> if.end7, if.else16 -> if.end15, if.then21 -> if.end22, if.else23 -> if.end22
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.else : 0
if.then -> if.then6 : 0
if.then -> if.else8 : 0
if.end -> while.cond : 0
if.else -> if.end : 100000000
if.then6 -> if.then14 : 0
if.then6 -> if.else16 : 0
if.end7 -> if.end : 100000000
if.else8 -> if.end7 : 50000000
if.then14 -> if.then21 : 0
if.then14 -> if.else23 : 0
if.end15 -> if.end7 : 50000000
if.else16 -> if.end15 : 25000000
if.then21 -> if.end22 : 8333333
if.end22 -> if.end15 : 25000000
if.else23 -> if.end22 : 16666667
END_PROFILE
