=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, if.end -> if.end4, if.then3 -> if.end4, if.end4 -> if.end9, if.then8 -> if.end9, if.end9 -> if.end14, if.then13 -> if.end14, if.end14 -> if.end19, if.then18 -> if.end19, if.end19 -> while.cond
INSTRUMENTED: while.body -> if.end, if.then -> if.end, if.end -> if.then3, if.end4 -> if.then8, if.end9 -> if.then13, if.end14 -> if.then18
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.end : 100000000
if.then -> if.end : 100000000
if.end -> if.then3 : 66666667
if.end -> if.end4 : 0
if.then3 -> if.end4 : 0
if.end4 -> if.then8 : 40000000
if.end4 -> if.end9 : 0
if.then8 -> if.end9 : 0
if.end9 -> if.then13 : 28571429
if.end9 -> if.end14 : 0
if.then13 -> if.end14 : 0
if.end14 -> if.then18 : 18181819
if.end14 -> if.end19 : 0
if.then18 -> if.end19 : 0
if.end19 -> while.cond : 0
END_PROFILE
