=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 5000
while.cond -> while.end : 1
while.body -> while.cond1 : 5000
while.cond1 -> while.body6 : 12497500
while.cond1 -> while.end7 : 5000
while.body6 -> if.then : 11245500
while.body6 -> if.end : 1252000
while.end7 -> while.cond : 5000
if.then -> if.end : 11245500
if.end -> while.cond1 : 12497500
END_PROFILE
