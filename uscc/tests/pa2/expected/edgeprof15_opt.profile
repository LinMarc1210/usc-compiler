=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> while.cond1, while.cond1 -> while.body6, while.cond1 -> while.end7, while.body6 -> if.then, if.end -> while.cond1
INSTRUMENTED: while.body6 -> if.end, while.end7 -> while.cond, if.then -> if.end
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> while.cond1 : 0
while.cond1 -> while.body6 : 0
while.cond1 -> while.end7 : 0
while.body6 -> if.then : 0
while.body6 -> if.end : 1252000
while.end7 -> while.cond : 5000
if.then -> if.end : 11245500
if.end -> while.cond1 : 0
END_PROFILE
