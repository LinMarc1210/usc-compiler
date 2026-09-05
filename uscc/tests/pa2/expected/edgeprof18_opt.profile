=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, if.end -> if.else7, if.else -> if.end, if.then5 -> if.end6, if.end6 -> if.else15, if.else7 -> if.end6, if.then13 -> if.end14, if.end14 -> if.else23, if.else15 -> if.end14, if.then21 -> if.end22, if.end22 -> if.end27, if.else23 -> if.end22, if.then26 -> if.end27, if.end27 -> while.cond
INSTRUMENTED: while.body -> if.else, if.then -> if.end, if.end -> if.then5, if.end6 -> if.then13, if.end14 -> if.then21, if.end22 -> if.then26
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.else : 0
if.then -> if.end : 100000000
if.end -> if.then5 : 0
if.end -> if.else7 : 0
if.else -> if.end : 0
if.then5 -> if.end6 : 0
if.end6 -> if.then13 : 0
if.end6 -> if.else15 : 0
if.else7 -> if.end6 : 0
if.then13 -> if.end14 : 0
if.end14 -> if.then21 : 0
if.end14 -> if.else23 : 0
if.else15 -> if.end14 : 0
if.then21 -> if.end22 : 0
if.end22 -> if.then26 : 379505
if.end22 -> if.end27 : 0
if.else23 -> if.end22 : 0
if.then26 -> if.end27 : 0
if.end27 -> while.cond : 0
END_PROFILE
