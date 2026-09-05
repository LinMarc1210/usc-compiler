=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, if.end -> if.else10, if.else -> if.end, if.then8 -> if.end9, if.end9 -> if.else21, if.else10 -> if.end9, if.then19 -> if.end20, if.end20 -> if.else32, if.else21 -> if.end20, if.then30 -> if.end31, if.end31 -> if.end39, if.else32 -> if.end31, if.then38 -> if.end39, if.end39 -> while.cond
INSTRUMENTED: while.body -> if.else, if.then -> if.end, if.end -> if.then8, if.end9 -> if.then19, if.end20 -> if.then30, if.end31 -> if.then38
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.else : 0
if.then -> if.end : 100000000
if.end -> if.then8 : 0
if.end -> if.else10 : 0
if.else -> if.end : 0
if.then8 -> if.end9 : 0
if.end9 -> if.then19 : 0
if.end9 -> if.else21 : 0
if.else10 -> if.end9 : 0
if.then19 -> if.end20 : 0
if.end20 -> if.then30 : 0
if.end20 -> if.else32 : 0
if.else21 -> if.end20 : 0
if.then30 -> if.end31 : 0
if.end31 -> if.then38 : 379505
if.end31 -> if.end39 : 0
if.else32 -> if.end31 : 0
if.then38 -> if.end39 : 0
if.end39 -> while.cond : 0
END_PROFILE
