=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 5000
while.cond -> while.end : 1
while.body -> while.cond1 : 5000
while.cond1 -> while.body4 : 12497500
while.cond1 -> while.end5 : 5000
while.body4 -> if.then : 11245500
while.body4 -> if.end : 1252000
while.end5 -> while.cond : 5000
if.then -> if.end : 11245500
if.end -> while.cond1 : 12497500
END_PROFILE
