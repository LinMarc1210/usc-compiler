=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, while.body -> if.else, if.then -> if.then3, if.end -> while.cond, if.then3 -> if.then8, if.then3 -> if.else10, if.end4 -> if.end, if.else5 -> if.end4, if.then8 -> if.then13, if.then8 -> if.else15, if.end9 -> if.end4, if.then13 -> if.then18, if.then13 -> if.else20, if.end14 -> if.end9, if.end19 -> if.end14
INSTRUMENTED: if.then -> if.else5, if.else -> if.end, if.else10 -> if.end9, if.else15 -> if.end14, if.then18 -> if.end19, if.else20 -> if.end19
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.else : 0
if.then -> if.then3 : 0
if.then -> if.else5 : 50000000
if.end -> while.cond : 0
if.else -> if.end : 100000000
if.then3 -> if.then8 : 0
if.then3 -> if.else10 : 0
if.end4 -> if.end : 100000000
if.else5 -> if.end4 : 0
if.then8 -> if.then13 : 0
if.then8 -> if.else15 : 0
if.end9 -> if.end4 : 0
if.else10 -> if.end9 : 25000000
if.then13 -> if.then18 : 0
if.then13 -> if.else20 : 0
if.end14 -> if.end9 : 25000000
if.else15 -> if.end14 : 12500000
if.then18 -> if.end19 : 6250000
if.end19 -> if.end14 : 12500000
if.else20 -> if.end19 : 6250000
END_PROFILE
