=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.end, if.then -> while.cond, if.end -> if.end7, if.then6 -> while.cond, if.end7 -> if.end12, if.then11 -> while.cond, if.end12 -> if.end17, if.then16 -> while.cond, if.then21 -> while.cond, if.end22 -> while.cond
INSTRUMENTED: while.body -> if.then, if.end -> if.then6, if.end7 -> if.then11, if.end12 -> if.then16, if.end17 -> if.then21, if.end17 -> if.end22
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.end : 0
if.then -> while.cond : 0
if.end -> if.then6 : 33333333
if.end -> if.end7 : 0
if.then6 -> while.cond : 0
if.end7 -> if.then11 : 13333333
if.end7 -> if.end12 : 0
if.then11 -> while.cond : 0
if.end12 -> if.then16 : 7619047
if.end12 -> if.end17 : 0
if.then16 -> while.cond : 0
if.end17 -> if.then21 : 4155844
if.end17 -> if.end22 : 41558443
if.then21 -> while.cond : 0
if.end22 -> while.cond : 0
END_PROFILE
