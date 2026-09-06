=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.end, if.then -> while.cond, if.end -> if.end4, if.then3 -> while.cond, if.end4 -> if.end8, if.then7 -> while.cond, if.end8 -> if.end12, if.then11 -> while.cond, if.then15 -> while.cond, if.end16 -> while.cond
INSTRUMENTED: while.body -> if.then, if.end -> if.then3, if.end4 -> if.then7, if.end8 -> if.then11, if.end12 -> if.then15, if.end12 -> if.end16
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.end : 0
if.then -> while.cond : 0
if.end -> if.then3 : 33333333
if.end -> if.end4 : 0
if.then3 -> while.cond : 0
if.end4 -> if.then7 : 13333333
if.end4 -> if.end8 : 0
if.then7 -> while.cond : 0
if.end8 -> if.then11 : 7619047
if.end8 -> if.end12 : 0
if.then11 -> while.cond : 0
if.end12 -> if.then15 : 4155844
if.end12 -> if.end16 : 41558443
if.then15 -> while.cond : 0
if.end16 -> while.cond : 0
END_PROFILE
